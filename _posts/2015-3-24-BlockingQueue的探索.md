---
layout: post
category: Linux
tags: Linux
description: BlockingQueue的一些基本知识
---

BlockingQueue对于很多人来说，再熟悉不过了，在多线程的学习中，最基础的估计就是阻塞队列了，以及建立在此基础上的生产者和消费者模式。最近在实际的编码中遇到和他相关的问题，就仔细再去了解了一些围绕于BlockingQueue的基本知识。需要强调下，`以下描述的内容可能由于水平问题，导致存在差错，因此很关键的内容都给出参考链接，敬请指导！`

### 问题
---------------

阻塞队列的使用在代码中实在太普遍了，大量的程序的非阻塞模式实现就借助于阻塞队列，譬如提高吞吐量的方式可以采用将请求直接扔到阻塞队列里，然后返回，由其他的线程从阻塞队列中获取请求再进一步处理。经典的生产者和消费者模式，正是阻塞队列的大放光彩之处。

在开发一个小工具时，就正好用到基础代码组件中的阻塞队列，然后风风火火的把代码写完，功能测试过，性能测试没有严格的评估，在测试环境的小量数据下测试结果还很乐观。然而在接受十几G数据的处理过程中，发现性能慢了不少。仔细分析了程序的设计及实现，百思不得其解。万般无赖下，决定替换原有的BlockingQueue，采用自己写的,因为原有的BlockingQueue实现过度封装了。替换之后，性能高下立判。同时，还发现如果消费者(C)和生产者(P)的数量均是1和1个P多个C的情况，性能完全不一样。

就想起以前，参加公司的中间件性能挑战赛时，client端采用多线程往BlockingQueue里写数据，多线程从中消费到最后舍弃阻塞队列，程序处理比赛数据的时间从100s瞬间就到了10s左右。

因此想对阻塞队列更深入的了解点。

### BlockingQueue实现
-------------------

BlockingQueue的实现异常简单，结合`pthread_mutex_t, pthread_cond_t`可以迅速的给出一个稳定靠谱的实现。在陈硕大神的Muduo源码中，给出了经典的实现，包含BoundBlockingQueue和BlockingQueue两种，前者有大小限制，后者无。完整的实现，猛戳下面的链接就可以看到：

* [Muduo  BlockingQueue](https://github.com/chenshuo/muduo/tree/master/muduo/base/BlockingQueue.h)

对于其中有两点需要注意：

* put方法：往队列放一个元素之后，就会notify一次， notify是在mutex释放之前
* take方法：需要通过while循环来判断是否真的有数据了，防止`spurious wakeup`，这也是标准的写法

对于第一点put方法，我们可以先看下下面的博客：

* [Implementing a Thread-Safe Queue using Condition Variables](https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html)

在这篇博客中，介绍了如何实现一个线程安全的队列，即我们说的阻塞队列，其中，对于put（博客中是push）的实现，特意强调了unlock mutex和notify的顺序，抄录在这儿：

    void push(Data const& data)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        the_queue.push(data);
        lock.unlock();
        the_condition_variable.notify_one();
    }

对比Muduo中的实现如下：

    void put(const T& x)
    {
        MutexLockGuard lock(mutex_);
        queue_.push_back(x);
        notEmpty_.notify(); // wait morphing saves us
        // http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
    }

差别在于，先notify，后unlock mutex。Muduo源码中对于这一个问题给出了强有力的链接：

* [Condvars: Signal With Mutex Locked Or Not?](http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/)

在上面的文章中提到了`On some platforms, the OS performs a context switch to the woken thread right after the signal/broadcast operation, to minimize latency`，即signal和broadcast之后，立刻执行context switch，这可能会导致唤醒的线程又阻塞在mutex上，对于此问题，[NPTL](http://en.wikipedia.org/wiki/Native_POSIX_Thread_Library)库对此进行了优化，详情参考上面的文章。但是如果先unlock，可能会唤醒其他阻塞在mutex上的线程，对此情形，分析了两种切实的可能，一种是spurious wake-up，另一种是delay wake-up。

用简单的话总结上面的问题是：`I personally prefer to signal or broadcast while holding the mutex. `

对于第二点，spurious wake-up，上面也提到了一种可能，在`man pthread_cond_wait`上提到了:

* When using condition variables there is always a Boolean predicate involving  shared  variables  associated  with each condition wait that is true if the thread should proceed. Spurious wakeups from the pthread_cond_timedwait() or pthread_cond_wait() functions may occur. Since the return from pthread_cond_timedwait() or pthread_cond_wait() does not imply anything about the value of this predicate, the predicate should be re-evaluated upon such return.

即，唤醒之后，最好再check下predicate。

### 性能分析
----------------------

以Muduo的BlockingQueue作为学习的模板，我们可以尝试分析1个P和1个C，与1个P和多个C的情况下，性能的差别。对此，我写了一个简单的测试程序，[请猛戳这儿](https://github.com/liuan/allan/blob/master/tests/test_block_queue.cc)，具体的用法很简单，可以参考写的脚本，[请猛烈的点这儿](https://github.com/liuan/allan/blob/master/tests/perf_block_queue.sh)

(更新：在阅读了大神写的[多线程队列算法优化](http://www.parallellabs.com/2010/10/25/practical-concurrent-queue-algorithm/)一文之后，重新审视了简单考虑的测试程序，在上面的测试代码中，一个生产者线程负责往阻塞队列里放整数，N个消费者线程负责从中取整数，消费者线程所做的事情，只有一个，不停的去取，没有任何其他的操作，这个模式和平常在业务中用到的模式不一样，需要特意强调下，因此虽说有N个消费者，但由于除了Get之外没有其他操作他们均串行起来了，没有多级流水线的效应，因此不会因多线程获得提速，模式决定了，因此，这个测试程序反应的是锁带来的竞争加剧，上下文切换带来的性能损失。大神在他的文章末尾强调了，`多线程队列算法有多种，大家应根据不同的应用场合选取最优算法`。下面的实验结果也充分的说明了对于多线程的程序的性能问题，不能太直觉，除非建立在对多线程的模式，锁系统等深入的了解，否则最后充分的实际测试。)

在我的VirutalBox CentOS6.6 x86_64 1 core 机器上，跑出来的结果如下：

    1 Producer 1 Consumer time: 1.517s
    1 Producer 2 Consumer time: 2.829s
    1 Producer 3 Consumer time: 4.266s

同样的程序在性能更加强劲的开发机上跑出来的结果却另人大跌眼镜，数据如下：

    1 Producer 1 Consumer time: 4.881s 
    1 Producer 2 Consumer time: 10.135s
    1 Producer 3 Consumer time: 22.497s

从1个Consumer到3个，耗时明显的上来了，直觉是加剧了竞争，多了线程来争抢Mutex。从一台单核的虚拟机切换到多核的强劲物理机，反而耗时更多了，耗的还不是一点儿，明显的反直觉。对于出现的问题，可以使用gperftools来分析。

#### gperftools
---------------

关于gperftools可以先参考这篇文章，[google perftools分析程序性能](http://www.cnblogs.com/GODYCA/archive/2013/05/28/3104281.html)，在编译代码时加上gperftools的动态库，然后按下面的方式处理：

* env CPUPROFILE=./helloworld.prof ./helloworld
* pprof --text helloword helloworl.prof

前者生成取样的数据，后者将数据可视化，我们选择最简易的text格式，对于输出数据的意义，请点击下面的链接：

* [How to find CPU Usage in google profiler](http://stackoverflow.com/questions/6227186/how-to-find-cpu-usage-in-google-profiler)

注意一点，对于多线程程序的profiling，在[gperftools的网页上提到](http://gperftools.googlecode.com/svn/trunk/doc/cpuprofile.html)`In Linux 2.6 and above, profiling works correctly with threads, automatically profiling all threads. In Linux 2.4, profiling only profiles the main thread (due to a kernel bug involving itimers and threads)`。

先观察两个不同机器上，同样程序和参数跑出来的结果对比

     // 1 producer 1 consumer  10000000
     // mutil core machine
      1 Total: 690 samples
      2      237  34.3%  34.3%      237  34.3% __lll_unlock_wake
      3      119  17.2%  51.6%      119  17.2% __lll_lock_wait
      4      108  15.7%  67.2%      108  15.7% __pthread_mutex_lock
      5       61   8.8%  76.1%       61   8.8% pthread_cond_signal@@GLIBC_2.3.2
      6       44   6.4%  82.5%       44   6.4% pthread_mutex_unlock
      7       17   2.5%  84.9%       19   2.8% std::deque::pop_front
      8       11   1.6%  86.5%      148  21.4% choudan::BlockQueue::Get
      9        9   1.3%  87.8%      126  18.3% MutexLock
     10        8   1.2%  89.0%        9   1.3% __gnu_cxx::new_allocator::construct
     11        8   1.2%  90.1%      116  16.8% choudan::Mutex::Lock

    // single core virtual machine
      1 Total: 244 samples
      2 36  14.8%  14.8%   36  14.8% pthread_cond_signal@@GLIBC_2.3.2
      3 32  13.1%  27.9%   32  13.1% pthread_mutex_unlock
      4 24   9.8%  37.7%   24   9.8% __pthread_mutex_lock
      5 19   7.8%  45.5%   19   7.8% _Deque_iterator
      6 18   7.4%  52.9%   53  21.7% MutexLock
      7 16   6.6%  59.4%  120  49.2% choudan::BlockQueue::Get
      8 11   4.5%  63.9%  117  48.0% choudan::BlockQueue::Put
      9 11   4.5%  68.4%   33  13.5% choudan::Mutex::Lock
     10 11   4.5%  73.0%   12   4.9% std::deque::pop_front
     11 10   4.1%  77.0%   10   4.1% std::operator== 


从上面的数据来看，相同的参数下跑出来的结果差异很大，单核虚拟机和多核物理机的差别在于多核物理机上存在几个函数调用十分严重，`__lll_unlock_wake, __lll_lock_wait`，最两个函数就占据了51%的取样结果，而对于单核虚拟机来说，这两个函数甚至连前77%的函数都没进。很可能这就是时间相距如此之大的原因。

从glibc中跟踪`pthread_mutex_lock和pthread_mutex_unlock`两个函数，为什么会导致`__lll_unlock_wake和__lll_lock_wait` 出现的次数相差如此大。

在进一步分析之前，先来了解下Linux下的`Futex(Fast userspace mutex)`。

#### Futex
-----------------

对于Futex的理解，先看一片大神的博客：

* [Linux Futex的设计与实现](http://blog.csdn.net/Javadino/article/details/2891385)

其中强调了Futex的背景，摘抄在这儿，`在传统的Unix系统中，进程间同步机制都是对一个内核对象操作来完成的，这个内核对象对要同步的进程都可见，其提供了共享的状态信息和原子操作。当进程要同步的时候必须要通过系统调用在内核完成。可是研究发现，很多同步状态是无竞争的，即某个进程进入互斥去，到再从互斥区出来这段时间，常常是没有进程也要进入这个互斥区或者请求同一同步变量的。但在这种情况下，进程也要陷入到内核去看有没有竞争者，退出的时候还要陷入内核去看有没有进程等待在同一变量上。这些不必要的系统调用（或者说陷入内核）造成了大量的性能开销。`

上面这段话就道出了Futex的来由，减少不必需要的内核陷入。Glibc中使用的线程库NPTL所采用的同步方式，大多构造于futex上。对于Futex的原理，同样可以在上面的文章中略之一二。

在没有竞争的情况下，不会陷入到内核中去，有竞争时，这会调用__lll_lock_wait和__lll_lock_wake陷入到内核中。根据glibc NTPL代码情况和我们实际的数据，可以大致的解释上面提到的现象，在单核和多核下同样程序的差异。

在单核上，程序实际上是高度串行的，在线程A获得Mutex之后，取下东西或放入东西，然后释放，又开始下一次，结果显示，两个线程在锁上的碰撞很低，很少陷入到内核中去wake。那么很可能是在线程的执行时间片上，程序已经完成了加锁，获取数据，解锁的整个过程，在调度到线程B执行时，就没有锁的竞争，直接完成操作。这是在单核下，线程获取锁的概率很高，并享受到Futex带来的性能提升。而在多核上，每个线程并行的执行，上来就开始竞争锁，陷入到内核中的概率要显著增加，就是profiler上lock和wait两个函数的比例较高。

据此推测，如果在多核机器上，将线程均绑定到同一个CPU上，性能应该会改善不少，并且profiler之后的结果中不会出现高频率的__lll_lock_wait和__lll_lock_wake。根据此思路，在代码中添加了下面几行：


    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);

简单的直接将线程绑定到0号CPU上，观察的结果如下，多次运行取平均：

    1 Producer 1 Consumer time: 1.560s

然后再查看profiler的结果，

      Total: 155 samples
      34  21.9%  21.9%       34  21.9% pthread_mutex_unlock
      23  14.8%  36.8%       23  14.8% __pthread_mutex_lock
      21  13.5%  50.3%       21  13.5% pthread_cond_signal@@GLIBC_2.3.2
       7   4.5%  54.8%       42  27.1% choudan::Mutex::Unlock
       6   3.9%  58.7%        6   3.9% _Deque_iterator
       6   3.9%  62.6%       79  51.0% choudan::BlockQueue::Get
       6   3.9%  66.5%        8   5.2% choudan::Condition::Notify
       5   3.2%  69.7%        6   3.9% __gnu_cxx::new_allocator::construct
       4   2.6%  72.3%       27  17.4% choudan::Mutex::Lock
       4   2.6%  74.8%        6   3.9% std::deque::pop_front
       4   2.6%  77.4%       43  27.7% ~MutexLock
       3   1.9%  79.4%       31  20.0% MutexLock
       3   1.9%  81.3%        3   1.9% __gnu_cxx::new_allocator::destroy

从上面profiler的结果看，也符合我们的预期。这些问题均是由于锁导致的竞争加剧，获取锁的概率导致性能的变化的。对于其中的问题更深入的探索，可以先操作下面的两篇博客。

* [searchdb: Pthreads mutex vs Pthreads spinlock](http://www.searchtb.com/2011/01/pthreads-mutex-vs-pthread-spinlock.html)
* [alexonlinux: pthread mutex vs pthread spinlock](http://www.alexonlinux.com/pthread-mutex-vs-pthread-spinlock)

第一篇博客是来自于淘宝搜索技术博客，讨论的是pthread和mutex多种场景的下带来的性能表现，分析的较透彻，测试情况中出现的现象有些类似，可以对比观察分析，加深对pthread_mutex_t和pthread_spinlock_t的理解。

后一篇博客，主要是简单的比较了mutex和spinlock，真正的价值在于文章末尾的讨论，强调了一句`the reason why we wanted to use spinlocks in the first place we wanted to avoid context switch. The truth is that spinlocks have no meaning on single-processor machines. This is because on SP computers, two threads cannot run at the same time for real. It is simulated by scheduler and the operating system, using preempts. Preempt in itself implies context switch and putting the thread asleep.`，使用spinlock的关键是避免context switch，在具体的场景下，需要在spinlock和context switch二者的代价中抉择权衡。


### 优化
-------------

对于BlockingQueue的优化，这个我没有深入的查看资料，在最初的直觉上，必然是减少锁的竞争。对于如何减少锁的竞争，有几种不同的考量。一是BlockingQueue本身的性能优化，下面将贴出大神写的文章，很实用，有理有据；二是，与具体的业务结合起来，将数据分散开来，减少竞争压力，例如在Memcached的hash结构体，就存在两种粒度的锁，全局的锁和以bucket为单位进行划分的锁，根据具体的逻辑来确定使用的锁。换到BlockingQueue中来，就可以考虑将数据进行hash，划分到不同的BlockingQueue。

对于多线程队列的优化，参考下面的文章，如何从队列实现的角度出发来优化，据作者提到的，32个线程下，性能能提升11%。具体做法以及思想来源，大家直接阅读大神的文章吧，质量更高。

* [多线程队列的算法优化](http://www.parallellabs.com/2010/10/25/practical-concurrent-queue-algorithm/)

对于数据分散的做法，我应用到文章开头提到的小工具上，性能也是明显改善。当然，这个问题还需要更加具体的探讨。


### 总结
--------------

从BlockingQueue出发，发现了这些问题，由于涉及到了很多更加系统化的知识，所以以上的过程可能存在理解差错的，欢迎指出和讨论。对于BlockingQueue的使用需要明确具体的场景，如上面的测试用例，不管是几个consumer线程，都是串行的执行，并没有所谓的并行，多线程还带来了效率的降低。我们利用多线程提升性能的时候，往往考虑的是流水线般的操作，当线程阻塞到某个事件上，可以去做其他的事情，文件描述符的非阻塞模式，这样真的并行起来。

对于mutex和spin，在单核和多核的场景下，同样需要考虑清楚，系统行为很可能不如你简单所想。

很多的荣耀属于晖哥，谢谢晖哥的指导和启发。

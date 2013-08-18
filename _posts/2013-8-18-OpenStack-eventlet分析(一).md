---
layout: post
category: OpenStack
tags: OpenStack
---

Eventlet库在OpenStack服务中上镜率很高，尤其是在服务的多线程和WSGI Server并发处理请求的情况下，深入了解eventlet库是很必要的。Eventlet库是由second life开源的高性能网络库，从Eventlet的源码可以知道，其主要依赖于两个关键的库：

> 1.greenlet
> 2.select.epoll (或者epoll等类似的库)

greenlet库过程了其并发的基础,eventlet库简单的对其封装之后，就构成了GreenTread。select库中的epoll则是其默认的网络通信模型。正由于这两个库的相对独立性，可以从两个方面来学习eventlet库，首先是greenlet。

### greenlet

在介绍GreenThread之前，可以先参考下面三篇文章，对greenlet来个迅速入门。

> 1.[greenlet官方文档](http://greenlet.readthedocs.org/en/latest/)
> 2.[greenlet官方文档翻译](http://gashero.yeax.com/?p=112)
> 3.[greentlet原理详细介绍](https://code.google.com/p/libhjw/wiki/notes_on_greenlet#greenlet_not_stated)

还补充一篇文档，写的很好。

> [openstack nova基础知识之eventlet](http://blog.csdn.net/hackerain/article/details/7836993)

通过这三篇循序渐渐的文章，大概可以了解到greenlet是一个称为协程(coroutine)的东西，有下面几个特点。

> 1.每个协程都有自己的私有stack及局部变量
> 2.同一时间内只有一个协程在运行，故无须对某些共享变量加锁
> 3.协程之间的执行顺序，完成由程序来控制


总之，协程就是运行在一个线程内的伪并发方式，最终只有一个协程在运行，然后程序来控制执行的顺序。可以看下面的例子来理解上面的意思。

    import greenlet

    def test1(n):
        print "test1:",n
        gr2.switch(32)
        print "test1: over"

    def test2(n):
        print "test2:",n
        gr1.switch(23)
        print "test2: over"


    greenlet = greenlet.greenlet
    current = greenlet.getcurrent()
    gr1 = greenlet(test1,current)
    gr2 = greenlet(test2,current)
    gr1.switch(2)

这段程序的执行结果如下：

    test1: 2
    test2: 32
    test1: over

整个程序的过程很直白，首先创建两个协程，创建的过程传入了要执行的函数和父greenlet（在前面给出的三个链接中有详细介绍），然后调用其中的一个协程的switch函数，并且传递参数进去，就开始执行test1，然后到了gr2.switch(32)语句，切换到test2函数来，最后又切换回去。最终test1运行结束，回到父greenlet中，执行结束。这个过程就是始终只有一个协程在运行，函数的执行流由程序自己来控制。这个过程在上面的链接中描述的更加具体。

### GreenThread

那么在eventlet中对greenlet进行了简单的封装，就成了GreenThread,并且上面的程序还会引来一个问题，如果我们想要写一个协程，那到底该如何来控制函数的执行过程了，如果协程多了，控制岂不是很复杂了。带着这个问题来看eventlet的实现。

在介绍下面的内容之前，先贴出[eventlet官方的文档](http://eventlet.net/doc/basic_usage.html)，这个上面详细的介绍了该如何来使用eventlet库。我们从其中选出一个接口来分析。spawn函数，调用该函数，将会使用一个GreenThread来执行用户传入的函数。函数具体接口如下：

    def spawn(func, *args, **kwargs):

参数很清晰,想要执行的函数以及函数的参数。该函数实际上只做了三件事,最后返回创建的greenthread，因此该函数相比于spawn_n可以，得到函数调用的结果。

    hub = hubs.get_hub()
    g = GreenThread(hub.greenlet)
    hub.schedule_call_global(0,g.switch,func,args,kwargs)
    return g

第一，我们要先知道hubs的作用，在[eventlet的官方文档有介绍](http://eventlet.net/doc/hubs.html),在greenlet的官方文档开始就是我们可以自己构造greenlet的调度器，那么hub的第一个作用就是greenthread的调度器。另外一个作用于网络相关，所以hub有多个实现，对应于epoll，select，poll，pyevent等，我们先看前面的第一个作用。

hub在eventlet中是一个单太实例，也也就是全局就这有这一个实例，其包含一个greenlet实例，该greenlet实例是`self.greenlet = greenlet(self.run)`,这个实例就是官方文档说的MAINLOOP，主循环，更加具体就是其中的run方法，是一个主循环。并且该hub还有两个重要的列表变量，`self.timers 和 self.next_timers`，前者是一个列表，但是在这个列表上实现了一个最小堆，用来存储将被调度运行的greenthread，后者，用来存储新加入的greenthread。

第二，创建一个GreenThread的实例，greenthread继承于greenlet，简单封装了下，该类的构造函数只需要一个参数，父greenlet，然后再自己的构造函数中，调用父类greenlet的构造函数，传递两个参数，GreenTread的main函数和一个greenlet的实例。第二代码就知道，hubs中作为MAINLOOP的greenlet是所有先创建的greenthread的父greenlet。由前面介绍greenlet的例子中，我们可以知道，当调用该greenthread的switch方法时，将会开始执行该才传递给父类的self.main函数。

第三，然后单态的hub调用schedule_call_global函数，该函数的作用可以看其注释,用来调度函数去执行。

    """Schedule a callable to be called after 'seconds' seconds have
    328         elapsed. The timer will NOT be canceled if the current greenlet has 
    329         exited before the timer fires.
    330             seconds: The number of seconds to wait.
    331             cb: The callable to call after the given time.
    332             *args: Arguments to pass to the callable when called.
    333             **kw: Keyword arguments to pass to the callable when called.
    334         """
                t = timer.Timer(seconds, cb, *args, **kw)
    340         self.add_timer(t)
    341         return t

注释中提到的timer是指，传递进来的参数会构造成Timer的实例最后添加到self.next_timer列表中。注意在spawn中传递进来的g.switch函数，如果调用了这个g.switch函数，则触发了它所在的greenthread的运行。

这三步结束之后，对spawn的调用就返回了，然而现在只是创建了一个GreenThread,还没有调度它去执行，最后还需要再返回的结果上调用`g.wait()`方法，这样就开始GreenThread的神奇之旅了。

我们看GreenThread的wait方法的具体代码,

    def __init__(self, parent):
        greenlet.greenlet.__init__(self, self.main, parent)
        self._exit_event = event.Event()
        self._resolving_links = False

    def wait(self):
        """ Returns the result of the main function of this GreenThread.  If the   
        result is a normal return value, :meth:`wait` returns it.  If it raised
        an exception, :meth:`wait` will raise the same exception (though the 
        stack trace will unavoidably contain some frames from within the
        greenthread module)."""
        return self._exit_event.wait()

wait方法调用了Event实例的wait方法,就是在这个wait函数中，调用了我们前面提到的单态实例hub的switch方法，然后该switch真正的去调用hub的self.greenlet.switch(),我们已经所过该greenlet是所有调用spwan创建的greenlet的父greenlet，该self.greenlet在初始时传递了一个self.run方法，就是所谓的MAINLOOP。最终，程序的运行会由于switch的调用，开始run方法中的while循环了，这是多线程开发者最熟悉的while循环了。

在该while循环中，就对self.next_timers中的timers做处理。

    def prepare_timers(self):
        heappush = heapq.heappush
        t = self.timers
        for item in self.next_timers:
            if item[1].called:
                self.timers_canceled -= 1
            else:
                heappush(t, item)
        del self.next_timers[:]

首先处理next_timers中没有被调用的timers，push到最小堆中去，也就是时间最小者排前面，越先被执行。然后将所有已经调用了的timer删除掉，这是不是会有一个疑问：如果删除了的timers没有运行结束，那么下次岂不是没有机会再被调度来运行了。再看了greenthread.py中的sleep函数之后，就会明白。

加入到heap中的timers这会按照顺序开始依次遍历，如果到了他们的执行时间点了，timer对象就会直接被调用。看下面的代码

    def fire_timers(self, when):
        t = self.timers
        heappop = heapq.heappop
        while t:
            next = t[0]
            exp = next[0]
            timer = next[1]
            if when < exp:
                break
            heappop(t)
            try:
                if timer.called:
                    self.timers_canceled -= 1
                else:
                    timer()
            except self.SYSTEM_EXCEPTIONS:
                raise
            except:
                self.squelch_timer_exception(timer, sys.exc_info())
                clear_sys_exc_info()

Timer对象重载了__call__方法，所以可以直接调用了，timer被调用之后，我们前面知道，传递进来的是g.switch，在timer中就是调用了该switch函数，直接触动了greenthread的执行，此时，我们自定义的函数就可以被执行了。

我们知道，如果我们自定义的函数要运行时间很长，怎么办，其他的greenthread则没有机会去运行了，在[openstack nova官方文档中介绍thread](http://docs.openstack.org/developer/nova/devref/threading.html)中也提到这个问题，此时我们需要在自己定义的函数中调用`greenthread.sleep(0)`函数，来进行切换，使其他的greenthread也能被调度运行。看看greenthread.sleep函数的代码。


    def sleep(seconds=0):
        """Yield control to another eligible coroutine until at least *seconds* have
        elapsed.

        *seconds* may be specified as an integer, or a float if fractional seconds
        are desired. Calling :func:`~greenthread.sleep` with *seconds* of 0 is the
        canonical way of expressing a cooperative yield. For example, if one is
        looping over a large list performing an expensive calculation without
        calling any socket methods, it's a good idea to call ``sleep(0)``
        occasionally; otherwise nothing else will run.
        """
        hub = hubs.get_hub()
        current = getcurrent() # 当前正在执行的greenthread，调用这个sleep函数
        assert hub.greenlet is not current, 'do not call blocking functions from the mainloop'
        timer = hub.schedule_call_global(seconds, current.switch)
        try:
            hub.switch()
        finally:
            timer.cancel()

从该sleep函数可以知道，我们又重新调用了一遍hub.schedule_call_global函数，然后直接调用hub.switch，这样在运行的子greenlet中，开始触发父greenlet（也就是MAINLOOP的greenlet）的执行，上次该greenlet正运行到` fire_timers 的timer()`函数处，此时父greenlet则接着运行，开始新的调度。

至此，调度的过程就大致描述结束了。

greenthread中其他的函数都基本同样，如果我们的函数只是简单的进行CPU运行，而不涉及到IO处理，上面的知识就可以理解eventlet了，然而，eventlet是一个高性能的网络库，还有很大一部分是很网络相关的。在留给下次。

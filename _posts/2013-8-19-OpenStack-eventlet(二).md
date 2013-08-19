---
layout: post
category: OpenStack
tags: eventlet
---

上一篇博客[OpenStack-Eventlet分析(一)](http://www.choudan.net/2013/08/19/OpenStack-eventlet分析%28一%29.html)以eventlet库中的spawn函数为代表，详细的介绍了spawn函数的运行过程。最终的重点是在hubs用来进行调度的一面，而hubs关于网络的一面还没有涉及。然而，上一篇只专注于每一行代码的执行流程了，而没有eventlet用来调度greenthread的框架分析，这样容易导致阅读源码会出现的一个常见问题，只见树木不见树林。所以在这一篇，再详细分析eventlet用来调度greenthread的框架问题，下次再将其网络部分补齐，构成一个整体。

首先来看一个例子。

    from eventlet import hubs
    from eventlet import greenthread

    def tellme(secret):
        print "a secret:",secret
        

    hub = hubs.get_hub()
    hub.schedule_call_global(0,tellme,"you are so beautiful")
    hub.switch()
    #greenthread.sleep(0)

在这个例子中，没有使用提供的spawn函数，而是直接使用hub来调度来运行我们定义的tellme函数，结果很显然，打印完a secrete: you are so beautiful 之后，并没有结束。我们在上一篇文章中提到，hub是单态的，存在一个greenlet，作为`MAINLOOP`，使用hub的switch函数来开始这个MAINLOOP的运行，也就是说，MAINLOOP的循环运行，需要触发。在MAINLOOP中完成调度，执行tellme然后就返回到MAINLOOP中继续运行了。

当我们使用`greenthread.sleep(0)`来代替上面的`hub.switch()`,程序就能正常结束了。sleep函数将自己所在的greenlet的switch函数加入到hub的调度列表中，然后调用switch来触发MAINLOOP的调度。我们知道如果一个greenthread运行结束了，那么就会回到父greenlet来，正是因为如此，sleep函数中向hub添加的`current.switch`函数运行之后，就结束了sleep函数的整个内容，返回到父greenlet来，父greenlet正式我们自己写的这片代码。

从上面的例子和spawn的例子对比，都是使用hub来调度一个函数的运行，差别在于，上面的例子，是调度一个普通函数运行，spawn在于调度一个greenlet的switch函数运行。这就引入了一个基本问题，hub调度的是什么？

### Timer

对于任何传入到hub的函数，首先就会封装成Timer，代表了该函数将会在多久之后被执行。实际上，我们知道了，hub调度的是一个个Timer，不管这个Timer中存储的是什么函数，普通的函数还是greenlet的switch函数，都是一样的被处理。对于普通函数，我们可以让等待一定时间运行，我们关注的函数hub如何来调度greenthread。这才是重点。

### event

再来看一个例子。

    from eventlet import event
    from eventlet.support import greenlets as greenlet
    from eventlet import hubs
    import eventlet

    evt = event.Event()

    def waiter():
        print "about to wait"
        result = evt.wait()
        print 'waited for,',result


    hub = hubs.get_hub()
    g = eventlet.spawn(waiter)
    eventlet.sleep(0)
    evt.send('a')
    eventlet.sleep(0)

在[eventlet的官方上有这段代码](http://eventlet.net/doc/modules/event.html)，引入这段代码是因为event在调度greenthread中有重要的作用。上面的代码运行结果如下：

    about to wait
    waited for,a

首先解释下，调用spawn会创建一个greenthread放入到hub中，然后使用sleep(0)从当前的greenlet切换到刚才创建的greenthread，就开始执行waiter函数，打印第一行。然后函数就在此wait了，我们前面介绍了wait会触发hub的switch方法，回到MAINLOOP的循环中，由于在每一次循环都将next_timer清空了，所有要执行的timer都添加到self.timer这个小堆中去了。在MAINLOOP中，由于这个包含timer的wait已经被执行过一次，所以下次循环时不会再执行了，sleep函数就让程序切换到了我们写的代码上来，接着运行evt.send('a'),这一行同样触发了hub的调度，接着运行到waiter阻塞的地方，我们发现，这儿send有一个很关键的作用，用来在不同的greenthread中传递结果。所以后面紧接着打印了waited for,a。最后一句sleep则从MAINLOOP的空循环中切回到我们的代码尾，然后结束。

通过event，就明白了event可以用来再不同的greenthread中进行值的传递。官方文档介绍了，event和队列类似，只是event中只有一个元素，send函数能够用来唤醒正在等待的waiters，是不是和线程中的诸多概念相似了。

### 总结

我们回过头来看整个hub作为调度模块的结构，hub调度对象是Timer实例，只是有的timer实例封装了greenthread的switch函数，从而切换到greenthread的执行。不同的greenthread中进行通信，这需要用来event，每个greenthread封装一个event实例，event完成对本身greenthread的结果传递。而我们普通使用的spawn系列函数则是整个调度系统提供对外的api，使用该api，则可以将我们的任务作为一个greenthread添加到hub中，让它调度。至此，可以大致看到eventlet的调度框架。并且后面将提到的greenpool则是一个greenthread的池，使用也差不多了。



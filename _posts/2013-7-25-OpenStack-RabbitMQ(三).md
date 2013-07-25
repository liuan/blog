---
layout: post
category: OpenStack
tags: OpenStack RabbitMQ
description: （序列三）:主要分析OpenStack Nova中对RabbitMQ的运用
---

### OpenStack RabbitMQ

下面将描述的内容都是基于OpenStack Folsom版本代码。参考这两篇博文，[这儿](http://blog.csdn.net/hackerain/article/details/7888672)和 [这儿](http://blog.csdn.net/hackerain/article/details/7880121)

文章中后面两张图片是visio画的，如果看不清除，可以直接通过文章后给出的链接下载。

在这一部分，重点分析nova（以下均以nova-compute为例）中是如何使用RabbitMQ来实现不同服务之间的通信的，rpc调用到底是怎么实现的，中间涉及到哪些python新知识和设计模式。

在nova服务启动的过程中，会有如下一些与rpc相关的代码，具体截图如下。

<img src="/assets/img/OpenStack_rabbit_code.png" atl="OpenStackRabbitMQCode01">

这八行代码就是我们敲开rpc实现的入口。这六行代码分为三个步骤，一个是创建一个与RabbitMQ Server的连接；一个是rpc_dispatcher，简单的理解是一个callback函数；一个是创建消费者，典型的队列中的生产者和消费者模式。

首先分解rpc.create_connection部分。

<img src="/assets/img/OpenStack_rabbit_rpc.png" atl="OpenStackRabbitMQRPC" width="700px">

上图描绘了使用nova中rpc的各种类之间的关系图。

从412行代码rpc.create_connection开始，调用rpc模块中__init__文件的create_connection 函数，该函数会判断到底使用什么样的rpc_driver，这个已经在配置文件中配置好了，以下都假设使用impl_kombu driver。

然后调用impl_kombu模块中的create_connection函数，该函数随即调用ampq.py文件中的create_function，再调用时传递的一个参数是connection_pool,此时就创建这个connection_pool,若Connection类中的类属性pool没赋值，则创建，若已经复制，则返回。这样还确保了一个全局唯一的pool，有点单态模式的味道。通过上图，已经知道pool是使用eventlet中的pools.pool来实现的，pool中存储的就是connection实例。

在amqp.py文件中的create_connection函数就直接返回一个ConnectionContext的实例，该实例在初始化时，会创建一个connection实例，如果ConnectionContext初始化时的参数pooled为true，就从pool中取出，否则就创建，该connection实例就完成了对RabbitMQ Server的连接。ConnectionContext实例重载了__enter__和__exit__函数，就可以使用with语句了。

总之，一句话，第412行代码返回一个ConnectionContext的实例。

从前面我们已经知道self.manager是nova.compute.manager.ComputeManager的实例。

<img src="/assets/img/OpenStack_rabbit_manager.png" atl="OpenStackRabbitMQManager" width="700px">

上图是manager相关类的关系图，细看这些关系，ComputeManager继承于manager，调用manager的create_rpc_dispatcher会返回一个RpcDispatcher的实例，该实例的存在一个callbacks的列表，这个callbacks存在一个元素就是manager这个实例。RpcDispatcher类还存在一个dispatch方法，该方法会调用msg指定的函数，这个函数是manager实例的一个方法。

总之，self.manager.create_rpc_dispatcher返回的是RpcDispatcher这么一个用来回调的实例。
Nova-compute服务启动的过程中，要创建三个持久的consumer，两个topic consumer，差别在于绑定的队列的key（topic:compute; topic:compute.gb07）一个fanout consumer。

创建consumer，最终会由Connection中的create_consumer函数来完成。第一篇文章中介绍过RabbitMQ具有不同类型的Exchange，下面这张图就描述代码中关于Exchange的consumer关系。

<img src="/assets/img/OpenStack_rabbit_Consumer_Code.png" atl="OpenStackRabbitMQConsumerCode" width="700px">

### 图片链接

* [RPC.vsd](/assets/img/openstack_rpc.vsd)
* [Manager.vsd](/assets/img/openstack_manager.vsd)
* [consumer.vsd](/assets/img/openstack_consumer.vsd)

### 下一节

对OpenStack 中对RabbitMQ的使用做一个简单的总结

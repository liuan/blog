---
layout: post
category: OpenStack
tags: OpenStack RabbitMQ
description: （序列四）:简单的总结
---

### 总结

总结下openstack中rpc的机制。

总之，RabbitMQ 提供的是一个消息队列，那么很自然的使用这个RabbitMQ的方式是经典的生产者-消费者模式。故存在一个或多个生产者publisher，向队列中发送消息，同样存在一个或多个消费者consumer来读取消息。由于需要发送的消息很多，发往的不同消费者也多，因此需要将消息发送到不同的队列，这样存在一个exchange来将消息路由到不同的队列，队列需要一个key，则消息需要一个对应的key。所以使用RabbitMQ需要这几个基本的对象queue，exchange（它们均由RabbitMQ来维护）。因此，还需要存在一个connection来连接RabbitMQ Server。同时由于需要多个connection，故使用eventlets.pools.pool来维护一个全局的pool，来管理这些connection。

Publisher通过connection将消息按照method和args的方式发送到RabbitMQ Server对应的队列上，Consumer则轮询该队列，有了msg，则通过connection将该msg取出来，解析出msg的method和args，由于consumer在创建时，已经注册了一个callback函数，由于不同的msg需要调用不同的method，为了解决这个问题，故使用了一个RpcProxy来代理这个callback函数。所以consumer取出msg后，调用RpcProxy来处理msg，最后完成msg指定方法的调用。由于msg特别多，执行的任务时间也长，故是使用greenthread来执行每个method。

除了上面的大致流程分析之外，还需要考虑下面这些问题。

* API版本的处理技巧
* 中间涉及到的较多context，如何处理的
* 多线程是如何避免死锁问题的
* 这套rpc机制是如何实现对不同的ampq实现方式的支持

当然，这些都是技术层面的技术问题，并没有解释出为什么使用RabbitMQ来进行通信，其他分布式文件系统中也需要RPC来完成远程通信，却不是选择得消息队列方式，这些有什么区别。还有RabbitMQ运用在这个平台上，是否还有哪些欠缺的地方，这些更需要了解！



---
layout: post
category: OpenStack
tags: OpenStack RabbitMQ
description: （序列二）:主要介绍OpenStack中对RabbitMQ的使用
---

### OpenStack RabbitMQ

此文基本上是从OpenStack官方网站翻译而来，[链接在此](http://docs.openstack.org/developer/nova/devref/rpc.html)

Nova在RabbitMQ的基础上实现了RPC（两种形式：一，request+reponse,二：one way，前者是rpc.call，后者是rpc.cast）。每个nova服务在初始化是都会创建两个队列，其中一个接收的消息的routing key形式是’NODE-TYPE.NODE-ID’，例如compute.gb06，另一种routing key形式是‘NODE-TYPE’，例如compute。当API请求需要路由到具体的节点时，则发送消息到前者类型的队列。

每个nova服务都会连接到rabbitmq server来，创建两个消息队列，创建两个Topic consumer. 依赖于服务自身的特点，服务可能使用这个队列作为调用者或者作为工作者。服务作为调用者可以使用rpc.call或者rpc.cast来发送消息到队列，作为工作者，从队列接收消息处理，若是rpc.call则返回response。

![OpenStack_RabbitMQNode](/assets/img/OpenStack_RabbitMQNode.png)

补充，在上图中，name:control_exchange，在nova中，该control_exchange的值是nova，type为topic。
上图显示了一个rpc.call的过程。其中存在多个概念：

* Topic Publisher：该对象存始于服务调用rpc.call时，结束与消息发送出去，生命周期短暂。用来将消息发送到队列系统中，每个topic publisher连接于具有topic的exchange。

* Direct Consumer：该对象存在于服务调用rpc.call之后，专门用来接收rpc.call调用返回的结果，接收结果后销毁。每个direct consumer连接到一个特定msg_id的direct exchange，中间连接一个特定msg_id的队列，该msg_id是一个uuid，由rpc.call发送消息的msg_id决定。

* Topic Consumer：该对象始于服务的创建，和服务一直共同存在着。用来接收消息队列中的消息。每个Topic Consumer连接到同一个topic的exchange，中间倚靠一个独占或者一个共享的队列连接。每个worker都有两个consumer，一个连接key为topic的队列，一个连接key为topic.host的队列。

* Direct Publisher：该对象始于rpc.call调用，当response返回时创建，结束与消息的发送，连接这特定msg_id的direct类型的exchange。

由此可见，当调用rpc.call时，需要生成msg_id，指定topic与host，由topic publisher将消息发送到key为topic.host的队列，topic consumer接收到之后，处理生成结果，由Direct Publisher直接发送过去，direct consumer接收到返回结果，完成调用。


下图是rpc.cast的过程，与rpc.call相比，差别在于消息经过的队列是key为topic的共享队列，没有返回结果，过程相较简单多了。

![OpenStack_RabbitMQNode2](/assets/img/OpenStack_RabbitMQNode2.png)

Nova使用Kombu来连接RabbitMQ server。Kombu是一个Python库，实现了标准的AMQP 0.8版的协议。当使用Kombu时，调用者和工作者都需要一些参数来初始化对RabbitMQ Server 的连接对象。Hostname，userid，password，virtual_host，port
除了上面一些，还有三个默认变量：
* Insist：默认值false，对server的一个持续的connection
* Connect_timeout：默认是没有timeout的
* SSL：使用SSL来连接server，默认为false

作为consumer，还需要更多的参数，如下：
* Connection：连接rabbitmq server的对象
* Queue，Exchange，Routing_Key
* Durable：默认为true，决定了queue和exchange的持续性，如果是durable queue 和exchange，那么当rabbitmq server重启之后，这些queue 和exchange都在。
* Auto_delete：默认false，如果为true，当所有的queue都没了，和他们绑定的exchange就结束。
* Exclusive：独占的队列只能被当前连接的consumer使用，当设置了Exclusive，这预示着auto_delete
* Exchange_type，auto_ack(默认为false，收到消息后的自动回答)，no_ack（关闭了回复，牺牲了稳定性换来了性能）)

### 下一节

主要在代码层面分析OpenStack Nova如何使用RabbitMQ

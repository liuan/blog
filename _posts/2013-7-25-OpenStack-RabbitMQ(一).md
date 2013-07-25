---
layout: post
category: OpenStack
tags: OpenStack RabbitMQ
description: （序列一）:主要介绍RabbitMQ的基础知识
---

### RabbitMQ

以下内容都是网上摘抄而来，时间久远了，没有保存链接，一时找不到原文出处了。

MQ是一种应用程序和应用程序之间通信的方法，应用程序通过写和检索出入队列的针对应用程序的数据或消息来通信。消息传递指的是程序之间通过在消息中发送数据进行通信，而不是通过直接调用彼此来通信。

MQ是一个典型的消费者-生产者模型。RabbitMQ是一个在AMQP基础上完整的可复用的企业消息系统。

RabbitMQ有四个重要的基本概念：

* Virtual Host:虚拟主机

* Exchange: 又称交换机，主要接受和路由消息，将消息发送给消息队列，具有三种类型，direct,fanout,topic

* Queue: 消息队列是一个特定的缓冲区，代表一组应用程序保存消息

* Bingding: 将一个特定得Exchange 和特定的Queue绑定，绑定关键字成为BandingKey.

##### Exchange 

![ExchangeDirect](/assets/img/rabbitmq-exchange.png)

##### Exchange Direct

![ExchangeDirect](/assets/img/rabbitmq-direct.png)
![ExchangeDirect](/assets/img/rabbitmq-direct2.png)

##### Exchange Fanout

![ExchangeDirect](/assets/img/rabbitmq-fanout.png)

##### Exchange Topic

![ExchangeDirect](/assets/img/rabbitmq-topic.png)
![ExchangeDirect](/assets/img/rabbitmq-topic2.png)
![ExchangeDirect](/assets/img/rabbitmq-topic3.png)

### 下一节

介绍OpenStack中Nova是如何使用RabbitMQ的。

---
layout: post
category: OpenStack
tags: OpenStack
description: 介绍Restful API，以及OpenStack中Restful API的设计
---

### Restful API

最近听到很多平台说在提供Restful API，例如OpenStack就是设计这样的API，淘宝的飞天对外提供的也是Restful API。那么到底什么才是Restful API了，带着这些问题在网上翻阅了一遍，记录在此。

首次大家可以参考这个链接来[掌握Restful概念](http://www.ruanyifeng.com/blog/2011/09/restful.html),文章开头写的很有启发性，`网站即软件`。Rest是由Roy Thomas Fielding大神在他博士论文中提出的,他在自己的论文摘要中写道。

> 本文研究计算机科学两大前沿----软件和网络----的交叉点。长期以来，软件研究主要关注软件设计的分类、设计方法的演化，很少客观地评估不同的设计选择对系统行为的影响。而相反地，网络研究主要关注系统之间通信行为的细节、如何改进特定通信机制的表现，常常忽视了一个事实 ，那就是改变应用程序的互动风格比改变互动协议，对整体表现有更大的影响。我这篇文章的写作目的，就是想在符合架构原理的前提下，理解和评估以网络为基础的应用软件的架构设计，得到一个功能强、性能好、适宜通信的架构。

Rest即`Representational State Transfer`,中文大家翻译成表述性状态转移。上面链接出来的博客解释的很清晰，Restful最大的两个特点是：

* 无状态的： 客户端与服务器端的交互请求之间是无状态的，每个请求都必须包含理解请求的所必须信息，没有上下文，例如服务器重启了，也不影响这一次请求

* 资源：在Restful的角度看来，Web上的所有东西本质上都是资源，例如一张照片，一条记录等等，每个资源都使用URI得到唯一的地址。对于资源，可以使用四个HTTP动词来操作,GET POST DELETE PUT。使用HTTP这四个动词就可以很好的处理应用程序需求，如CRUD操作。

<img src=""/>

举一个具体的例子来展示到底是如何利用HTTP 头中的这四个动词来完成CRUD操作。

<img src=""/>

需要深刻的理解这两点,尤其是资源两字,精髓所在，然后再看OpenStack中的API设计就更加清晰明了。

注意，上面给出的博客还提到了Restful API设计是存在的两点误区，URI中不能包含动词，URI中包含版本，大家具体的看那篇博文。

### Routes

对Restful API有了一个基础的了解，那么我们来看通过URL是怎样映射到具体的应用程序操作函数上了。在OpenStack中的API Daemon都会有一个Router类，来构建资源与URL直接的映射关系，完成从接收到URL请求然后映射到具体的函数上执行的整个过程。

这就要了解Python 中的Routes模块。

[Routes](http://routes.readthedocs.org/en/latest/introduction.html) 是一个python重新实现的Rails routes system，用来将urls映射到应用具体的action上，相反的，还生成url。由于Routes是Rails routes system的python实现，故从rails的restful入手，能很好的理解Routes库。

首先看一个简单的例子，就明白routes的作用，

> 例如浏览器接收到下面的HTTP请求，
> GET /instances/1
> Raisl的路由请求则负责将此请求解析后dispatch来代码中的具体某个函数，完成调用，例如返回虚拟机的信息。

### OpenStack Restful API

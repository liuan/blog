---
layout: post
category: OpenStack
tags: OpenStack
---

### 前言

前面一篇文章简单混乱的介绍了[OpenStack Restful API](http://www.choudan.net/2013/07/28/OpenStack-Restful-API.html),可以了解到OpenStack 中API的设计方式，还有一篇文章详细全面的介绍了[Nova API的启动](http://www.choudan.net/2013/07/28/OpenStack-paste-deploy介绍.html),这篇文章只描述了服务是如何一步步运行起来的，侧重在WSGI Server使用paste deploy模块的方式，Server起来之后再怎样处理各种http 请求就没有介绍了。

这一篇文章就重点介绍，OpenStack Nova中是如何定义各种API的，怎样完成HTTP Request最终路由到具体的某个函数调用上的。


### 概念

在paste deploy一文中已经知道了，Nova API的启动最终会运行到APIRouter class的构造函数中来，由该函数发散出去，就完成了整个Nova API的启动，该构造函数主要做了下面几件事。

> 1.创建一个ExtensionManager,用来加载所有的扩展资源(Extension) 
> 2.调用routes的Mapper.resource方法完成对核心资源的定义，建立路由规则
> 3.调用routes的Mapper.resource方法完成对扩展资源的定义，也建立路由规则
> 4.将扩展资源对核心资源扩展的action或extends注册，其实action,extends就是该资源的某个方法


Nova API service 接收到HTTP请求之后，处理过程主要分为四个阶段:

> 1.首先通过WSGI Server将HTTP request封装成wsgi request 
> 2.使用api-paste.ini文件中定义的Filter对wsgi request进行处理
> 3.处理完毕后，就根据mapper中的记录，将不同的请求路由到不同的WSGI APP
> 4.WSGI APP接收到请求之后，并将请求disptach到controller中的方法上

如果没有仔细阅读源码或者没了解Restful中强调的资源，可能就对上面提到的杂乱概念毫无头绪。下面就列出一些关键概念，帮助理解。前面多次强调，Restful中，一切都是围绕资源进行操作。OpenStack就定义了两种类型的资源。

> *  core resource: 核心资源，我们可以在`./nova/api/openstack/compute/`目录下面看到servers.py, ips.py,images.py等等文件，它们就是核心资源,可以看出这些资源是云平台的最基础的东西。
> *  extension resource: 扩展资源，在`./nova/api/openstack/compute/contrib/`目录下面有很多文件，这些都是扩展资源，非核心的，例如keypairs.py等等，扩展资源又分为两种情况：1. 扩展资源本身也是一种资源，只是没那么核心，2. 扩展资源是对核心资源的扩展丰富，例如keypairs就扩展了servers核心资源；当然扩展资源也可以同时具备这两种情况。

了解所谓的资源之后，我们还需要了解controller，所谓的controller实际上就是代表对该资源的操作集合，controller中定义了很多的操作，有基本的CRUD操作，还有其他杂七杂八满足我们功能需要的操作，这些操作大致上也可以分为三类：

> *  CRUD: 最基本的操作，对资源的index，create，delete,show,update。
> *  action: 基本操作不可能满足所有对资源的操作，若要增加，则使用@wsgi.action装饰
> *  extends: 使用来@wsgi.extends装饰了的函数，如果扩展资源扩展某个核心资源，扩展资源增加的函数就会添加这个装饰

下一个需要了解的概念便是WSGI APP，在OpenStack中，定义了很多的资源，每个资源有一些操作函数，最终，这每一个资源组成了一个单一的WSGI APP，这就说明了，一个WSGI Server可以对应多个WSGI APP，这样保证了资源之间的独立性。

最后一个是，ExtensionManager,存在这么多的资源，需要使用一个较好的方式对他们进行管理，能够方便的为后期添加更多的资源，丰富OpenStack的API。这样就出现了ExensionManager，对这些扩展资源进行统一的管理。

有了这些基本的概念之后，再回过头来看前面的流程，应该是更加清晰了。下一篇，就使用代码来分析这些文章提到的内容。


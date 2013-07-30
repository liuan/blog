---
layout: post
category: OpenStack
tags: OpenStack
---

### 前言

前面一篇文章简单混乱的介绍了[OpenStack Restful API](http://www.choudan.net/2013/07/28/OpenStack-Restful-API.html),可以了解到OpenStack 中API的设计方式，还有一篇文章详细全面的介绍了[Nova API的启动](http://www.choudan.net/2013/07/28/OpenStack-paste-deploy介绍.html),这篇文章只描述了服务是如何一步步运行起来的，侧重在WSGI Application使用paste deploy模块的方式，正在起来之后再怎样处各种http 请求就没有介绍了。

这一篇文章就重点介绍，Nova-api服务是如何处理外界的API调用请求的，如果需要添加API，到底要怎样做。


### Nova API

在paste deploy一文中已经知道了，Nova API的启动最终会运行到APIRouter class的构造方法中来，由该方法发散出去，就完成了整个wsgi service的启动，从整个service的角度来看，该方法就做了下面几件事。

1.  创建一个ExtensionManager，用来加载所有的extension
2.  调用routes的Mapper.resource方法完成对核心资源的定义，将资源与路由关联起来
3.  同样调用上面的方法完成对扩展资源的定义，也将他们与路由关联起来
4.  

Nova API service 接收到HTTP请求之后，处理过程主要分为四个阶段:

1. 首先通过WSGI Server将HTTP request封装成wsgi request 
2. 使用api-paste.ini文件中定义的Filter对wsgi request进行处理
3. 处理完毕后，就根据mapper中的记录，将不同的请求路由到不同的WSGI APP。
4. WSGI APP接收到请求之后，并将请求disptach到controller中的方法上

以上并是Nova API加载各种API的定义和处理HTTP请求的过程，描述的很抽象，里面存在几个关键的概念，ExtensionManager,extension,WSGI APP, controller,core resource。前面的文章提到过，其中一个很关键的概念是资源，资源具有一个controller，该controller包含一系列的基本方法，例如index，create，show，delete等等基本的CRUD操作。显然，这些操作还不够，需要对它进行扩展来满足丰富的API接口设计。就了后面提到的ExtensionResource，ExtensionController等等概念。下面一一分析。

### Extension Load

APIRouter Class的第一件事，并是创建ExtensionManager,然后用它来加载各种extension。

<img src="/assets/img/openstack_novaapi_extensions01.png" width="700px"/>

上面描述了ExtensionManager的类继承关系和部分功能，其中关键的函数_load_extensions会调用load_standard_extensions方法，该方法并遍历`contrib目录`，该目录便是存放所有extension的地方，下面举了以Keyparis为例，load_standard_extensions函数将会做哪些工作。

<img src="/assets/img/openstack_novaapi_extensions02.png" width="700px"/>

最终load_standard_extensions将contrib目录下的所有extension注册到ExtensionManager中，并且是使用的extension的alias(http request中使用到，必须保证它的唯一性，则样就可以根据http请求，知道是查找哪个extension的controller)。上面这张图也告诉我们如果要实现自己定义的extension就必须定义四个变量name,alias，namespace,updated，并且按情况是决定是否需要冲在get_resource和get_controller_extensions函数，如果需要定义新的Restful资源，这我们需要实现get_resource函数，如果要扩展一个存在的Restful资源的controller,我们需要实现get_controller_exntension函数。例如，Keypairs重新定义了新的keypairs资源，也扩展了servers的controller，所以重新实现了这两个函数。

第二三步均是mapper.resource，第一次是core resource的定义，第二次是extension的定义。

<img src="/assets/img/openstack_novaapi_extensions03.png" width="700px"/>

此处使用的就像前面文中提到的Rails routes中的用法。

<img src="/assets/img/openstack_novaapi_extensions04.png" width="700px"/>

### Handle HTTP Request

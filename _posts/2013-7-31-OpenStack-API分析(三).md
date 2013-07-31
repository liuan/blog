---
layout: post
category: OpenStack
tags: OpenStack
---

### 自定义API

根据前面的介绍，我们就可以按照它的规则自定义自己的API了。创建自己的API有三种方式：

> 1.在原有的资源上增加函数，例如在servers上增加一个接口，查看虚拟机的资源利用情况
> 2.增加扩展资源，定义新的资源
> 3.增加核心资源

### method 1

对于第一种情况，应该是最简单的，我们想在现有的servers上添加一个接口，意味着在servers的controller中添加一个函数，如何能够正确的添加了，我们需要像下图一样，给该函数添加几个装饰器，其中@wsgi.action装饰器需要起一个短名字，然后再http请求的body中添加一条记录`{action:name}`，这样就OK了。

<img src="/assets/img/openstack_novaapi_extensions09.png" width="700px"/>

### method 2

添加新的扩展资源，我们需要写一个py文件，定义一个class，将其放在`nova.api.openstack.compute.contrib`目录下面，文件名小写，然后再文件中定义一个class，类名和文件一样，只是首字母大写，该class要继承于ExtensionDescriptor,并且定义四个变量，name,alias,updated,namespace,并且要保证alias不和其他的冲突，唯一性。

> 1.如果新定义的扩展资源只是扩展以存在的核心资源，需要重写`get_controller_extensions`方法，该方法需要有一个新的Controller class，该class 继承于所扩展的核心资源的controller，并且该class的成员函数需要添加@wsgi.extends装饰器
> 2.如果新定义的扩展资源也定义了新的资源，那么需要再定义个属于自己的Controller类，包含，index，create，delete等等函数
> 3.以上可以二者兼备

 参考下面这张图：

<img src="/assets/img/openstack_novaapi_extensions10.png" width="700px"/>

<img src="/assets/img/openstack_novaapi_extensions11.png" width="700px"/>

<img src="/assets/img/openstack_novaapi_extensions12.png" width="700px"/>

### method 3

定义一个新的核心资源，我们需要在`nova.api.openstack.compute`目录下面创建一个新的py文件，按照servers这样的文件来写一个自己的核心资源，大家可以参考servers.py的方法，还有上面两种方法，最复杂的是在nova.api.openstack.compute.__init__.py文件中的APIRouter class中使用mapper.resource方法。

<img src="/assets/img/openstack_novaapi_extensions13.png" width="700px"/>

首先创建该资源，然后调用mapper.resource方法，该方法有很多可选的变量用来对url进行约束。除了前三个参数是必须的外，后面的都是可选的，至于这些参数到底什么作用，[参考这篇文章中的链接](http://www.choudan.net/2013/07/28/OpenStack-Restful-API.html),概念和Rails routes 中的一样。

### 总结

OpenStack API的框架设计的很灵活，可以很方便灵活的添加api，扩展功能，参考它的实现方式，可以学习到该如何优雅的实现一套API接口。关于HTTP request，抽象的带过了，其中还有很多知识值得学习，例如，在filter层，可以进行身份权限验证，API请求流量控制,Debug开关等等，使用paste deploy来开发WSGI Server确实很灵活很方便，只需要简单的更改paste.ini文件就可以，如果想做一个日志统计，也可以想filter一样，添加一层。

如果理解了Restful所谓的对资源定义，就能够明白你哪些URL，其中的Resource，Extension等等，并且这些概念这OpenStack的其他service中也是一样，在python-****client中同样也存在这样的概念，方便来补充功能。

理解了HTTP 请求的处理，再加上前面分析的RPC机制，这样就理清除了OpenStack中间的两大通信方式，把底层的基础扫清除了，为后面更好的关注于OpenStack的功能和云平台的设计奠定基础。

这一块都是自己在阅读源码的基础上思考得来，可能不存在错误，不够严谨，欢迎大家批评指导！任何问题都及时回复大家！

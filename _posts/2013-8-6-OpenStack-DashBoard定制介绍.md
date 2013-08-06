---
layout: post
category: OpenStack
tags: OpenStack
---

### Horizon

OpenStack Dashborad可能无法满足我们的需求，我们可能需要对Dashboard进行简单的定制，例如在dashboard上添加监控，告警，丰富dashboard支持的功能等等。这篇短文记录当初简单修改Dashboard的过程，添加一个Metro风格的展示页面，统计整个平台的容量信息，例如主机数，主机下所有的VM，可用cpu，memory,查看某台Host上的虚拟机等等。

Horizon作为Openstack的Dashboard模块，使用Django框架来开发的，对于Django框架的学习，可以[猛戳这个链接](http://djangobook.py3k.cn/2.0/)。简单易上手。

我们知道，站点的设计，一般都是采用三层设计模式，Django框架易如此，将前端，业务，数据分离开来。

本章就按照这三层的思路并结合Horizon的源码，从最初的界面展示模块开始入手，分析是如何使用python语言实现了界面的绘制到如何将数据展现到界面上来，到最后的如何从界面获取到用户输入的数据进行OpenStack API的调用。

### 分析

我们查看`horizon/dashboards/syspanel/images`目录下的文件，就可以看出端倪，主要由四部分组成，首先是处理url的模块，余下的便是网页模板templates文件夹，与页面布局相关的views.py,其他的forms，panels，tables则是具体的页面展现形式,具体的业务逻辑则包含在`horizon/api`目录下面。

<img src="/assets/img/openstack_dashboard01.png" width="700px">

下面就以自己写的一个十分简单的metro风格的模块来介绍，如何定制其他的views。

###### url

首先需要处理的是urls.py，确定了点击链接之后跳转到哪儿去的问题，参考下面的图片

<img src="/assets/img/openstack_dashboard02.png" width="700px">

<img src="/assets/img/openstack_dashboard03.png" width="700px">

###### views

再次是views.py，该模块主要是完成页面的简单逻辑，关联一个具体的TestTile对象，用来处理数据。可以重载TileView中的get_context_data，丰富context内容，能够在页面上访问获取的context中的数据。get_data方法，可以调用api，获取想要显示的数据，将该数据在传至到TestTile对象。

<img src="/assets/img/openstack_dashboard04.png" width="700px">

###### TestTile
下面是TestTile类，简单的继承于tiles.Tile

<img src="/assets/img/openstack_dashboard05.png" width="700px">

######  TileView

在dashborad中，我们看见很多这样的forms，panels,tables等等，现在需要设计自己的TileView，这需要写个类似于forms这样的view类。下面便是TileView的实现，可以看出，继承该类都需要指定一个具体的tile_class类，通过get_tile函数完成tile_class类的实例化。Get_context_data函数获取了该tile_class对象，保存在context中，这样可以在View的html template中完成对context的特定数据访问，例如访问`\{\{tile.render\}\}`。
另外一个是获取view得到的数据，将该数据传递给Tile实例。
通过_get_data_dict函数，将获取的数据传递进来，然后简单的保存到tile._data中，最后Django框架会调用关键的函数get，来完成页面的响应。

<img src="/assets/img/openstack_dashboard06.png" width="700px">

###### Tile

这个是Tile类，最关键的是render方法和self.data属性，self.data保存了页面要显示的数据，render实现了将数据传递到页面的过程。

<img src="/assets/img/openstack_dashboard07.png" width="700px">

最终的效果如下图：

<img src="/assets/img/openstack_dashboard.png" width="700px">

###### 

除了界面的生成之外，另外一个重要的问题是，如何处理页面url的跳转，参数的传递。在table页面中，一个url参数的传递由三部分在组成，下面一一介绍。
首先是url，观察urls.py文件，内容如下图，可以看到url的格式，名字是detail，这个链接将由HostIndexView来处理，链接有一个匹配的正则表达式。注意其中传递了一个host参数。获取该host值得方式是，self.kwargs[‘host’]

<img src="/assets/img/openstack_dashboard08.png" width="700px">

其次是在table上给某一个列增加link，下面第三行就标识了具体的链接，有路径加处理的名字组成，对应到urls.py中得name。

<img src="/assets/img/openstack_dashboard09.png" width="700px">

观察第401行，给每个实例添加了一个id属性，因为在table的column处理link时，默认将id最后其要传递的值。*修正：这个id不一定是一个数字，也可以是host的名字，如gb07,必须唯一*。

<img src="/assets/img/openstack_dashboard10.png" width="700px">

这样三步就完成了table表格中链接的处理。

###### 界面刷新

下面详细介绍table中的刷新机制。
在table中，horizon目前实现了table的自动更新机制，主要涉及到的技术是JQuery和Ajax，JQuery作为java script的库完成了页面的动态显示工作，Ajax则主要负责网页动态刷新机制。在JQuery库中，已经很好的封装了Ajax的动态刷新功能。
Ajax的动态刷新可以实现只刷新整个网页的一小部分，而不需要刷新整个网页。其中最核心的是使用XMLHttpRequest对象和DOM来实现网页的刷新。这部分入门很快。[关于JQuery刷新的内容参考如下网页](http://www.w3school.com.cn/jquery/ajax_ajax.asp)。
Horizon中table的刷新，这功能的代码主要在horizon.communication.js和horizon.tables.js两个文件中。其中第一个完成通信方面的工作，后面完成html特定元素的刷新工作。
首先查看horizon.communication.js文件。
该文件主要有两个队列，一个queue函数和一个next函数，

<img src="/assets/img/openstack_dashboard11.png" width="700px">

关键的函数在32行，$.ajax(opts)，该函数是JQuery内置的函数，接收一个对象类型的参数，该函数完成了与服务器的异步通信。
查看opts具体的内容，则回到horizon.tables.js文件。该文件就是不断地间隔执行update函数，update函数则检测页面上是否存在tr.status_unknown.ajax-update元素，不存在则退出，若存在则对每个这样的元素调用queue函数，传递的对象具有多个属性，其一是url，后面依次是error，success，complete函数，分别对应错误，成功后的函数，最后complete为无论失败成功都要执行的。

<img src="/assets/img/openstack_dashboard11.png" width="700px">

其中的setTimeout(horizon.datatables.update,next_poll)，则是间隔next_poll时间去调用update函数。

<img src="/assets/img/openstack_dashboard12.png" width="700px">

至此，就大致明白了如果实现table的自动刷新，这也就是在dashboard上创建虚拟机时，Task状态一直在变化，直到虚拟机运行起来。刚创建虚拟机的时候，页面会存在status_unknown元素，当虚拟机变为running后，该元素就remove了。故，之后页面不会再刷新了。

### 总结

很早之前记录的这些内容，现在看来，内容有些混乱，没有做到条理清晰的描述到底该如何一步步添加新的view。并且tile功能简单，需要再完善。不过，整体感觉，Django的框架简化了开发，能专注于功能开发，摆脱细致繁琐的页面编写。

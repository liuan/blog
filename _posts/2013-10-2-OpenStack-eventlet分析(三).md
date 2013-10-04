---
layout: post
category: OpenStack
tags: OpenStack
---

很早之前潦草的记录了eventlet库的简单分析，主要偏重于eventlet处理多协程并发调度方面的问题，而没有讲述网络方面的内容，趁国庆有时间，争取将eventlet网络部分补齐。

之前的两篇eventlet的博客链接在此。

> 1. [OpenStack Eventlet分析(一)](http://www.choudan.net/2013/08/18/OpenStack-eventlet分析%28一%29.html)
> 2. [OpenStack Eventlet分析(二)](http://www.choudan.net/2013/08/19/OpenStack-eventlet分析%28二%29.html)

关于linux网络编程方面的基础知识，链接在此。

> 1. [Unix网络IO模型](http://www.choudan.net/2013/08/17/Unix网络IO模型.html)
> 2. [Unix网络模型之select模型](http://www.choudan.net/2013/08/18/Unix网络模型之Select.html)

除了我个人写的几篇简陋的博客之外，这儿还有一重量级的关于eventlet网络分析的博客，属于强烈推荐型的，请猛戳。

> 1. [python eventlet并发原理分析](https://github.com/stanzgy/wiki/blob/master/openstack/inside-eventlet-concurrency.md)

前面我们强调过，eventlet在网络方面主要依赖于python的select库，该库在windows下只支持socket，在linux支持所有的文件描述符。可以先参考以下几篇文章了解python的select库。

> 1. [Python网络编程中的select和poll I/O复用的简单使用](http://www.cnblogs.com/coser/archive/2012/01/06/2315216.html)

### 正文

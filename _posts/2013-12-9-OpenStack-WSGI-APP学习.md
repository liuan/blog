---
layout: post
category: OpenStack
tags: OpenStack
---

前面已经写了几篇关于OpenStack API分析的博客了，涉及到了paste.deploy对WSGI APP的配置和部署，routes完成URL到resources的映射关系，OpenStack API的设计方式以及如何扩展其中的API等问题。这些内容已经差不多构成一个WSGI Application的全部了，这一篇则尝试将所有的内容都贯通一起来，通过学习OpenStack的源码来自己动手，快速实现一个WSGI Application的原型。在了解如何实现之前，可以先参考这些基本知识。

* [OpenStack Paste Deploy介绍](http://www.choudan.net/2013/07/28/OpenStack-paste-deploy介绍.html)
* [OpenStack RESTful API](http://www.choudan.net/2013/07/28/OpenStack-Restful-API.html)
* [OpenStack API分析(一)](http://www.choudan.net/2013/07/30/OpenStack-API分析%28一%29.html)
* [OpenStack API分析(二)](http://www.choudan.net/2013/07/31/OpenStack-API分析%28二%29.html)
* [OpenStack API分析(三)](http://www.choudan.net/2013/07/30/OpenStack-API分析%28三%29.html)

### 入门

OpenStack大部分的project都会附有一个api的server，例如nova-api，glance-api，还有不单独存在一个api服务得project，例如neutron，keystone，这些server内部也有对api的处理，只是没有分离出来。简单的来说，所有这些api都是一个WSGI的application。如何实现一个WSGI APP了，网上有很多博客，大多浅尝则止，还不足以构成想OpenStack中这样确实可用的API application，但是对于入门来说，都是恰好不过了，[可以参考这个链接](http://www.blog.csdn.net/sraing/article/details/8455242)。

从上面链接给出的博客中，我们知道实现一个wsgi的application的方法就是实现一个callable对象，这个对象接收两个函数，通常为environ,start_response。具体就是下面的代码模样：

    class ShowVersion():
        def __init__(self):
            pass
        
        def __call__(self,environ,start_response):
            start_response("200 OK",[("Content-type","text/plain")])
            return ["Paste Delpoy : Version = 1.0.0"]

        @classmethod
        def factory(cls,global_conf,**kwargs):
            return ShowVersion() 
     
上面的代码重载了__call__函数，所以ShowVersion对象就是一个可调用对象了，满足了WSGI规范的要求，只是这个应用极其简单，只返回一个字符串信息。def factory函数是为了paste.deploy服务的。现在，我们尝试用OpenStack中的实现方式再实现一遍这个show version的内容。

### 进阶

有了上面这个简单的例子，我们知道复杂的wsgi application肯定也是由它这个基本模型扩展起来的。openstack中的wsgi application会涉及到几个比较重要的python 库。

1. [eventlet.wsgi](http://eventlet.net/doc/modules/wsgi.html)
2. [paste.deploy](http://pythonpaste.org/deploy/)
3. [routes](http://routes.readthedocs.org/)
4. [webob](http://docs.webob.org/)
5. [wsgiref.simple_server](http://docs.python.org/2.5/lib/module-wsgiref.simpleserver.html)

前三个都可以通过我之前的文章了解到，最后一个webob则是用来对http请求和响应进行封装处理的库，现在我们不需要深入探讨其用法，知道简单的使用就行。eventlet.wsgi和wsgiref.simple_server都是提供一个WSGI server的功能，前者更为复杂，性能也更好，后者简单，用来学习不错。

现在，我们明确下，开发一个OpenStack 风格的WSGI APP原型需要完成的几方面的工作：

1. 从配置文件中找到WSGI APP程序启动的入口，例如nova的api-paste.ini文件。
2. 定义好APP需要操作的资源，这儿我们主要是app的版本资源。
3. 完成好url到资源的映射。

明确了上面开发的三点工作之后，就可以动手写了。

### 实现

首先，我们实现一个server.py文件，该文件主要实现一个标准的paste.deploy加载应用的过程。

    import os
    import logging
    import sys
    from paste import deploy
    from wsgiref.simple_server import make_server

    LOG = logging.getLogger(__name__)

    module_dir = os.path.normpath(os.path.join(os.path.abspath(sys.argv[0]),
                            os.pardir,os.pardir))

    sys.path.insert(0,module_dir) # 将当前的目录添加系统路径中去

    bind_host = "127.0.0.1"
    bind_port = 8080

    def server(app_name, conf_file):
        app = load_paste_app(app_name,conf_file) // 加载应用
        serve = make_server(bind_host,bind_port,app) // 创建server
        serve.serve_forever() //启动server

    def load_paste_app(app_name, conf_file):
        LOG.debug("Loading %(app_name) from %(conf_file)",
                    {'app_name':app_name, 'conf_file':conf_file})
        
        try:
            app = deploy.loadapp("config:%s" % os.path.abspath(conf_file), name=app_name)
            return app
        except (LookupError, ImportError) as e:
            LOG.error(str(e))
            raise RuntimeError(str(e))

        
    if __name__ == '__main__':
        app_name = "choudanview"
        conf_file = "choudanview.ini"
        server(app_name,conf_file)

第二步，我们定义一个简单的配置文件choudanview.ini，内容很简单，只有两行，告诉程序，WSGI APP的入口何在。

    [app:choudanview]
    paste.app_factory = choudanview.router:API.factory

第三步，我们开始尝试去实现一个WSGI APP了。这就开始按照前面提到的简单WSGI APP的例子了，我们需要重载__call__函数，这此基础上，这个app又能完成成url到resources的映射。我们先实现一个wsgi.py文件，其中定义一个router类，作为基类，重载了__call__函数，但是参数只有一个，因为通过了webob.dec.wsgify的装饰。

    import logging
    import routes.middleware
    import webob.dec
    import webob.exc


    class Router(object): //作为WSGI APP的基类，能够完成url到resource的映射

        def __init__(self, mapper=None):
            self.map =  mapper //建立了resource的map
            self._router = routes.middleware.RoutesMiddleware(self._dispatch,
                                                             self.map) //注册关于url的回调函数
        @classmethod
        def factory(cls, global_conf, **local_conf): # 实际的入口
            return cls() # 构造该app

        @webob.dec.wsgify # 能够将request和response封装成WSGI 风格的
        def __call__(self,req): # callable对象
            return self._router

        @staticmethod
        @webob.dec.wsgify
        def _dispatch(req):
            # TODO
            match = req.environ['wsgiorg.routing_args'][1]
            if not match:
                return webob.exc.HTTPNotFound()
            app = match['controller']
     
第四步，我们看看如何构造url到resource的映射。router.py文件如下：

    import routes

    from choudanview import wsgi
    from choudanview import versions

    class API(wsgi.Router):

        def __init__(self, mapper=None):
            if(mapper == None): #创建mapper对象
                mapper = routes.Mapper()
            
            versions_resource = versions.create_resource() # 创建资源
            mapper.connect("/",controller=versions_resource, # 建立对应关系
                            action="index")
            super(API,self).__init__(mapper) 

第五步，创建versions的资源类，versions.py文件。

    import httplib
    import json
    import webob.dec

    from webob import Response

    class Controller(object):
        def __init__(self):
            # TODO
            self.version = "0.1"

        def index(self,req):
            response = Response(request=req,
                                      status=httplib.MULTIPLE_CHOICES,
                                      content_type='application/json')
            response.body = json.dumps(dict(versions=self.version))
            return response
                
        @webob.dec.wsgify
        def __call__(self, request):
            # TODO
            return self.index(request)

    def create_resource():
        return Controller()

至此，整个wsgi app的原型则创建完毕，大部分openstack的api app都是按照这个思路来实现的，只是他们需要包含的功能更多。我们可以通过简单的执行`python server.py`来启动server，然后通过浏览器`localhost:8080`来访问。最终会得到"versions=0.1"的信息。

### 总结

上面的几个文件就已经将openstack中的wsgi app的原型提炼出来了，不同的project的api app都是根据业务需求来进行扩展或增加更多的设计，例如glance-api就大致是这样的内容，而nova-api显然需要管理的资源更多，资源的操作也更加复杂，所有在此基础上添加了extension manager，还可以更灵活的管理插件。总之，通过python的wsgi，我们可以快速的实现一个可用的server及application，对于性能方面的要求，我们可以使用eventlet库中封装的wsgi server。还可以采用glance-api中实现的多进程模型。

OpenStack的源码，提供了一个窥探python的绝佳机会，从中可以学到很多东西的实现，有了这些基础，可以再做更加深入的工作。

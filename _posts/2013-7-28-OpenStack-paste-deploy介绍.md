---
layout: post
category: OpenStack
tags: OpenStack
---

### Paste.deploy

python中的WSGI（Web Server Gateway Interface）是Python应用程序或框架与Web服务器之间的一种接口，定义了一套借口来实现服务器与应用端的通信规范。按照一套规范，应用端想要通信，很简单，只需要实现一个接受两个参数的，含有__call__方法并返回一个可遍历的含有零个或者多个string结果的python对象。

服务端，对于每个http请求，调用一次应用端“注册”的那个协议规定应用必须实现的对象，然后返回相应的响应消息。
WSGI Server唯一的任务就是接收来自client的请求，然后将请求传给application，最后将application的response传递给client。中间存在的一些东西，就需要中间件来处理。

Paste Deployment是用于发现和配置WSGI appliaction和server的系统。对于WSGI application，用户提供一个单独的函数（loadapp），用于从配置文件或者python egg中加载WSGI application。因为WSGI application提供了唯一的单独的简单的访问入口，所以application不需要暴露application的内部的实现细节。

Paste.deploy加载WSGI的应用十分简单，主要是使用paste.deploy.loadapp函数，基本用法如下： 

<img src="/assets/img/openstack_pastedeploy01.png" title="openstack_pastedeploy_01" width="700px">

其中URI存在两种模式：config: 和 egg: 。
下面详细的介绍下配置文件模式。
首先了解几个基本的概念：

* application: 应用，符合WSGI规范的可调用对象，接受参数（environ,start_response）, 调用start_response返回状态和消息头，返回结果作为消息体。

* filter：过滤器，可调用对象，类型python中的装饰器，接受一个application对象作为参数，返回一个封装后的application。

* app_factory:可调用对象，接受参数（global,\*\*local_conf）,返回application对象

* composite_factory: 可调用对象，接受参数（loader,global_config,\*\*local_conf）, loader有几个方法, get_app用于获取wsgi_app, get_filter用于加载filter, 返回application对象。

* filter_factory: 可调用对象，接受参数(global_config, \*\*local_conf)，返回filter对象

以上这些概念若结果代码来看，就更容易理解了。
配置文件的格式基本如下：

<img src="/assets/img/openstack_pastedeploy02.png" title="openstack_pastedeploy_02" width="700px">

由section type和name构成一段section，然后是key = value。
在一个配置文件中一般存在多个section。

* default section,如下图，该section中的key与value是定义的全局的变量，在其他的section中可以引用这些变量，也可以使用set 语句来重载这些变量

<img src="/assets/img/openstack_pastedeploy03.png" title="openstack_pastedeploy_03" width="700px">

* application section,如下图，一个文件可以定义多个application section，每个application都有自己独立的section，该section需要指定运行的方式，如上面的，存在四五种方式，第一种指定了paste.app_factory，精确到具体运行的python代码。第二种调用其他的配置文件及执行的应用程序名字，第三种，使用python包。第四种，直接指定需要调用的类。第五种，引用其他section。该section其他的key与value作为参数传递给factory。 

<img src="/assets/img/openstack_pastedeploy04.png" title="openstack_pastedeploy_04" width="700px">

* compostion section,如下图，Composite是由多个application组成。每个key与value对应一个application 

<img src="/assets/img/openstack_pastedeploy05.png" title="openstack_pastedeploy_05" width="700px">

* filter section 将filter应用到一个application。

<img src="/assets/img/openstack_pastedeploy06.png" title="openstack_pastedeploy_06" width="700px">

* pipeline section, Pipeline 由一些列的filter组成，最后一个是应用，即将前面的fiiter应用到application。 

<img src="/assets/img/openstack_pastedeploy07.png" title="openstack_pastedeploy_07" width="700px">


### api-paste.ini

基于上面的基础知识，我们来具体的分析nova中的api-paste.ini 文件，结合nova的代码实现。 

<img src="/assets/img/openstack_pastedeploy08.png" title="openstack_pastedeploy_08" width="700px">

上图就是nova-api-os-compute daemon涉及到的大部分配置。从服务启动的过程中，就调用deploy.loadpp使用config方式来load section名为osapi_compute的应用，找到具体的配置文件，就对应到代码行66，是一个composite类型的section，直接调用nova.api.openstack.urlmap中的urlmap_factory函数，其他的三行作为local_conf参数传入。

<img src="/assets/img/openstack_pastedeploy09.png" title="openstack_pastedeploy_09" width="700px">

上面的代码就是待用的urlmap_factory,其中161行开始，就将上面三行指定的应用加载进行，构成一个dict类型的urlmap，即不同的http请求路径对应不同的app。然后开始分别加载其中三个application，我们关注openstack_compute_api_v2应用，由上面代码可知，它在配置文件中是一个composite section，会调用一个pipeline_factory函数，查看该函数。 

<img src="/assets/img/openstack_pastedeploy10.png" title="openstack_pastedeploy_10" width="700px">

在nova.conf文件中宏，配置了auth_strategy为keystone，那么就找到了75行中的keystone pipeline，然后提取出这个pipeline中的前n-1个filter，提取最后一个app，然后将filter逆序，再分别应用到这个app上。注意其中的顺序。我们来看keystone对应的最后一个app，osapi_compute_app_v2，找到具体的section，直接执行APIRouter.factory。到这儿，这个application就启动来了。在访问这个app时，还有一些列的filter需要应用来，譬如权限检查等等。

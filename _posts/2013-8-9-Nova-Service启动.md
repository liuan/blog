---
layout: post
category: OpenStack
tags: OpenStack
---

Nova project下面具有多个service，api,compute,sceduler等等，他们的启动过程都几乎类似，这一篇博客就详细记录nova-sceduler的启动过程。文章中贴出的源码都是从OpenStack Folsom版截取过来的。

下面就开始分析nova-sceduler的启动过程了，后面还有涉及到启动之后，做的一些周期性工作，这部分可能与sceduler无关，是在compute中的，一次帖上来。

<img src="/assets/img/openstack_nova_launch01.png" width="700px">

首先是解析启动脚本的参数，包括配置文件，设置日志，utils.monkey_patch现在不明白（为了能够使用高效的eventlet模块，需要打些补丁），然后创建服务，最后启动服务，等待请求。
Service.Service.create(binary=’nova-scheduler)过程如下：

<img src="/assets/img/openstack_nova_launch02.png" width="700px">

过程为获取host name，topic即为用来与rabbit通信的标识，为scheduler，manager为scheduler_manager，在flags中搜索其对应的类名，其值在nova.conf中指定，默认值为nova.scheduler.manager.SchedulerManager ，report_interval为节点将状态报告给数据库的时间间隔，默认为10秒。Periodic_interval，执行周期性任务的周期。再开始初始化service。

<img src="/assets/img/openstack_nova_launch03.png" width="700px">

其中关键的manager，其中self.manager_class_name 为nova.scheduler.manager.SchedulerManager，通过importutils.import_class动态的导入该对象，故manager_class是`<class ‘nova.scheduler.manager.SchedulerManager’>`这么一个对象。（记住，在python中，一切都是对象，有类型对象，实例对象，譬如int为一个类型对象，5则为一个实例对象，其类型是int，而int的类型则为type。）然后，调用SchedulerManager的__init__函数完成初始化，注意SchedulerManager的__init__函数的参数列表，有args和kwargs，所以host=self.host就成了kwargs的一项。

创建完service类之后，开始启动service。

<img src="/assets/img/openstack_nova_launch04.png" width="700px">

其中workers为none，执行else语句，lanuch_server如下：

<img src="/assets/img/openstack_nova_launch05.png" width="700px">

使用eventlet.spawn启动一个green thread，run_server如下：

<img src="/assets/img/openstack_nova_launch06.png" width="700px">

开始启动server，其中start的关键代码如下：

<img src="/assets/img/openstack_nova_launch07.png" width="700px">

首先获取一个到rabbitmq server的连接，然后再注册一个rpc_dispatcher，该对象与回调函数相关，接收到rabbitmq的消息后，再由它来处理，接着创建多个consumer来接收特定topic的消息队列的消息，并设置好消息监听。
这样一个服务就启动来了，在后面还有设置周期性的task。

在初始化service的过程中，会调用importutils动态导入具体的manager，对于nova-compute，导入的则是ComputeManager，该类的继承关系是，ComputeManager，Manager.SchedulerDependentManager，Manager，nova.db.Base.其中Manager包含ManagerMeta元类。可以参考[RabbitMQ(三)](http://www.choudan.net/2013/07/25/OpenStack-RabbitMQ%28三%29.html)中的manager类关系图。
对Manager与ManagerMeta的分析如下，这块代码涉及到一个资源刷新的问题。对于metaclass的分析可以参考后面的python高级中的metaclass最后一个例子分析。Manager具有一个类属性_periodic_tasks,是一个列表类型的属性，元素是各个需要周期执行的task。在manager类创建时，因为使用到了metaclass，会首先检查每个具有_periodic_task属性的函数，该属性由装饰器periodic_task装饰上的。

<img src="/assets/img/openstack_nova_launch08.png" width="700px">

这部分的代码就是启动周期性的task。

LoopingCall初始化最关键的参数是f，即传递进一个函数。430行，就将service的report_state函数传递进去，然后调用start函数，下面是start函数的实现。

<img src="/assets/img/openstack_nova_launch09.png" width="700px">

可以知道，start函数内部有一个闭包函数，然后启动一个greenthread的来执行这个内部函数，内部函数根据传递的参数，决定是否要推迟启动，然后开始周期性的执行传递进来的函数，即self.report_state和self.periodic_task.

查看report_state的代码，关键的如下：

<img src="/assets/img/openstack_nova_launch10.png" width="700px">

主要的作用是周期性的更新数据库中的nova库service表的report_count字段，目前不知道该字段有什么作用！

再查看periodic_task函数，service.periodic_tasks函数最后会调用self.manager.periodic_tasks，该函数在nova.manager.py中，该函数会去调用被periodic_task装饰的函数，在nova-compute服务中，被periodic_task装饰的有_publish_service_capabilities函数，_report_driver_status函数等等，report_driver_status读取host的相关信息，然后更新capabilities，最后通过scheduler_rpcapi发送到scheduler服务去。

再开始看其中重要的一块RPC，Rpc参考前面的[RabbtMQ三](http://www.choudan.net/2013/07/25/OpenStack-RabbitMQ%28三%29.html)，关于rpc有详细的介绍。
最后一块pluginManager。



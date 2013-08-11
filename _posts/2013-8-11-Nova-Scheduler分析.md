---
layout: post
category: OpenStack
tags: OpenStack
---

Nova-Scheduler主要完成虚拟机实例的调度分配任务，创建虚拟机时，虚拟机该调度到哪台物理机上，迁移时若没有指定主机，也需要经过scheduler。资源调度是云平台中的一个很关键问题，如何做到资源的有效分配，如何满足不同情况的分配方式，这些都需要nova-scheduler来掌控，并且能够很方便的扩展更多的调度方法，可能我需要虚拟机调度到空闲的机器，可能还需要将某类型的虚拟机调度到固定的机架等等，现在就来看看Nova-Scheduler是如何完成调度任务的。

*注意:所有的分析代码都是基于OpenStack Folsom*

上面一篇博客已经介绍了[Nova-Scheduler服务的启动过程以及资源更新的方式](http://www.choudan.net/2013/08/09/Nova-Service启动.html),在nova-scheduler服务的启动过程中，已经指定了manager类型，可以从配置文件配置scheduler_manager来确定具体的manager。`默认的manager是SchedulerManager`，该类位于nova.scheduler.manager.py文件中。

<img src="/assets/img/openstack_nova_scheduler01.png" width="700px">

如上，在SchedulerManager文件中，存在init函数，确定了scheduler_dirver，`默认的为scheduler.multi.MultiScheduler对象`，一种调度方式。

下面将分析创建一个虚拟机，nova-scheduler将进行的所有操作，主要就分为以下几个过程：

> 1.过滤掉为满足虚拟机要求的主机
> 2.对满足虚拟机要求的主机进行权重计算
> 3.选取权重计算值最优的主机返回


### 处理run_instance过程

创建一个虚拟机，nova-api则会通过rpc调用scheduler的run_instance函数完成主机的分配！

<img src="/assets/img/openstack_nova_scheduler02.png" width="700px">

上面便是启动一个instance的过程，然后根据driver类型调用scheduler_run_instance方法。

<img src="/assets/img/openstack_nova_scheduler03.png" width="700px">
<img src="/assets/img/openstack_nova_scheduler04.png" width="700px">
<img src="/assets/img/openstack_nova_scheduler05.png" width="700px">
<img src="/assets/img/openstack_nova_scheduler06.png" width="700px">

在scheduler.multi.MultiScheduler中，调用下一个函数。其中`drivers[‘computer’]默认为nova.scheduler.filter_scheduler.FilterScheduler类`。启动虚拟机的任务在由FilterScheduler类中的scheduler_run_instance完成。

#### 计算满足条件主机的权重weighted

<img src="/assets/img/openstack_nova_scheduler07.png" width="700px">

###### 获取权重weighted和成本cost计算的函数

该函数随后调用_schedule函数，访问一系列的hosts。开始进入了host的调度策略了。查看_scheduler函数，首先关键的是获取权重和计算成本的函数，查看cost_functions = self.get_cost_functions()

<img src="/assets/img/openstack_nova_scheduler08.png" width="700px">

具体到get_cost_functions该函数将会为每个topic对应一个权重和计算成本的列表，如果之前缓存过，就直接返回。如下，topic为compute，若没有缓存，则从FLAGS中读取，可知，该函数列表是可以通过配置文件设置的。

<img src="/assets/img/openstack_nova_scheduler09.png" width="700px">
<img src="/assets/img/openstack_nova_scheduler10.png" width="700px">

默认的least_cost_functions是compute_fill_first_cost_fn，该函数就是返回host的剩余内存大小。其中，每个函数对应一个权重因子，也是在flag中定义，名字为函数名加上”_weight”,例如compute_fill_first_cost_fn对应的就是compute_fill_first_cost_fn_weight，

<img src="/assets/img/openstack_nova_scheduler11.png" width="700px">

由此可见，若host的内存越大，所获得优先级越高。还可以配置多个计算函数，在nova.conf文件中，以逗号分开。

<img src="/assets/img/openstack_nova_scheduler12.png" width="700px">

###### 过滤掉不满足要求的主机

然后，获取所有这些未过滤掉的节点信息，具体在host_manager.HostManager.get_all_host_states函数中，首先从数据中选出所有可用的未过滤的节点信息，组成一个字典host_state_map，涉及到的表示nova.compute_nodes表。这些节点信息由节点的内存容量，虚拟cpu个数，硬盘容量等组成。

然后根据需要，过滤这些主机，还是在HostManager中。

<img src="/assets/img/openstack_nova_scheduler13.png" width="700px">

Filter_hosts函数首先选择可以配置的过滤规则对象，默认的如下：

<img src="/assets/img/openstack_nova_scheduler14.png" width="700px">

所有这些过滤规则对象都是继承于scheduler.filters.BaseHostFilter类，只要实现了host_passes方法就可以实现对主机的过滤，返回真就是这个主机符合条件。

以RamFilter为例，只有一个host_passes方法，查看其host_passes函数，该函数就是简单的以内存来作为限制条件过滤。首先获取instance请求的内存，host剩余的内存，host可以使用的内存，根据FLAGS.ram_allocation_ratio * total_usable_ram_mb得到host可以分配的总内存。然后减去host现在实际已经使用的内存，比较其与请求的内存。若可以，则返回true。

<img src="/assets/img/openstack_nova_scheduler15.png" width="700px">

获取到这些过滤规则的对象之后，让每一个host都依次经历每一个过滤规则过滤，最后返回所有通过的host。

###### 计算满足要求的主机的权重

最后开始计算权重，在least_cost.weighted_sum函数中，完成计算。

<img src="/assets/img/openstack_nova_scheduler16.png" width="700px">

具体查看weighted_sum函数，可以知道如下，根据前面得到的计算函数，这些函数根据权重属性去衡量每一个host，然后乘以这个函数的权重值，得到host的一个打分，找出最小分数的host，其优先级最高，然后返回这个host。根据前面的描述，可知，目前的计算函数只是简单的返回host的剩余内存大小。

<img src="/assets/img/openstack_nova_scheduler17.png" width="700px">

选择好host之后，则立马更新该host消耗的资源，以为下一个instance准备。这样就完成了host的选择。最后通过_provision_resource函数完成rpc调用。

总之，整个过程可以描述为，先对节点进行过滤，然后按权重，用户配置的策略进行排序，选出最优的节点。


### 总结

纵观整个选取主机的过程，发现默认的方法很简单，只是先通过虚拟机需要的内存硬盘等信息来过滤主机，然后计算满足条件的主机的权重，选取最优的返回。所以我们实验环境使用的多台一样的配置的主机，然后在dashboard上创建虚拟机，每台虚拟机会分别运行在不同的主机上。

通篇都在讨论是如何进行调度的，我们知道调度的策略是依赖于主机的资源数据的，那么主机的资源到底是如何更新的，在上一篇中我们介绍了，[Nova-Scheduler服务的启动过程以及资源更新的方式](http://www.choudan.net/2013/08/09/Nova-Service启动.html)，nova中的服务会做周期性的工作，就是_periodic_tasks来完成对主机资源的更新！

虽说默认的调度方式很简单，思路直观！但是scheduler给我们展示了一个灵活，扩展性特别强的设计，提供了很多机会让用户根据自己的需求来扩展调度策略！从开始的scheduler_driver,compute_driver,cost_functions等等，我们都可以设计新的策略替代原本的方案！对于缺乏的经验的程序猿来说，通过这个nova-scheduler，可以受到很多启发，如何设计合理的调度策略，怎样让他保持扩展等等！


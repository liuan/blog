---
layout: post
category: OpenStack
tags: OpenStack
---

很久没写OpenStack方面的博客了，更新一篇觉得略微有意思的。Heat参照于AWS CloudFormation，用简单的话说，CloudFormation提供了一个模板语言，描述了怎样利用AWS提供的虚拟资源，进行一系列的有序组合创建目标系统，根据这个描述文件，CloudFormation的执行引擎最后会有序高效的组合出我们期待的系统，完成了成早期的虚拟机模板到系统模板的跨越，在云平台上部署系统也是如此简单，老板再也不用担心了。

Heat学习过来，提供了多种格式，兼容AWS的资源描述名称，老版本的Quantum，新版本的Neutron，这些兼容都是通过简单的名字映射完成转换。[AWS CloudFormation模板的创建规则可以猛搓这儿](http://docs.aws.amazon.com/AWSCloudFormation/latest/UserGuide/GettingStarted.html), [Heat的可以参考这儿](http://docs.openstack.org/developer/heat/template_guide/)。 抛开具体的描述规则，主要看看Heat中是如何调度描述的资源去有序创建的，顺道再学习Python中的yield关键字。

* 注意：以下内容基于OpenStack Heat-2014.1版本

### 拓扑序
-----------

Heat模板描述了一个系统需要用的资源，以及资源之间的依赖关系（体现在创建系统的过程中，便是时间上的先后顺序），可以通过下面简单的例子说明两个资源间的依赖关系。

[Resource Dependencies](http://docs.openstack.org/developer/heat/template_guide/hot_spec.html) 

    resources:
        server1:
          type: OS::Nova::Server
          depends_on: server2
        server2:
          type: OS::Nova::Server

[WordPress_NoKey.yaml](https://github.com/openstack/heat-templates/blob/master/hot/F18/WordPress_NoKey.yaml)

    resources:
      securitygroup:
        properties:
          SecurityGroupIngress:
            - IpProtocol: 'icmp'

    wordpress_instance:
        properties:
          SecurityGroups:
            - get_resouce: securitygroup

从上面的两个例子可以知道，资源之间的依赖关系可以通过两种形式，显示的关键语法`depends_on`和资源属性来体现，虚拟机实例依赖于SecurityGroups，则意味着SecurityGroups会先于instance创建（尽管securitygroup可以创建完虚拟机之后再添加，但是先创建securitygroup，则可以在创建虚拟机时就一遍完成）。解析这样的模板文件，并可以获取到系统所需要的云资源，以及这些资源间的依赖关系。显然，大家会想到这事一种偏序关系，可以构成拓扑图。

在heat的实践中,也是这样创建了一个拓扑图，详见`engine/depencies.py`文件，其中定义了`class Node, class Graph, class Dependencies`用于描述资源间的关系。至此，创建资源的顺序就是一个典型的拓扑排序了。

知道这个拓扑排序之后，再看里面复杂的scheduler实现方式，这个略微有趣。实现的代码可以着重参考，当作一个简单的任务调度执行样式。

###  yield
------------

Heat实现调度的资源操作的方法基本都在`engine/scheduler.py`文件中，在看这份代码之前，还是先了解python中的yield关键字。先上个参考文档：

* [Python yield 使用浅析](http://www.ibm.com/developerworks/cn/opensource/os-cn-python-yield/)

这篇大神博客讲的浅显易懂，其中最为关键的一点，需要记住：

> 简单地讲，yield 的作用就是把一个函数变成一个 generator，带有 yield 的函数不再是一个普通函数，Python 解释器会将其视为一个 generator，调用 fab(5) 不会执行 fab 函数，而是返回一个 iterable 对象!

即按习惯的方式调用函数而不会执行该函数，而是返回一个iterable对象。在scheduler.py文件中，存在大量的yield使用。

### 调度 

以创建一个系统的过程为例，解析完模板文件之后，获得了对应资源的对象，例如`servers`。资源对象基本上位于`resources`目录下，每个资源都定义了一套标准的action，均以`handle_`作为前缀，这些action完成了对需要创建的系统的管理，包括创建，更新，删除，意味着可以对云上的系统进行生命周期管理了，加上监控，可以做更多的事了，Heat很酷。

获得资源对象之后，Heat会处理一系列的参数等事情之后，然后正式调用`engine/parser.py中Stack的create方法`，该方法正式进入创建资源的调度过程。这个调度在Heat中称为Task，将任务封装到TaskRunner中，就可以执行Task。下面是create函数的具体内容。

    def create(self):
        '''
        Create the stack and all of the resources.
        '''
        def rollback():
            if not self.disable_rollback and self.state == (self.CREATE,
                                                            self.FAILED):
                self.delete(action=self.ROLLBACK)
        creator = scheduler.TaskRunner(self.stack_task,
                                       action=self.CREATE,
                                       reverse=False,
                                       post_func=rollback)
        creator(timeout=self.timeout_secs())

TaskRunner的构造参数中定义了，这个Task将执行的函数，对stack所涉及的资源执行的action，即创建。最后调度这个Task运行，运行的过程是执行self.stack_task函数，这个函数的内容略微复杂了。省略了其他不重要的代码。

    @scheduler.wrappertask
    def stack_task(self, action, reverse=False, post_func=None):
        ...
        action_task = scheduler.DependencyTaskGroup(self.dependencies,
                                                    resource_action,
                                                    reverse)
        try:
            yield action_task()
        except exception.ResourceFailure as ex:
            ...
 
 这个stack_task有三个重要的点：

 * scheduler.wrappertask进行了装饰，意味这task需要处理subtask 
 * scheduler.DependencyTaskGroup，构造该对象，即拓扑结构的图，该图上的每个节点都是一个资源，即资源上执行的task，即stack_task的子任务 
 * yield action_task()，这是yield关键字的使用，前面介绍的文档说了，有yield之后，函数就成为一个对象了，那么直接调用stack_task不会执行该函数，而是返回一个迭代器对象

注意到这三点之后，我们知道其中的关系可以描述为如下：

<img src="/assets/img/openstack_heat_create_resources.png" atl="OpenStack Heat Resources Create" width="700px"/>

这张图惨不忍睹，大致想表达的意思是TaskRunner要做的事，最后就是一层包含一层的了，最里面的Task代表着DependencyTask的顺序执行。从一个create task开始，运行一个经过装饰的task，意味着这个task具有多个sub task，每个sub task来源于资源拓扑图中的每一个节点，这些sub task都是调用对应资源的handle_create方法。

`engine/scheduler.py`文件定义了`Class TaskRunner, def wrappertask, class DepencyTaskGroup`，其中Class TaskRunner中的关键函数是`def start`，详细的来看该函数的实现，

    143     def start(self, timeout=None):
    144         """
    145         Initialise the task and run its first step.
    146 
    147         If a timeout is specified, any attempt to step the task after that
    148         number of seconds has elapsed will result in a Timeout being
    149         raised inside the task.
    150         """
    151         assert self._runner is None, "Task already started"
    152 
    153         logger.debug(_('%s starting') % str(self))
    154 
    155         if timeout is not None:
    156             self._timeout = Timeout(self, timeout)
    157 
    158         result = self._task(*self._args, **self._kwargs)
    159         if isinstance(result, types.GeneratorType):
    160             self._runner = result
    161             self.step()
    162         else:
    163             self._runner = False
    164             self._done = True
    165             logger.debug(_('%s done (not resumable)') % str(self))

这个函数主要是执行传递进来的Task，一个函数，`重点看158，159两行，注意158行的result = self._task(*self._args, **self._kwargs)`不一定是去执行这个函数，就因为`yield`关键字的存在导致其成为一个Generator对象了，所以下 一步有个类型判断，对于GeneratorType意味着需要一步步执行，调用step函数。若没有yield关键字，一切正常如以往。step函数则调用与yield配合的next函数，最后在run_to_completion函数中，循环的调用next函数，直至yield抛出StopIteration异常。

考虑被装饰了的函数，看`def wrappertask`函数，注释解释了该函数的用途 `Decorator for a task that needs to drive a subtask. `。 被装饰的函数需要这样写，使用yield关键字。

    @wrappertask
    def parent_task(self):
        self.setup()

        yield self.child_task()

        self.cleanup()

Class Stack中的create函数正是这么写的。在wrappertask中，通过`parent = self._task(*args, **kwargs)`会获取到GeneratorType的对象，然后对其调用next，得到了`def stack_task中通过yield action_task() 返回的GeneratorType，就是实际的DependencyTaskGroup`，然后对于substask，调用next，促使DependencyTaskGroup执行，yield出结果，返回给TaskRunner，TaskRunner则不停next，完成了yield，next的配合。

在DependencyTaskGroup中，完全按照拓扑排序来执行资源的创建，首先选取一批入度为0的节点执行，然后下一批，直到结束。

### 总结 
-----------

理解了yield关键字，就可以很好的理解了heat中的scheduler的实现方式，这种scheduler相比Nova Scheduler来说是两种完全不同的调度，并且都是很简单的基础调度思想。前者是提供了一个简单的框架，完成一批有序任务的按序执行，后者只是更多的是根据任务的要求，调度分配合适的资源来满足任务，Nova Scheduler 存在更多需要扩展的地方，以满足更多的需求。

这个scheduler虽说简单，但至少提供了特定需求的实现方式，可以用来作为参考。

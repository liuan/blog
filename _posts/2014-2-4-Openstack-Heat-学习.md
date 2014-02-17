---
layout: post
category: OpenStack
tags: OpenStack
---

随着更深入的了解OpenStack，越发感觉到Heat在一定程度上更体现出了云计算与服务器虚拟化的差别，Heat完成了OpenStack从一键虚拟机创建到一键应用系统创建的巨大跨越，并且结合监控实现了所谓的弹性扩展。Heat的作用意义非凡，而它的实现更是充满了趣味。

### 入门
------------------

在了解Heat的实现之前，我们先看看Heat的简单知识。前面有[一篇拙劣的博客对OpenStack Heat](http://www.choudan.net/2013/11/25/OpenStack-Heat-Project%E4%BB%8B%E7%BB%8D.html)做了简单的介绍，主要是围绕Heat的基本功能和工作流程。对heat的概念性理解重点参考下面这位大神的文章。

* [OpenStack G版孵化项目Heat介绍](http://blog.csdn.net/wangyish201201/article/details/8933581)

借用上面文章中的话：

> 对于Heat的功能和实现，简单来说就是用户可以预定义一个规定格式的任务模板，任务模板中定义了一连串的相关任务（例如用某配置运行几台虚拟机，然后在其中一台中安装一个mysql服务，设定数据库属性，然后再配置几台虚拟机安装web服务器集群等等），然后将模板交由Heat执行，Heat就会按一定的顺序执行模板中定义的一连串任务。

往往，这个任务模板中的一连串任务就是用来指导openstack创建应用系统。在heat中，这个创建的应用系统称之为Stack。Nova是直接创建虚拟机，Heat则是直接创建应用系统。

那么如何使用heat来创建Stack了，下面这位大神的文章就给我们指出了一条明路，很好的一个完整演示。

* [OpenStack Heat服务介绍](http://zhenhua2000.blog.51cto.com/3167594/1324918/)

其实，Heat的使用方式和其他的OpenStack Project十分相似，也是通过python-XXXclient提供的命令来直接操作。值得指出来的是，OpenStack不同project的命令形式都具有很好的一致性，降低了学习成本。

有了上面的基础之后，我们可以再进一步探讨heat的使用。对于Heat的使用，我们已然感受到了，最为关键的是模板文件的创建。

### 模板
---------------

我们知道Heat的目的之一就是致力于应用系统的自动化部署，那么若要自动化部署，则需要存在某个语言规范来描述应用系统，并且解决应用系统在不同场合下的配置适应问题。Heat模板文件则是用来对前者的支持。

在此先插入一篇博客，增加对heat的深入了解。

* [OpenStack中的Heat进阶](http://blog.csdn.net/lynn_kong/article/details/17195047)

模板文件的格式多种多样，例如，Heat的鼻祖Amazon提供的cloudformation格式，Heat自有的格式HOT, Json等等，格式之间的差别在于表现形式。以[Amazon的cloudformation](http://docs.aws.amazon.com/AWSCloudFormation/latest/UserGuide/gettingstarted.templatebasics.html)为例，模板文件大致分为以下几个部分：

* AWSTemplateFormatVersion: 版本信息
* Description: 描述信息
* Parameters: 一些自定义的变量，可以提供默认值，可选的值等等
* Mappings: 一些映射，例如"AWSInstanceTypeArch:" {"m1.tiny":{"Arch":"32"}, "m1.small" : {"Arch":"64"}}，这就是一个字典
* Resources: 描述了一些资源，例如Instance，Network等等，资源有Type，Properties等等信息
* Outputs: 返回值

模板文件的这些部分中最为关键的是Resources段，Resource具有很多的attribute，例如type，properties等等，还有一些可选的attribute，例如DependsOn, DeletionPolicy, Metadata等等。除了属性之外，属性中还会用到一些函数，例如：Fn:Base64, Fn:FindInMap, Fn:GetAttr等等。通过resource，我们可以描述应用系统包含的虚拟机，虚拟机的属性，开机初始化信息，虚拟机软件栈的配置以及应用系统的网络等等信息。

例如，从heat-templates中摘取的一个resources例子，描述的LoadBalancer，有类型，属性等，用到了Fn::GetAZs函数。

    "LoadBalancer" : {
          "Type" : "AWS::ElasticLoadBalancing::LoadBalancer",
          "Properties" : {
            "AvailabilityZones" : { "Fn::GetAZs" : "" },
            "Instances" : [{"Ref": "WikiServerOne"}],
            "Listeners" : [ {
              "LoadBalancerPort" : "80",
              "InstancePort" : "80",
              "Protocol" : "HTTP"
            }]
          }
        }

对于Instance类型的resource，常会有下面这些可选的attribute：

* Metadata: 作为nova创建虚拟机时指定的meta选项， 主要完成虚拟机启动时的基本配置，例如安装软件包，启动服务等等。
* Properties中的UserData: 一般是脚本文件内容，根据环境来配置虚拟机的软件栈。

上面例举的attribute与虚拟机软件栈的配置，服务的安装启动密切相关，通过在虚拟机内部安装cloudinit和heat-cfntools这两个工具，结合nova-metadata完成应用的自动化部署的大量工作。

需要指出的是cloudformation中的某些字段，heat目前还没有支持。可以在[这个页面查看heat目前支持的内容](http://docs.openstack.org/developer/heat/template_guide/index.html)。理解resources对于理解整个heat的实现十分重要。

### heat-engine
------

整个heat的实现最为关键的代码在heat-engine，heat-engine的实现没令人失望，充满了趣味。前面提到，heat就是来操作stack，管理stack的整个生命周期: create，update，delete。

重点看create的过程，查看heat stack-create命令： 

    choudan@ubuntu:~$ heat help stack-create
    usage: heat stack-create [-f <FILE>] [-e <FILE>] [-u <URL>] [-o <URL>]
                             [-c <TIMEOUT>] [-r] [-P <KEY1=VALUE1;KEY2=VALUE2...>]
                             <STACK_NAME>
    Create the stack.
                                                      
    Positional arguments:
        <STACK_NAME>          Name of the stack to create.
                                                        
    Optional arguments:
        -f <FILE>, --template-file <FILE> 
                    Path to the template.
        -e <FILE>, --environment-file <FILE>
                    Path to the environment.
        -u <URL>, --template-url <URL>
                    URL of template.
        -o <URL>, --template-object <URL>
                    URL to retrieve template object (e.g from swift)
        -c <TIMEOUT>, --create-timeout <TIMEOUT>
                    Stack creation timeout in minutes. Default: 60
        -r, --enable-rollback
                    Enable rollback on create/update failure
        -P <KEY1=VALUE1;KEY2=VALUE2...>, --parameters <KEY1=VALUE1;KEY2=VALUE2...>
                    Parameter values used to create the stack.

三个关键的optional arguments:

* template-file: 模板文件
* environment-file: 环境文件
* parameters: 设置模板文件中的parameters

### `后续补充`

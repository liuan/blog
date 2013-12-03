---
layout: post
category: OpenStack
tags: OpenStack
---

OpenStack的项目貌似越来越多了，在Grizzly版之前，每个项目都得实现一套处理配置文件的代码。在每个项目的源码中基本上都可以找到`openstack/common/cfg.py，iniparser.py`文件，当然，这些不同项目之间的cfg.py等文件很大可能是copy-and-paste分分钟来搞定。这种情况肯定无法被大神忍受，最终，社区决定改变这一切，提出了Oslo项目。Oslo项目的宗旨是提供一系列OpenStack Projects共享的基础库，可以从wiki的原话中了解到。

> To produce a set of python libraries containing code shared by OpenStack projects. The APIs provided by these libraries should be high quality, stable, consistent, documented and generally applicable.

社区显然已经无法忍受在不同项目中大量重复的代码了。Oslo 项目提供了一系列的库，我们接触的最多的为oslo.config这个库，要来处理程序命令行参数和配置文件。当然还有其他的，例如，[pbr](https://pypi.python.org/pypi/pbr)(Python Build Reasonableness)与setuptools相关的库，[hacking](https://pypi.python.org/pypi/hacking)，用来处理编码风格的库，还有[oslo.messageing](https://github.com/openstack/oslo.messaging)等等。这些库有的是在oslo这个namespace下，有的是完全独立的。Oslo开发者是这样考虑的，如果这个库存在被广泛使用的潜质的话，则不将其放在oslo命名空间下。

Oslo项目包含的库较多，我们将目光聚集到接触最多的oslo.config这个库上，这个库应该是所有OpenStack项目中重用最多的。

### oslo.config
******

在了解oslo.confg的使用和实现之前，我们需要知道，这个库是用来解决什么样的问题。在有了此问题答案的基础上，然后沿着怎样来使用这个库和这个库到底是如何实现的路线来分析。

前面我们介绍了，OpenStack在G版之前的几乎每个项目都得拷贝一份cfg.py，iniparser.py两个文件放到`openstack/common/`目录下，这两个文件主要致力于解决读取配置文件和解析命令行参数的问题。在实际使用OpenStack的过程中，我们启动一个服务，例如nova-api或者glance-api，往往都是这样的形式：

> /usr/bin/nova-api --config-file=/etc/nova/nova.conf --log-file=/var/log/nova/api.log

从这个启动命令来看，我们需要能够正确的处理命令行参数，还有配置文件。细心观察会发现，不同的服务，nova-api，glance-api等等，都会有一些共同的命令行参数，如上面的--config-file，--log-file等等，然后每个服务还有自己专属的命令行参数。对于配置文件，可能存在多个，例如，nova项目存在多个服务，nova-api，nova-compute等等。那么这些nova services之间会存在大量共同的配置，对此，Oslo建议如果支持多个配置文件的话，那么就很给力了，像这个形式：`--config-file=/etc/nova/nova-commmon.conf --config-file=/etc/nova/nova-api.conf`。对配置文件格式的支持，目前主要是[ini风格的文件](http://en.wikipedia.org/wiki/INI)。除了解析配置选项之外，另一个问题是，快速访问到这些配置选项的值。

因此，olso.config wiki上贴出了oslo.config需要解决的几点问题：

* command line option parsing
* common command line options
* configuration file parsing
* option value lookup

在wiki中还提到一种场景，建议最好将一些options的默认值写在code里面，同时也在config file中作为注释表明。这应该就是在config中看到的很多被注释掉的配置，在代码中同样可以看到这些默认值。

### oslo.config使用
*************

oslo.config库只有两个文件，cfg.py和iniparser.py，oslo.config的使用方法在cfg.py文件中已经给出不是一般详细的注释。

##### options

options即所谓的配置选项，可以通过命令行和配置文件设置，它一般的形式如下：

    common_opts = [
        cfg.StrOpt('bind_host',
                    default='0.0.0.0',
                    help='IP address to listen on'),
        cfg.IntOpt('bind_port',
                    default=9292,
                    help='Port number to listen on')
    ]

上面的一般形式，指定了options的名字，默认值和帮助信息，还可以看出，不同的配置选项属于不同的类型。StrOpt，IntOpt。除此之外，Options还支持floats，booleans，list，dict，multi strings。

这些options在被引用之前，必须先在运行期通过config manager注册该options，即使用前得先注册。例如下的情况：

    class ExtensionManager(object):

        enabled_apis_opt = cfg.ListOpt(...)

        def __init__(self, conf):
            self.conf = conf
            self.conf.register_opt(enabled_apis_opt)
            ...
                                                        
        def _load_extensions(self):
            for ext_factory in self.conf.osapi_compute_extension:
            ....

我们若要使用osapi_compute_extension选项，则需要先通过self.conf.register_opt(enabled_apis_opt)完成option的注册。

前面我们提到options可以在启动服务的命令行中启动，这些选项在被程序解析之前，必须先通过config manager注册。这样的好处，我们可以实现常用的help参数，并且确认命令行参数的正确性。命令行的注册略微不同前面提到的注册方式，调用的是特定的函数，conf.register_cli_opts(cli_opts)。

    cli_opts = [
        cfg.BoolOpt('verbose',
                    short='v',
                    default=False,
                    help='Print more verbose output'),
        cfg.BoolOpt('debug',
                    short='d',
                    default=False,
                    help='Print debugging output'),
    ]

    def add_common_opts(conf):
        conf.register_cli_opts(cli_opts)

##### config file

前面我们提到oslo.config支持的是ini风格的配置文件，该文件将所有的配置选项进行了分组，即所谓的section或者group，这两个单词是同一个概念，没有指定section的，则会分到default组。下面给出了一个ini风格的配置文件例子：

    glance-api.conf:
        [DEFAULT]
        bind_port = 9292
                
    glance-common.conf:
        [DEFAULT]
        bind_host = 0.0.0.0

在config manager中，会默认的指定两个值，即`--config-file --config-dir`,config manager会在没有显示指定这两个参数的情况下去默认的文件夹中查找默认的文件。例如`~/.${project}， ~/， /etc/${project}，/etc/`这几个目录下查找配置文件，如果程序是nova，则会查找默认路径下的nova.conf文件。

在代码中的注释指出，`Option values in config files override those on the command line.` 即config files中的选项值会覆盖命令行中的选项值。这貌似与潜意识中的相反呀，英文是原话。`补充：2013-11-28，经过自己的测试和对源码的阅读，应该是Option values specified on command lines override those in config files，具体参考下一篇的分析。` 多个配置文件会按顺序来解析，后面文件中的选项会覆盖前面出现过的选项。

##### option group 

在配置文件中，我们已经看到很多配置选项已经被我们主动的进行了一个分组的划分，没有归属的选项则扔到了default组。同样，在代码中options可以显示的注册某个组中。注册的方式有两种，直接指定group，或者指定group的name，参考下面代码：

    rabbit_group = cfg.OptGroup(name='rabbit',
                                title='RabbitMQ options'))
    rabbit_host_opt = cfg.StrOpt('host',
                                 default='localhost',
                                 help='IP/hostname to listen on')

    rabbit_port_opt = cfg.IntOpt('port',
                                default=5672,
                                help='Port number to listen on')

    def register_rabbit_opts(conf):
        conf.register_group(rabbit_group)
        # options can be registered under a group in either of these ways:
        conf.register_opt(rabbit_host_opt, group=rabbit_group)
        conf.register_opt(rabbit_port_opt, group='rabbit')

我们需要先定义一个group，指定group的name和title属性，也得将group注册，最后可通过两种方式将options注册到该组中。

若一个group仅只有name属性，那么我们可以不用显示的注册group，例如下面的代码：

    def register_rabbit_opts(conf):
        # The group will automatically be created, equivalent calling::
        #   conf.register_group(OptGroup(name='rabbit'))
        conf.register_opt(rabbit_port_opt, group='rabbit')

##### option values

若要引用某个option的值，则直接通过访问config manager属性的方式即可。例如，访问default组或者其他的组，可以通过如下的方式：

    conf.bind_port  conf.rabbit.port

同时，option值还可以通过[PEP 292 string substitution](http://www.python.org/dev/peps/pep-0292)(pep 292描述了字符串替换的方式)再引用其他的option的值，具体看下面的例子，在sql_connection值中，我们引用了其他的option的值。

    opts = [
        cfg.StrOpt('state_path',
                    default=os.path.join(os.path.dirname(__file__), '../'),
                    help='Top-level directory for maintaining nova state'),

        cfg.StrOpt('sqlite_db',
                    default='nova.sqlite',
                    help='file name for sqlite'),

        cfg.StrOpt('sql_connection',
                    default='sqlite:///$state_path/$sqlite_db',
                    help='connection string for sql database'),
    ]

还有在某些情况下，我们需要在日志文件中隐藏关键option的值，可以在创建该option时，添加secret参数，设置为True。

##### config manager

我们已经多次提到config manager了，要使用options，得先将options注册到config manager中，访问option的值，直接访问config manager的属性，config manager对options进行了统一的管理，其实config manager是一个全局的对象，重载了__call__方法，还有__getattr__方法。最为关键的，全局就只有这么一个实例，在cfg.py的注释尾，再给出了一个完整的例子，首先获取全局的这个实例，然后注册options，最后使用options。

    from oslo.config import cfg

    opts = [
    cfg.StrOpt('bind_host', default='0.0.0.0'),
    cfg.IntOpt('bind_port', default=9292),
    ]
                            
    CONF = cfg.CONF
    CONF.register_opts(opts)
                                    
    def start(server, app):
        server.start(app, CONF.bind_port, CONF.bind_host)

### 总结
***********

这一部分只是按照源码中的注释来介绍了下oslo.config的使用，下一篇，将分析cfg.py的代码结构，因为也只有这一个关键的文件，代码在两千行左右，任务不是很重，所以争取将其看仔细，写明白。

在阅读oslo wiki的时候，发现了一个很有趣的问题，`Why does oslo.config have a CONF object? Global object SUCK!`，看来社区对这个Global Object有很大的争论，导致作者还特意在wiki上做个专门的介绍！我们在使用cfg时，一般都是通过 CONF = cfg.CONF方式来获取这个全局的实例。作者提到在Folsom Design Summit上，有人想`remove our dependence on a global object like this`，显然，很多争论，结果的结果是，大家达成了一个初步共识，还是坚持使用这种global object的方式。作者还提到了一句话：`The idea is that having all projects use the same apporach is more important than the objections to the approach.` 不明觉历的模样！这还是强调了OpenStack的projects使用同样的方法更好！具体的回答大家可以点击后面的wiki链接，自己细读。这个问题的最后，作者留下了一句`This debate will probably never completely go away,though.`。

由于对这个问题理解不深，也不知道大神们为什么要争论这几种方式，难道这个global objects还不够好！

### 参考文档
******

1. [https://wiki.openstack.org/wiki/Oslo](https://wiki.openstack.org/wiki/Oslo) Oslo WiKi
2. [https://wiki.openstack.org/wiki/Oslo/Config](https://wiki.openstack.org/wiki/Oslo/Config) Oslo.config WiKi


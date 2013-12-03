---
layout: post
category: OpenStack
tags: OpenStack
---

上一篇博客简单的介绍了[Olso.config的背景和使用方法](http://www.choudan.net/2013/11/28/OpenStack-Oslo.config-学习%28一%29.html)，这一篇博客中，我们尝试去分析下其源码。在分析其源码之前，自己有些许困惑，分析其源码是否有必要，就算分析完之后，又能有什么收获！这个一直伴随着我看源码的过程，最后将在末尾写出自己的一点感想。

废话不多说了！

### argparse
**********

从`oslo.conf.cfg.py`文件的import可以看出，其所依赖的最重要的库是argparse，这个库在openstack中用的很多，尤其是各种project的client，例如，python-novaclient，python-glanceclient等等，这些client最先需要处理的便是命令行参数，这正是argparse的用武之地。对于argparse的使用，本文不再多述！大家先看这两个篇文章，有个基本了解。

* [argparse - 命令行选项与参数解析（译）](http://youngsterxyf.github.io/2013/03/30/argparse/) 
* [Argparse Tutorial](http://docs.python.org/2/howto/argparse.html)

在此，我们不需要明白argparse的实现，只需要了解它的一些基本用法，其中一些关键的概念将会涉及到后面对oslo.config的分析，列在此处。

* postional argument: 即所谓的定位参数，这种参数在参数列表中是讲究位置的，例如cp 命令， `cp src dest eg: cp ./tmp.txt ../`， 其中的./tmp则只能是src，../是dest，顺序不能反。在python-XXXclient中，很多这样的定位参数，例如nova boot ... boot则是定位参数
* ArgumentParser: 解析器对象，argparse的第一步就是创建一个解析器对象，然后，需要在此解析器中添加需要解析的参数。参考上面的链接。
* action: 在向解析器添加参数时，可能需要指定action，例如：parser.add_argument('-b',action="store_true",default=False)，这其中的action是用在解析参数时触发的动作，此action还可以自己定义，或者重载，更为详细的内容参考上面的链接 。
* namespace: 我们需要注意到parse_args()的返回值是一个命名空间，包含传递给命令的参数，这个命名空间对象将参数保存为命名空间的属性。

argparse库绝对是大大的简化了命令行参数的处理，再也不要担心处理参数了。整个Oslo.config库最终也是落脚到这个argparse库上来，完成各种各样的解析任务。


### olso.config
*****

前一篇已经提到oslo.conf库需要解决的核心问题了，对命令行，配置文件的解析，对option values的便捷访问。那么，在开始后面的源码分析之前，我们试着在自己来设计这个库的假设基础上提几个问题，然后看源码中是如何处理的，这样可以阅读源码时有个指导目标，不至于迷失。

* 我们知道对命令行参数的解析可以使用强大的argparse库，那么是否可以基于argparse把对命令行和配置文件的解析统一起来？
* 从OpenStack Project的源码中，我们看到大量的py文件，都会先register该文件中将会使用的配置选项，然后再使用，换句话说，不同的模块导入的顺序不一样，这样不同的配置选项register和使用的时间不同，那么该如何处理不同时刻register的选项了，能否统一管理？
* 我们知道有的options，只在命令行中存在，而有的在配置文件中存在，还有的即在命令行中存在也在文件中存在，那么如何管理这些不同来源的options，如何确定它们的重载关系，即能获取到options正确的值？
* 上面提到argparse调用parse_args后返回的一个namespace，options设置为该namespace的属性，访问options的值则直接访问namespace的相应属性，那么怎样做到迅速的访问options的值了? 解析配置文件之后，配置文件中所有的选项能够作为namespace的属性直接访问不？

带着这些疑问再来阅读代码，就更有条理了，也许对日后写代码会有启发。

##### Opt

先看一个简单的argparse例子，代码如下：

    parser = argparse.ArgumentParser(add_help=False) # 创建解析器
    parser.add_argument('--show','-s',action="store",help="show message"，default="You Know") # 向解析器添加options
    ns = parser.parse_args() # 解析参数，返回一个namespace
    print ns.print # 访问namespace的属性

    ### 执行结果
    choudan@ubuntu:~$ python arg.py 
    You Know
    choudan@ubuntu:~$ python arg.py -s "You are so beautiful"
    You are so beautiful

上面四行代码是argparse的简单使用，创建解析器之后，然后将需要解析的options添加到解析器中，注意add_argument的参数，一系列的option的属性，分为args(tuple)和kwargs(dict)类型，前者指定了options的name，dest，后者指定了其他属性，action，type，default等等。然后解析命令行参数，最后读取解析的结果。到此，我们知道了解析命令行需要做的基本事情。

看完上面这一小段代码之后，我们回到oslo.conf.cfg.py的源码中，cfg.py的一个重要的类便是Opt，该类是一个基类，用来对options的封装，从上面演示的代码中，我们知道了，一个options包含很多的属性，这些都封装到了Opt这个基类中。除了属性之外，Opt还需要为两个功能提供服务：

1. 将option添加到解析器中去，这就需要准备args和kwargs参数
2. 从namespace中获取option的值

对于第一点存在对应的函数，_add_to_cli，

    def _add_to_cli(self, parser, group=None):
        container = self._get_argparse_container(parser, group)
        kwargs = self._get_argparse_kwargs(group)
        prefix = self._get_argparse_prefix('', group.name if group else None)
        for opt in self.deprecated_opts:
            deprecated_name = self._get_deprecated_cli_name(opt.name,
                                                            opt.group)
            self._add_to_argparse(parser, container, self.name, self.short,
                                  kwargs, prefix,
                                  self.positional, deprecated_name)


* 第一行，获取一个container，其实就是一个ArgumentParser，解析器
* 第二行，获取该option的kwargs参数，包括help，default，type，action等等属性
* 第三行，对该option的name进行一个前缀修饰，其实就是若存在group name，则将其加到option name的前面
* 后几行，将这些准备好的args，kwargs添加到container中

对于第二点，则存在对应的函数，_get_from_namespace，这个很简单，不多阐述了。

我们知道，配置选项的值会存在很多的类型，有int，string，list等等，从nova.conf这个配置文件中也可以知道，不同的option对应的值类型会可能不一样。例如，`monkey_patch=false 是bool型，osapi_compute_ext_list= 是list类型等等`。从命令行或者配置文件中首先读取到的options值都是string类型，然后需要转换成指定的类型，执行指定的action。这就导致不同类型的option需要对Opt的某些方法进行重载。

大多数子类会根据自己类型的需求，重载`_convert_value和_get_argparse_kwargs`函数。

上面这些都是倾向于命令行参数的封装，那么对于配置文件和目录这样的选项了，是如何统一起来管理的了。上面的图告诉我们，_ConfigFileOpt和_ConfigDirOpt也是继承于Opt类，这样，就将他们统一起来了。那么对配置文件解析的部分就封装在了action属性中。_ConfigFileOpt实现了一个内部类ConfigFileAction，该类只重载了一个方法__call__，使其能够作为函数一样调用。 

    class ConfigFileAction(argparse.Action):
        def __call__(self, parser, namespace, values, option_string=None):
            """Handle a --config-file command line argument.
            if getattr(namespace, self.dest, None) is None:
                    setattr(namespace, self.dest, [])
                items = getattr(namespace, self.dest)
                items.append(values)

                ConfigParser._parse_file(values, namespace)

在最后一行，则调用了ConfigParser的方法来解析该values指定的文件，将解析的结果存储在namespace中。前面我们还提到一个问题，就是文件中所有的配置选项都会设置成namespace的属性吗？那么配置文件中的选项在使用前还需要注册不？其实，需要使用的选项都需要遵守先注册后使用的原则，只是在设置namespace属性时，会过滤掉没有注册的配置选项。

##### ConfigOpts

有了多Options的基本封装之后，剩下的任务就是将所有的Options统一管理起来，这就是ConfigOpts的工作了。首先我们来看下ConfigOpts的使用例子，

    from config import cfg

    opts = [
        cfg.StrOpt('bind_host', default='0.0.0.0'),
    ]
    cli_opts = [
        cfg.IntOpt('bind_port', default=9292),
    ]

    CONF = cfg.CONF
    CONF.register_opts(opts)
    CONF.register_cli_opts(cli_opts)

    print CONF.bind_host # 注册就直接使用
    print CONF.bind_port

    CONF(args=sys.argv[1:]) # 解析命令行参数 配置文件
    print CONF.bind_host
    print CONF.bind_port

    choudan@ubuntu:~$ cat oslo.conf
    [DEFAULT]
    bind_host = 192.168.0.1
    bind_port = 55553

    choudan@ubuntu:~$ python test_oslo.py --config-file ./oslo.conf --bind_port = 9090
    0.0.0.0
    9292
    192.168.0.1
    9090

上面的代码分为两个阶段，前面先注册options，然后直接使用，后面再解析命令行参数和配置文件，然后再使用配置选项，从运行的结果来看，文件中的配置值重载了注册时的默认值，命令行中指定的值重载了配置文件中写入的值。那么，这些options是如何被ConfigOpts管理起来的了，如何和argparse交互起来的了？

从ConfigOpts的构造函数中我们就可以窥见一二。 

    def __init__(self):
        """Construct a ConfigOpts object."""
        self._opts = {}  # dict of dicts of (opt:, override:, default:)
        self._groups = {}
        self._args = None
        self._oparser = None
        self._namespace = None
        self.__cache = {}
        self._config_opts = []

ConfigOpts持有多个dict类型的容器，有存储不署于任何groups的options的容器_opts，还有存储groups的容器_groups，还包含一个解析器，一个namespace，cache等等。

调用register_opts或register_cli_opts都是将该options直接存放到_opts或者_groups中，所以上面的代码在注册完之后，就可以直接来使用options了。

解析命令行参数和配置文件的工作则都交给了CONF(args=sys.argv[1:])这一行代码了。这背后的逻辑都是ConfigOpts的__call__方法。

    def __call__(self,args=None,project=None,prog=None,version=None,usage=None,default_config_files=None):
        self.clear()
        prog, default_config_files = self._pre_setup(project,
                                                     prog,
                                                     version,
                                                     usage,
                                                     default_config_files)
        self._setup(project, prog, version, usage, default_config_files)
        self._namespace = self._parse_cli_opts(args if args is not None
                                               else sys.argv[1:])
        if self._namespace.files_not_found:
            raise ConfigFilesNotFoundError(self._namespace.files_not_found)
        self._check_required_opts()

* 第一行：将除了_opts和_groups中注册的options之外，清空各种容器，将parser,namespace均置空，
* 第二行：创建parser，这样ConfigOpts就拥有一个解析器了
* 第三行：将命令行中的参数都注册起来，其中config-file和config-dir如果没有指定，系统则会在一些默认的路径下搜索特定的文件
* 第四行：开始真正的解析之路，并将结果返回给self._namespace这个命名空间
* 最后行：检查某些一定需要的options是否存在了

ConfigOpts解析命令行和配置文件的秘密都封装到这几个函数当中了。ConfigOpts下一个重要的工作是快捷的获取到options的值。在ConfigOpts类中重载了__getattr__函数，该函数简单的调用了_get方法，我们看下_get方法的实现：

    def _get(self, name, group=None, namespace=None):
        if isinstance(group, OptGroup):
            key = (group.name, name)
        else:
            key = (group, name)
        try:
            if namespace is not None:
                raise KeyError

            return self.__cache[key]
        except KeyError:
            value = self._substitute(self._do_get(name, group, namespace))
            self.__cache[key] = value
            return value

可以看到，_get方法会首先尝试在cache中进行查找，查找失败则会调用_do_get进行深一步查找，然后将结果保存在cache中，下一次就可以直接从cache中取了。其中的_substitute方法是用来处理字符串替代的。最关键的是_do_get函数，我们截取该函数的代码看看

    def _do_get(self, name, group=None, namespace=None):
        if group is None and name in self._groups:
            return self.GroupAttr(self, self._get_group(name))
        info = self._get_opt_info(name, group)
        opt = info['opt']
        if isinstance(opt, SubCommandOpt):
            return self.SubCommandAttr(self, group, opt.dest)
        if 'override' in info:
            return info['override']
        if namespace is None:
            namespace = self._namespace
        if namespace is not None:
            group_name = group.name if group is not None else None
            try:
                return opt._get_from_namespace(namespace, group_name)
            except KeyError:
                pass
            except ValueError as ve:
                raise ConfigFileValueError(str(ve))
        if 'default' in info:
            return info['default']
        return opt.default

可以看到，如果namespace还不存在，就直接返回默认值，这与我们在前面演示的小例子一致。若namespace存在，则调用该options的_get_from_namespace方法，查找到解析之后的结果。

至此，关于ConfigOpts的核心代码就剖析完毕了。

### 总结 
******

对于oslo.config的理解，还是需要基于对argparse的使用上来看，oslo.config只是将一些东西进行合理的封装，然后方便的用户来使用。简言之，oslo.conf就是封装两个事，`向解析器添加需要解析的项，其次是从namespace获取解析之后的结果。`

对于这么一个小的库，是否有必要如此深入的阅读其代码，我感觉如果在学习python的初期阶段，还是可以好好看看这份代码，其中涉及到很多python的magic，也有很浓的面向对象设计思想在其中。对于这样的工具库，反而觉的看明白其实现原理是其次的，而是背后为了完成作为OpenStack 的解析基础库的设计思想，这个应该是很有启发的，可以运用在今后自己独立完成某任务的设计过程中。经验渐长之后，还是只要知道怎么用就行了，python的工具库太多太多了。

还有对于前面提出的四个问题，可能还有第二个没在上文中涉及到，也就是如果现在已经调用CONF()完成了解析，而后还有其他的模块register一些options，然后访问这些options的值，是否能成功？我们前面提到CONF（）会触发解析过程，然后根据注册的选项过滤配置文件中的选项，只有注册了的选项才会成为namespace的属性，而在后面注册的options，是否还需要解析，其实是不需要的，在第一次解析的时候还是保存了配置文件解析的结果，只需要从这里面查找就可以了。

### 参考文档
******

1. [http://www.cnblogs.com/jianboqi/archive/2013/01/10/2854726.html](https://www.cnblogs.com/jianboqi/archive/2013/01/10/2854726.html) argparse 导入
2. [http://youngsterxyf.github.io/2013/03/30/argparse/](http://youngsterxyf.github.io/2013/03/30/argparse/) argparse - 命令行选项与参数解析（译）


---
layout: post
category: OpenStack
tags: OpenStack
---

在上一篇[OpenStack Small Tests](http://www.choudan.net/2013/08/12/OpenStack-Small-Tests.html)中翻译了OpenStack Wiki中关于Small Tests的介绍。关于测试有很多的术语，例如单元测试，功能测试，集成测试，白盒测试，黑盒测试，其实这写都是对某些同样的测试目的不同的术语而已。而OpenStack Wiki上将这些都统一成了Small Test, Medium Test, Large Test,分别对应于单元测试，功能测试，集成测试。不同的测试阶段，都已经有不同的测试工具了。

Small Tests中已经提到了Mox, Stubout等等python库，Large Tests则是使用Tempest。

第二次向OpenStack Nova提交的patch，主要是修改了api下面的hosts.py文件中的index函数，对应的api是`nova host-list`，目的是扩充原本的host-list返回的属性，简单的增加了此属性，然后在我搭建的OpenStack平台测试了一遍，注意，我是直接将修改后的代码替换安装的文件的，而不是做的单元测试，结果正是预料的，很正确，然而提交到Nova上之后，遇到了错误。首先是`gate-nova-pep8`测试失败，然后是`gate-nova-python26/***`测试失败。


### pep8

patch提交上去之后，第一件事是对代码风格进行检测，是否符合指定的编码规范。对代码风格的测试主要是使用[flake8](http://flake8.readthedocs.org/en/2.0)和[hacking](https://github.com/openstack-dev/hacking),其中这儿有对[代码编写风格的规范指导 OpenStack Style Guidelines](https://github.com/openstack-dev/hacking/blob/master/HACKING.rst)，这只是其中的Opentack自己的部分，还有整个[python语言的代码规范pep8](http://www.python.org/dev/peps/pep-0008/)。后者pep8是python官方推出的指南，描述了Python编程风格的方方面面，遵守此规范，可以保证不同的程序猿写的python代码具有最大程度的相似风格，易于阅读，交流。前者OpenStack hacking 则是在pep8的基础上，针对OpenStack开发者做的补充。

Jenkins首先检测你的编码规范是否符合要求，若不符合要求，会在测试结果中给出详细的说明。由于刚接触，没有经验，则出现了两个代码风格上的问题。

1. import *** 时，需要按照字母表的顺序在文件前头依次写下多个import语句
2. 我在写字典的某项时"status":art,其中冒号没有与art空一格，应该是"status": art.

### Small Test

如果修改了某个API，必须要现在本地做好测试，不然提交上去测试通不过。修改API后，需要做的测试是Small Tests。前面的文章是告诉我们如何编写Small Test case, 但是在Nova Project中到底该如何进行Small Tests了。

在nova的目录下面有一个`.run_tests.sh`的脚本，第一次运行它，会询问要不要创建虚拟环境，创建虚拟环境则会在本地创建一个目录.venv，然后将需要用到的python库安装到该目录下，建立一个隔离的环境，这样就不会因为系统中python某个库版本和nova中要求的冲突了，并且可以很方便的建立多个虚拟环境，方便测试。

然而，在我创建完虚拟环境，运行run_tests.sh后，出现了下面的错误。`invalid command testr`，之前还出现过找不到testr command，这需要使用`pip install testrepository`安装testrepository python 库，该库主要用来将存储测试结果，方便对大量的测试结果进行查询管理。运行run_tests.sh失败之后，在OpenStack Launchpad Nova bugs上看到类似的问题，有的建议直接在真实环境中运行，`./run_tests.sh -N`，这样就运行通过了，开始运行tests目录下所有的测试用例了。 

其实，测试所有的测试用例，比较费时，可以参考[这篇wiki,nova 开发者的 unit test](http://docs.openstack.org/developer/nova/devref/unit_tests.html)。

譬如测试tests下的某个子集：

    ./run_tests.sh scheduler //测试某个目录下的用例
    ./run_tests.sh test_libvirt //测试某个文件中的用例
    ./run_tests.sh test_libvirt:HostStateTestCase //测试某个文件中的class用例
    ./run_tests.sh test_utils:ToPrimitiveTestCase.test_dict //测试某个文件中class的某个方法用例


对于API的修改，还可能需要修改测试数据或者用测用例，否则，尽管自己修改后的API可以跑出正确的结果，却因为测试用例或者数据没有修改，Jenkins还是会报错的，导致测试无法通过。

### 总结

上面提到的代码风格和Small Tests都是我在提交patch中遇到的，总结在此，除了这两点外，还有其他的测试和python的关于测试的库，还有OpenStack 中的smokestack tempest等等相关的内容。创建虚拟环境的问题还没搞定，总是报错！


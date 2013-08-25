---
layout: post
category: python
tags: python
---

OpenStack的源码就是一部学习Python语言的指导书。各种Python的技巧都在其中，把之前学习with关键字的记录帖上来。

从python2.6开始，with就成为默认关键字了。With是一个控制流语句，跟if for while try之类的是一类，with可以用来简化try finally代码，看起来比try finally更清晰。With关键字的用法如下：

    with expression as variable:
        with block

该代码快的执行过程是：
* 1.先执行expression，然后执行该表达式返回的对象实例的__enter__函数，然后将该函数的返回值赋给as后面的变量。(注意，是将__enter__函数的返回值赋给变量)
* 2.然后执行with block代码块，不论成功，错误，异常，在with block执行结束后，会执行第一步中的实例的__exit__函数。)

下面看一个具体的实例。

    with open("decorator.py") as file:
        print file.readlines()

    file = open("with.py")
    print file.__enter__()
    print file.__exit__(None,None,None)

    class WithTest():
        def __init__(self,name):
            self.name = name
            pass

        def __enter__(self):
            print "This is enter function"
            return self 

        def __exit__(self,e_t,e_v,t_b):
            print "Now, you are exit"

        def playNow(self):
            print "Now, I am playing"

            
    print "**********"
    with WithTest("coolboy") as test:
        print type(test)
        test.playNow() 
        print test.name

    print "**********"

上述代码运行的结果如下：

    <open file 'with.py', mode 'r' at 0xb7710f40>
    None
    This is enter function
    **********
    <type 'instance'>
    Now, I am playing
    None
    coolboy
    **********
    Now, you are exit


分析以上代码：
*  一二行，执行open函数，该函数返回一个文件对象的实例，然后执行了该实例的__enter__函数，该函数返回此实例本身，最后赋值给file变量。从456句可以印证。

*  自定义的类WithTest，重载了__enter__和__exit__函数，就可以实现with这样的语法了，注意在__enter__函数中，返回了self，在__exit__函数中，可以通过__exit__的返回值来指示with-block部分发生的异常是否需要reraise，如果返回false，则会reraise with block异常，如果返回ture，则就像什么也没发生。

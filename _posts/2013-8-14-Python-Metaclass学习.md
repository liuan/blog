---
layout: post
category: python
tags: python
---

元类是可以让你定义某些类是如何被创建的，从根本上来说，赋予你如何创建类的控制权，元类也是一个类，是一个type类，在执行类定义时，解释器必须要知道这个类的正确元类，如果此属性没有定义，它会向上查找父类中的__metaclass__属性，如果没有找到，则在全局变量中查找。

在传统类中，它们的元类一般是type.ClassType

元类也有构造器，传递三个参数：类名，元组形式的基类序列和类方法属性字典。

基于上面的这些理论，可以简单的模拟，通过type来创建类这个对象。

<img src="/assets/img/openstack_python_metaclass01.png" width="700px">

对上面的代码分析，通过type函数，我们构造了Hello这个类对象，然后实例化了一个hello实例。

下面这个例子定义一个元类，要求子类必须实现提供一个__str__方法。

<img src="/assets/img/openstack_python_metaclass02.png" width="700px">

在命令行运行该文件，则出现如下提示：

<img src="/assets/img/openstack_python_metaclass03.png" width="700px">

若修改成这样，运行如下：

<img src="/assets/img/openstack_python_metaclass04.png" width="700px">
<img src="/assets/img/openstack_python_metaclass05.png" width="700px">

详解其中的metaclass __new__函数的参数，如下：

<img src="/assets/img/openstack_python_metaclass06.png" width="700px">
<img src="/assets/img/openstack_python_metaclass07.png" width="700px">
<img src="/assets/img/openstack_python_metaclass08.png" width="700px">

上面的例子详细的描述了metaclass的作用，用来定义类的，任何的类的创建都需要经过metaclass，可以使用继承自type的metaclass来创建属于自己的元类。
下面结合decorator再次演示metaclass。

<img src="/assets/img/openstack_python_metaclass09.png" width="700px">
<img src="/assets/img/openstack_python_metaclass10.png" width="700px">

程序执行的结果如下：

<img src="/assets/img/openstack_python_metaclass11.png" width="700px">

分析此程序：

> 1. 装饰器，简单介绍，在函数定义之后，就会执行装饰器函数，装饰器函数返回其内部定义的函数，就是对函数就行简单的处理，上面的代码说明了，只要加了@periodic_task装饰，这个函数就添加了一个_periodic_task属性，在nova中就是可以周期执行的一个task。
> 2.在元类中的__init__函数中，存在一个cls，由后面的代码分析，可以，cls代表了这个类，Manager，对比其他的函数有一个self，该self则代表了这个类的实例。所以说_periodic_tasks是一个类属性。
> 3.在getattr代码处，可以知道有_periodic_task属性的时tellMe函数，一个函数也可以用getattr函数来判断其属性，充分说明了，函数也是对象，印证了python中的一切皆对象。
> 4.在Manager的tellMe函数中，其中访问self._periodic_tasks，又其__init__函数可以，并没有赋值self._periodic_tasks变量，故可以self._periodic_tasks是类属性。由后面直接使用Manager._periodic_tasks.append来操作，再调用tellMe打印的结果可以，前面的猜想正确，self._periodic_tasks访问的是类属性。故，可以，在python中，没有使用self.xxx赋值的变量都不是实例属性。

补充：理解metaclass，首先要知道python中一切都是对象，一切都有类型，如，一个实例的类型是该类，该类的类型是type。例如：a = 4，a是int的实例，a的类型是int；int是metaclass type的实例，int的类型是type。因此创建一个类是，python解释器都会调用元类来生成它。

参考文档（一）：[http://www.cnblogs.com/coderzh/archive/2008/12/07/1349735.html](http://www.cnblogs.com/coderzh/archive/2008/12/07/1349735.html)

参考文档（二）：[http://developer.51cto.com/art/201003/188766.html](http://developer.51cto.com/art/201003/188766.html)

参考文档（三）：[http://blog.chinaunix.net/uid-22400280-id-3389418.html](http://blog.chinaunix.net/uid-22400280-id-3389418.html)

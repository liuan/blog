---
layout: post
category: OpenStack
tags: OpenStack
---

### Extension Load

APIRouter Class的第一件事，并是创建ExtensionManager,然后用它来加载各种extension。

<img src="/assets/img/openstack_novaapi_extensions01.png" width="700px"/>

上面描述了ExtensionManager的类继承关系和部分功能，其中关键的函数_load_extensions会调用load_standard_extensions方法，该方法并遍历`contrib目录`，该目录便是存放所有extension的地方，下面举了以Keyparis为例，load_standard_extensions函数将会做哪些工作。

<img src="/assets/img/openstack_novaapi_extensions02.png" width="700px"/>

最终load_standard_extensions将contrib目录下的所有extension注册到ExtensionManager中，并且是使用的extension的alias(http request中使用到，必须保证它的唯一性，则样就可以根据http请求，知道是查找哪个extension的controller),具体的代码形式是：self.extensions[alias]=ext。上面这张图也告诉我们如果要实现自己定义的extension就必须继承ExtensionDescriptor类，然后定义四个变量name,alias，namespace,updated，并且按情况是决定是否需要冲在get_resource和get_controller_extensions函数，如果需要定义新的Restful资源，这我们需要实现get_resource函数，如果要扩展一个存在的Restful资源的controller,我们需要实现get_controller_exntension函数。例如，Keypairs重新定义了新的keypairs资源，也扩展了servers的controller，所以重新实现了这两个函数。

第二步是定义核心资源，然后使用routes的Mapper.resource建立路由关联的关系。mapper.resource的就像前面文中提到的Rails routes中的用法。

<img src="/assets/img/openstack_novaapi_extensions03.png" width="700px"/>

看到这张图，就明白在浏览器中看到的url形式了吧。还有注意，其中servers.create_resource就是将该类资源封装成WSGI APP,并且self.resources是一个字典类型的变量，就建立了一个key为servers，value为WSGI APP的记录。

<img src="/assets/img/openstack_novaapi_extensions04.png" width="700px"/>

上面这张图，就说明了controller是怎样定义的，都有什么功能，我们上面文章提到，controller有几种类型的成员函数，有两种使用了@wsgi.action或者@wsgi.extends装饰器装饰了的。在controller的元类中，就会有wsgi_actions和wsgi_extends两个字典，将这些方法收集起来，key为action或者extends的名字，value为该方法，这样http请求中如果有这样的名字，就直接映射到对应的方法上了。

第三部就是对扩展资源使用mapper.resource。第一步已经将所有的扩展资源加载起来了，第三步则遍历ExtensionManager中的self.extensions字典，获取其中所有的extensions,如果该extensions定义了新的资源，则将其提取出来，并判断该资源是否继承于self.resource中的某类资源，然后将该资源封装成WSGI APP，建立路由规则。

<img src="/assets/img/openstack_novaapi_extensions05.png" width="700px"/>

第四步则处理扩展资源中扩展了核心资源的资源，将他们扩展的方法注册起来。

<img src="/assets/img/openstack_novaapi_extensions06.png" width="700px"/>


### HTTP Request

完成上面的这些工作之后，APIRouter的最后一个操作便是将包含了所有的WGSI APP的mapper传递到基类Router中去，Router类是所有wsgi request的入口类，接收到wsgi request后，并将该请求分发到具体的某个WSGI APP。

<img src="/assets/img/openstack_novaapi_extensions07.png" width="700px"/>

每个WSGI APP必需继承自Application类，然后实现__call__方法，在该__call__方法中，并根据http请求的URL,映射到controller上的对应的方法。

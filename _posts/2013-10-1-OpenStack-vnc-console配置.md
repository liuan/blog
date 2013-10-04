---
layout: post
category: OpenStack
tags: OpenStack
---

在dashboard上成功创建虚拟机之后，在虚拟机详情的页面我们会发现存在一个vnc的子页面，可是在大多数时候该页面并不能正常的显示出我们期待的结果，直觉告诉我们，肯定是vnc在哪个地方配置错误了。那么到底该如何来配置vnc了。

我们知道vnc提供了一种让我们登录到虚拟机内部去的有效方式，除此之外，还可以通过在启动虚拟机时再里面注入keypair来验证登录，或者直接知道用户名和密码ssh登录进去。然而很多时候，这些方式还是不足够好用，有实实在在的虚拟机桌面在眼前，按照普通的机器使用方式来登录才是最熟悉的。

在错误配置VNC后，很多情况如下图所示：

<img src="/assets/img/openstack_vncconsole01.png" width="700px"/>

上面的情况简单点描述就是，VNC配置错误，无法连接到VNC上来。我们可以查看nova-novncproxy的日志，看到底是什么原因导致连接失败了，可能是connection refused 111或者其他的问题，总之，就是配置失败。

### 正确的配置

在正确的配置之前，我们稍微简单的了解下VNC，[可以点击这个链接，百度百科的VNC词条](http://baike.baidu.com/view/160932.html)，总之，VNC可以将完整的窗口界面通过网络传输到另一台计算机的屏幕上。

再简单的回忆下，我们在使用qemu-kvm相关命令直接创建虚拟机时，往往需要使用`-vnc :0`这样类似的命令参数，然后就可以通过`vncview localhost:5900`来观察安装进度了。

现在大概可以猜到了，正是通过VNC，我们可以窥探到虚拟机界面。那么在OpenStack模式下，到底该如何配置了。如果仔细看过创建虚拟机之后，表示虚拟机的进程或者虚拟机的配置文件的话，你又会更加明白。

<img src="/assets/img/openstack_vncconsole02.png" width="400px"/>

上图就显示了创建一台虚拟机之后，`-vnc 10.10.42.45:0`,表明了vnc server监听的ip地址很端口号。我们知道虚拟机domain文件中也是这个配置。

好了，铺垫了这么多，总算到了该如何配置VNC了。我们需要让VNC代理获取到正确的ip地址和端口，这样就获取到虚拟机的界面了。

##### vncserver_listen

主要在nova.conf文件中，我们需要正确的配置vncserver_listen选项，该选项就是我们在上面到的ip地址。每个计算节点都需要将该选项配置成该节点的ip地址，例如：

> 在我的某台计算节点上，nova.conf配置文件中。

> my_ip=10.10.55.6 

> vncserver_listen=$my_ip

##### vncserver_proxyclient_address

第二项需要配置的是vncserver_proxyclient_address，因为当你跟踪代码的时候，你会发现，从dashboard的请求，最终会落实到这个函数的调用上，get_vnc_console,该函数返回一个字典，表明可以连接到那个ip地址和端口去获取虚拟机的界面数据。

<img src="/assets/img/openstack_vncconsole03.png" width="700px"/>

通过这份代码，我们就很明白了.

> port是从虚拟机的配置文件中读取出来的，host则是需要从配置文件中读取

> 因此，我们需要将每台计算节点的nova.conf文件中配置该选项

> vncserver_proxyclient_address=$my_ip

##### novncproxy_base_url 

前面两步已经完成大部分的配置了，因为不能直接将这些内部的地址和端口简单的提供出去，考虑安全，网络等问题，需要使用一个代理。所以nova下面不同的代理实现。最后只需要正确配置好novncproxy_base_url就行了。

> 该选项同样需要在每个计算节点上配置，都配置成一样的内容。

> novncproxy_base_url=http://controller_node_ip:6080/vnc_auto.html

> 该地址即是novncproxy监听的地址。

至此，就正确的完成VNC的配置了，可以方便的在dashboard上登录到虚拟机内部操作了。


---
layout: post
category: OpenStack
tags: OpenStack
description: 虚拟机上网的三种模式
---

这篇文章不详细，是自己开始接触OpenStack记录到文档里来的，再网上搜了下三种模式的资料，整理了大概的资料。现在还是帖到这儿，先当简单的记录。

### 三种模式


三种网络模式，bridge，nat和host-only。主要参考文档：

> 1.[http://blog.csdn.net/luckyitman/article/details/7306725](http://blog.csdn.net/luckyitman/article/details/7306725)
> 2.[http://www.cnblogs.com/xiaochaohuashengmi/archive/2011/03/15/1985084.html](http://www.cnblogs.com/xiaochaohuashengmi/archive/2011/03/15/1985084.html)

### 桥接模式

<img src="/assets/img/openstack_network_vmnet01.png" width="700px">
<img src="/assets/img/openstack_network_vmnet02.png" width="700px">

虚拟机的虚拟网卡和host的物理网卡通过virtual bridge 连接到虚拟交换机设备VMnet0上面。

### NAT模式

<img src="/assets/img/openstack_network_vmnet03.png" width="700px">
<img src="/assets/img/openstack_network_vmnet04.png" width="700px">

在这种模式下，主机上的VMVare Network Adapter VMnet8虚拟网卡直接连接到VMnet8虚拟交换机上与虚拟网卡通信。VMware Network Adepter VMnet8虚拟网卡的作用仅限于和VMnet8网段进行通信，它不给VMnet8网段提供路由功能，所以虚拟机虚拟一个NAT服务器，使虚拟网卡可以连 接到Internet。在这种情况下，我们就可以使用端口映射功能，让访问主机80端口的请求映射到虚拟机的80端口上。（难道Adapter VMnet8 就是这个虚拟网络的网关，在OpenStack单节点单网卡环境中，创建了demonetbr0网络，它就是Adapter VMnet8？是的！）

虚拟出来的网段和NAT模式虚拟网卡（Adapter VMnet8）的网段是一样的，都为192.168.111.X，包括NAT服务器的IP地址也是这个网段。在安装VMware之后同样会生成一个虚拟DHCP服务器，为NAT服务器分配IP地址。

### host-only模式

<img src="/assets/img/openstack_network_vmnet04.png" width="700px">

在Host-Only模式下，虚拟网络是一个全封闭的网络，它唯一能够访问的就是主机。其实Host-Only网络和NAT网络很相似，不同的地方就是 Host-Only网络没有NAT服务，所以虚拟网络不能连接到Internet。主机和虚拟机之间的通信是通过VMware Network Adepter VMnet1虚拟网卡来实现的。


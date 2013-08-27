---
layout: post
category: OpenStack
tags: OpenStack
description: 介绍VLAN
---

上一篇博客已经[简单介绍了网桥和交换机的基础知识](http://www.choudan.net/2013/08/28/OpenStack-Network学习%28一%29.html)。这一篇紧着这上文，介绍VLAN方面的基础只是。


### VLAN

VLAN是一种局域网设备从逻辑上划分成一个个网段，从而实现虚拟工作组的新兴数据交换技术。VLAN技术的出现，使得管理员根据实际应用需求，把同一 物理局域网内的不同用户逻辑地划分成不同的广播域，每一个VLAN都包含一组有着相同需求的计算机工作站，与物理上形成的LAN有着相同的属性。


Vlan在交换机上的实现方法，可以分为六类：

#### 基于端口的划分

这种划分VLAN的方法是根据以太网交换机的交换端口来划分的。它将VLAN交换机的物理端口和VLAN交换机内部的VPC（永久虚电路）端口分成若干组，每个组构成一个虚拟网，相当于一个独立的VLAN交换机。


这种划分的方法优点是定义VLAN成员简单，只要将所有的端口都定义为相应的VLAN组即可，缺点是如果某用户离开了原来的端口，到了一个新的交换机的某个端口，必须重新定义。

#### 基于MAC地址的划分

这种划分VLAN的方法是根据每个主机的MAC地址来划分，即对每个MAC地址的主机都配置他属于哪个组，它实现的机制就是每一块 网卡都对应唯一的MAC地址，VLAN交换机跟踪属于VLAN MAC的地址。这种方式的VLAN允许网络用户从一个物理位置移动到另一个物理位置时，自 动保留其所属VLAN的成员身份。

#### VLAN TRUNK

VLAN TRUNK（虚拟局域网中继技术）的作用让连接在不同交换机上的相同VLAN中的主机互通。

如果交换机1的VLAN1中的机器要访问交换机2的VLAN1的机器时，我们可以把两台交换机的级联端口设置为TRUNK端口，这样，交换机把数据包从级联口发出去的时候，会在数据包中做一个TAG，以使其它交换机识别该包属于哪一个VLAN，这样，其他的交换机收到这样的数据包后，只会将数据包转发到标记中的VLAN。


### 交换机上配置VLAN

####  PVID：端口vlan id

从某个端口接收到的数据包将被赋予该端口的pvid值，然后，将该数据包转发到目的地址。如果接收数据包的端口的pvid值与将转发该数据包的端口的pvid不同，那么，交换机将丢弃此数据包，不进行转发。在交换机内，不同的pvid意味着不同的vlan。如果交换机没有定义VLAN，那么，所有端口都属于一个缺省的vlan，其pvid都等于1。

####  Trunk:端口

配置某个端口为Trunk端口后，可以将交换机一个物理端口配置多个VLAN。如下图，便是H3C交换VLAN的配置。

<img src="/assets/img/openstack_network_vlan01.png" width="700px">
<img src="/assets/img/openstack_network_vlan02.png" width="700px">


交换机对tagged 数据帧和untagged的数据帧在只有PVID和有Trunk的情况下处理不同。

下图是使用Wireshark对gb07的eth1端口抓包后的分析结果：

<img src="/assets/img/openstack_network_vlan03.png" width="700px">

可见，对于eth1端口出来的包，已经打上了Vlan标记。 

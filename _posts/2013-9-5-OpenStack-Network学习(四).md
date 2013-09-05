---
layout: post
category: OpenStack
tags: OpenStack
description: 介绍Linux Bridge部署中的问题
---

这篇博客主要是当初自己在折腾Linux Bridge时记录的，主要是遇到的一些小问题，可能现在在G版问题都已经不复存在了，但是折腾的过程就是学习的过程，其中还是有一些益处。当时正是由于网络不通的问题，就学习到了iptables的工作方式，了解了一个网络包是如何被系统处理的。

### LinuxBridge

在使用LinuxBridge作为quantum的插件，实现VLAN，需要物理交换机支持VLAN。[参考VLAN的简单介绍](http://www.choudan.net/2013/08/28/OpenStack-Network学习%28二%29.html)。

在实验环境中，搭建了两个节点的Openstack平台，其中gb31作为control node，gb07作为compute node节点。创建了一个vlan，现在可以比较这两个节点上网络设别的不同，网桥的差别。

gb07的网桥情况：

<img src="/assets/img/openstack_network_linuxbridge01.png" width="700px">

gb07的route:

<img src="/assets/img/openstack_network_linuxbridge02.png" width="700px">

gb31的brctl:

<img src="/assets/img/openstack_network_linuxbridge03.png" width="700px">

gb31的route:

<img src="/assets/img/openstack_network_linuxbridge04.png" width="700px">

其中quantum上的网络情况：

<img src="/assets/img/openstack_network_linuxbridge05.png" width="700px">


为其中的vlan配置外网连接，可是NAT没有起作用。通过quantum 创建router，创建external network，然后将router与该external network连接起来，结果如下。

按道理，凡是从虚拟机10.0.0.0/24网络过来发往gb31之外的包都应该被修改成10.10.102.193的ip地址，实现所谓的NAT。然而结果没有。因为在POSTROUTING链中，有一条规则先匹配了，所以就没有接着匹配SNAT的规则了。

参考文档：[https://bugzilla.redhat.com/show_bug.cgi?id=877704](https://bugzilla.redhat.com/show_bug.cgi?id=877704)

较笨的解决办法：iptables -t nat -D quantum-l3-agent-POSTROUTING 1

下面是创建了两个vlan1 -1001 ：10.0.0.0/24 和vlan1000：192.168.0.0/24。ns开头的是dnsmq port，qr开头的是router port。

<img src="/assets/img/openstack_network_linuxbridge06.png" width="700px">

这是gb31的部分iptables nat表规则，详细分析下：包在从本机转发出去之前，会尽过nat表的POSTROUTING链，查看该链的规则，发现里面的规则链接到其他的链里面。第二条规则引用quantum-postrouting-bottom链，查看该链的内容，该链中的规则引用quantum-l3-agent-snat链，从quantum-l3-agent-snat链，可以看到，第二条规则做了SNAT操作，改变包的源地址。

<img src="/assets/img/openstack_network_linuxbridge07.png" width="700px">

<img src="/assets/img/openstack_network_linuxbridge08.png" width="700px">

对比之前的图，其中的qr-57b84556-e6是创建router并且绑定vlan1001之后生成的。

对于虚拟机需要绑定floating ip，这样其实是同一个道理，利用snat，进行ip地址转换，如下所示，在quantum-l3-agent-float-snat规则中，多了一条记录。

<img src="/assets/img/openstack_network_linuxbridge09.png" width="700px">

在创建vlan之后，属于该vlan的虚拟机在创建时需要dhcp到ip地址，整个过程是vm发送dhcp 请求，该包则路由到dnsmasq服务所属的机器上来。每个vlan都会有一个专属的dnsmasq服务程序，该服务程序监听着这个网络的dhcp地址。下面是印证过程：

<img src="/assets/img/openstack_network_linuxbridge10.png" width="700px">

使用quantum 创建了子网subnet01 10.0.0.0/24，并且该子网指定了enable_dhcp为true，然后再主机上创建一个虚拟网卡，分配ip地址为10.0.0.2，通过quantum port-show 查看其详情，该虚拟网卡用来作为dhcp使用。

<img src="/assets/img/openstack_network_linuxbridge11.png" width="700px">

通过ifconfig，可以查看到该虚拟网卡的信息，ip地址是10.0.0.2，并且mac地址和quantum port查看到mac地址不一样，说明在bridge上存在一个端口，为上面的port，该port直接与这个虚拟网卡设备相连。

<img src="/assets/img/openstack_network_linuxbridge12.png" width="700px">

然后，我们ps查看系统中存在的dnsmasq进程，存在两个进程服务于这个子网所属的网络。dhcp-range为10.0.0.0

<img src="/assets/img/openstack_network_linuxbridge14.png" width="700px">

然后我们通过netstate查看该进程监听哪个地址，正如我们预期的，dnsmasq正在监听10.0.0.2地址，即子网中用来作为dhcp用的ip地址。

<img src="/assets/img/openstack_network_linuxbridge15.png" width="700px">

在使用linux bridge情况下，猜测的图，应该是这样。eth1.1001的地址和eth1的mac地址一样。当然这都是基于自己实验情况下的猜测，更为可靠和具体的，可以参考一个日本同行的pdf，下次找到了帖出来。

<img src="/assets/img/openstack_network_linuxbridge16.png" width="700px">

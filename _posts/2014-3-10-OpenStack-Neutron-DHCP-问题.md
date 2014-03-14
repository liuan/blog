---
layout: post
category: OpenStack
tags: OpenStack
---

OpenStack在网络上的坑一个接一个，层出不穷，还是接着上面的两篇博客中描述的网络环境，OpenStack Havana + Neutron + LinuxBridge + 多个vlan。在前期测试阶段，通过dashboard创建一两台虚拟机，查看其网络情况，因为有了之前的打磨，这种情况下虚拟机获取ip地址，访问外网，通过浮点ip地址访问虚拟机都已不是个事。然而，如果尝试去反复批量的创建虚拟机，删除虚拟机，十有八九，网络又要开始让你忧伤了。一次批量创建的虚拟机中间可能大部分无法获取到ip地址，若虚拟机是多网卡的，还可能是部分网卡获取到了ip地址，部分无法获取。

### 问题描述
---------


这个情况，简单的描述是：使用neutron dhcp agent来分配ip，在反复多次的批量创建虚拟机删除虚拟机的场景下，虚拟机很大概率无法获取到ip地址。在dnsmasq的日志中，记录了这一现象的原因：

    dnsmasq-dhcp[13578]: DHCPDISCOVER(ns-ea864f58-69) 192.168.101.14 fa:16:3e:07:e2:8d no
    address available 

日志记录的原因是`no address available`。实际上该网段还有大量可用的ip地址。在尝试解决的过程中，重启虚拟机的网络或重启虚拟机都无效，但可以通过简单的`重启neutron-dhcp-agent`来解决问题。但是，当下次重复上面的操作时，问题又出现。

### 问题解决
------

google后发现社区已经在讨论这一问题了，可以看下面的帖子：

* [DHCP problem in Grizzly](https://lists.launchpad.net/openstack/msg23817.html) : 这个帖子有点长，前面的回合在定位问题，后面的回合将问题锁定在dnsmasq这个组件上，但是没有确证，然后就没有然后了。
* [DHCP Problem in Grizzly](http://www.gossamer-threads.com/lists/openstack/dev/27481) : 这个和上面的一样，但是有结果，可以直接拖到最后。时隔一个多月，大神终于找到了真正的原因。

* [Dnsmasq-disucss Possible Bug: DHCPDISCOVER no address available](http://lists.thekelleys.org.uk/pipermail/dnsmasq-discuss/2013q2/007212.html): 开始时怀疑是dnsmasq的问题，就报告到dnsmasq社区，讨论了一番，dnsmasq的开发者觉得这问题是openstack的代码导致的。但是这个帖子可以对neutron-dhcp-agent的工作方式有个深入了解。

从第三个链接，我们了解到neutron会创建dnsmasq daemon来提供dhcp服务，dnsmasq的工作方式主要是：当有新的ip地址项（可以查看/var/lib/neutron/dhcp/xxx/hosts文件）添加到hosts文件来，或者旧的ip地址项从该文件删除后，那么neutron就会像dnsmasq发送一个SIGHUP的信号给dnsmasqd，该daemon则重新读取该hosts文件，这样就可以完成client的dhcp-discover了。

而当dhcp-agent负载很重时，就是需要不停的更新hosts文件，还有ip地址的lease更新，这样就导致dhcp-agent发送给neutron-server的report延迟了，从而进一步导致neutron server认为dhcp-agent已经down了，就不会将port creation发送给dhcp-agent，最终，dnsmasq host file没有得到更新，就无法为新创建的虚拟机提供ip地址了。

根据上面的帖子，我尝试更改了neutron.conf中的`report_interval=15 agent_down_time = 30`，然后重启neutron的服务，经过大量测试，现在变的正常了。


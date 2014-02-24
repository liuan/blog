---
layout: post
category: OpenStack
tags: OpenStack
---

这几天在尝试使用cloudinit工具来完成虚拟机配置适应问题的时候，遇到了`169.254.169.254`无法访问的忧伤。折腾了一天，总算给解决了。

### 问题描述

安装的是OpenStack Havana版本，使用neutron提供网络功能，采用linuxbridge下的vlan网络模型，因为使用的是centos6.2，所以没使用ip namespace功能。

在主控节点上对metadata相关的关键配置如下：

* `/etc/nova/nova.conf`
    
    service_neutron_metadata_proxy=true

    neutron_metadata_proxy_shared_secret=neutron 

* `/etc/neutron/metadata_agent.ini`

    nova_metadata_ip = 10.122.122.1

    nova_metadata_port = 8775

    metadata_proxy_shared_secret = neutron

* `/etc/neutron/dhcp_agent.ini` 

    enable_isolated_metadata = False

    enable_metadata_network = False

在虚拟机内部执行`curl 169.254.169.254`得到`curl: (7) couldn't connect to host`错误。刚开始条件反射的怀疑是iptables的问题，去iptables查看了下，没什么不正常。然后再通过wireshark去抓包，发现了大量的arp包，询问`169.254.169.254`的mac地址。现在大概有些眉目了。 

### 问题解决

在解决上面的问题之前，先通过这几篇博客来对metadata做个了解。

* [什么是OpenStack的metadata](http://www.pubyun.com/blog/openstack/%E4%BB%80%E4%B9%88%E6%98%AFopenstack%E7%9A%84-metadata/) ,这一篇博客对metadata的前因后果做了很好的阐述 
* [Metadata在OpenStack中的使用](http://blog.csdn.net/lynn_kong/article/details/9115033), 这一篇博客描述的是grizzly版本中metadata的工作流程

现在知道了虚拟机可以通过访问`169.254.169.254`这个地址来获取一些元数据，iptables会将访问该ip地址，且端口是80的包重定向到9697地址上。

    Chain neutron-l3-agent-PREROUTING (1 references)                            
    num  target     prot opt source               destination         
    1    REDIRECT   tcp  --  0.0.0.0/0            169.254.169.254     tcp dpt:80     redir ports 9697

在我配置的环境中，却无法访问该ip地址，并且在抓包的过程中发现大量的arp包，截图如下：

<img src="/assets/img/openstack_metadata_02.png" width="700px">

我们知道，在vm机内部，通过`curl 169.254.169.254`会首先根据vm的route表来查找下一跳地址，如果route表中有匹配的记录，则根据匹配的记录来，没有则走默认的出口。一般的认为，虚拟机的route表不会有匹配`169.254.169.254`网络的记录()，所以访问`169.254.169.254`的包应该直接发到网关上去，而不是通过arp查找169的mac地址。我们查看vm的route表，如下：

<img src="/assets/img/openstack_metadata_03.png" width="700px">

存在一条这样的记录，link-local:

    link-local * 255.255.0.0 U 1002 0 0 eth0

那么link-local和`169.254.169.254`有神马关系了，查看下面的博客。

* [Link-Local address的作用](http://www.kernelchina.org/node/329)

从上面的推论中，尝试将该route规则删除掉，结果就一切ok了。

<img src="/assets/img/openstack_metadata_01.png" width="700px">

可是，当你重启网络时，这条route记录又出现了，问题又回来了，不过现在清晰了一点，一切都与这条记录有关。google之后，看到这两篇文章，

* [Metadata via the Quantum router](http://techbackground.blogspot.ie/2013/06/metadata-via-quantum-router.html)
* [Metadata via the DHCP namespace](http://techbackground.blogspot.ie/2013/06/metadata-via-dhcp-namespace.html)

可见，neutron提供了两种方式来处理metadata的获取，前者我们已经在前面给出的博客中看到了，而后者没有见着，仔细阅读第二种方式，我们知道dhcp server会通过dhcp option 121的方式为虚拟机提供静态路由。下面有一篇文章方便我们了解dhcp option 121。

* [DHCP OPTION 33 & 121特性及应用]

从上面大神的博客中，知道若要使用dhcp namespace方式，则需要在dhcp_agent.ini中配置下面这项。然后dhcp server会推送一条和`169.254.169.254/32`相关的路由。

    # The DHCP server can assist with providing metadata support on isolated 
    # networks. Setting this value to True will cause the DHCP server to append 
    # specific host routes to the DHCP request.  The metadata service will only 
    # be activated when the subnet gateway_ip is None.  The guest instance must 
    # be configured to request host routes via DHCP (Option 121). 
    enable_isolated_metadata = True

以为有希望了，结果，发现在我的配置文件中，这一项设置的是False，由此可见，跟dhcp namespace没关系，并且在虚拟机内部'/var/lib/dhclient/'下查看dhcp server提供的信息也没有与`169.254`相关的，这下更肯定与dhcp namespace没关系。再google之，发现了社区上的这个邮件讨论，大神们的讨论一步一步的指明了道路，也与前面的分析相似。

* [Grizzly: Does metadata service work when overlapping IPs is enabled](https://lists.launchpad.net/openstack/msg22983.html)

在邮件讨论的结尾，大神一致将问题聚焦到image本身来，可能是image内部存在某个文件在修改route表，所以如下:

    grep -rn "169.254" /etc/*

结果不出所料，在'/etc/sysconfig/network-scripts/ifup-eth'中找到源头:

    # Add Zeroconf route.
    if [ -z "${NOZEROCONF}" -a "${ISALIAS}" = "no" -a "${REALDEVICE}" != "lo" ]; then
            ip route add 169.254.0.0/16 dev ${REALDEVICE} metric $((1000 + $(cat /sys/class/net/${REALDEVICE}/ifindex))) scope link
    fi

一切都是这个脚本在作怪，将其注释掉之后，世界和平了。而zeroconf是啥东西了，我们看下面的链接。

* [ZEROCONF是什么](http://blog.chinaunix.net/uid-20644632-id-2172223.html)

当然，我们也找到了一种更加体面的方式，而不是简单的将那三行注释掉来解决问题。

在`/etc/sysconfig/network`文件中添加如下一行后重启网络。

    NOZEROCONF=yes

### 总结 

总算把metadata给整通了，接下的就是赶紧走上cloudinit的康庄大道。再整了快一天之后，发现了另一个问题，当明白了从问题现象到问题根源的整个过程之后，发现如果开始时就顺着博客的思路来处理应该会在很短的时间内把问题搞定，而实际上却耗费了将近一天的功夫。由此可见，效率很低呀！ 没把思路理清就盲目下手的结果将是忧伤相随更久，不应该！ 

把cloudinit整明白之后，再将之前的heat那篇半吊子文章完成。


---
layout: post
category: OpenStack
tags: OpenStack
---

新年第一篇，但愿更加成熟稳重！

在原来安装OpenStack Folsom的机器上，重新手动安装了一遍OpenStack Havana，在安装方面遇到的问题大都是OpenStack Project依赖的各种python包版本的问题，根据日志文件差不多都可以发现和解决问题。安装的环境是在CentOS 6.3 64bit 机器上进行的，每台机器两个网卡，eth0, 连接外网， eth1没有配ip地址，两个机器的eth1网卡连接到支持vlan的交换机上面。然后，配置中最为忧伤的就是剩下的网络了，网络采用的是neutron 配 linux-bridge。

两台机器的配置情况如下：

    havana-01: 控制节点+计算节点+网络节点
               eth0: 10.10.172.1
               eth1: 无ip地址
               网络服务:neutron-server, neutron-l3-agent，neutron-linuxbridge-agent
        
    havana-02: 计算节点
               eth0: 10.10.172.2
               eth1: 无ip地址
               网络服务:neutron-linuxbridge-agent

    这两台机器的eth1通过一个单独的交换机连接。

### 问题 
--------------------------

我遇到的网络问题具体描述如下：

> 当创建一个vlan网络，ip: 192.168.0.0/24之后，创建虚拟机，虚拟机很愉快的拿到了分配给它的ip地址。

> 当在创建一个external网络: 10.10.92.0/24，并将router的外部网关指到external网络，router的内部接口连接上192.168.0.0/24的网络之后，结果，很是忧伤，在havana-02计算节点的虚拟机就拿不到ip地址了，havana-01上的虚拟机一切正常。

> 在随后的检查中，发现若将router与192.168.0.0/24的连接口给删除之后，那么一切又愉快的正常了。

> 通过抓包分析，发现，在没有将router与192.168.0.0/24没有连接起来的情况下，dhcp server返回的包的src是192.168.0.2，连接之后，dhcp server返回的包的src是10.10.92.2，即external网络与router连接的网关地址。

> dhcp server监听的地址是192.168.0.0/24网段中的192.168.0.2地址，在router没和内网连接起来的情况下，dhcp server发出的包没有被修改，连接之后，src地址被修改了。 

> 还需要注意的是：havana-01节点的虚拟机总是可以愉快的得到ip地址，havana-02节点的虚拟机就略显忧伤了，只能在一种情况下获取到。

在这儿附一张网络拓扑图，就更能明白我上面说的网络情况了。

<img src="/assets/img/openstack_havana_neutron_linuxbridge_01.png">

### 分析 
--------------------------

首先看在havana-02计算节点的虚拟机问题，通过前面的分析，基本上可以知道由于dhcp server ack的包由于src地址改变了，导致虚拟机无法接收到分配的ip地址了，因此怀疑是iptables在捣鬼，并且iptables带来的泪太多了，很多时候网络不通都是iptables无情的把包给吞了。关于跟踪iptables处理包的流程可以[参考这个篇文章](http://blog.youlingman.info/debugging-iptables-with-raw-table)，通过插入两条记录来trace iptables处理包的过程，我在使用的过程中，略有区别的是，`日志在/var/log/messages中`，最后通过查看日志，发现该包在FORWARD表中被无情的drop掉了，所以虚拟机只能不停的dhcp request，而拿不到结果。

具体操作如下：

    iptables -t raw -A OUTPUT -p udp -j TRACE
    iptables -t raw -A PREROUTING -p udp -j TRACE
    modprobe ipt_LOG

然而，前面说了，在havana-01网络节点上的虚拟机就丝毫不受影响，特意查看了它的iptables，结果很为惊讶，控制节点的规则和计算节点的规则都一样的呀，按道理说，虚拟机也应该接收不到呀。在虚拟机网卡挂载的网桥上抓包，发现dhcp server返回的包的src也修改成了10.10.92.2，和计算节点的一模一样呀！在虚拟机的网卡上抓包，发现它已经接收到了dhcp server发过来的包！为什么同样的规则，却得到不同的结果了。通过同样的方式查看iptables的日志，竟然没有看到和ip地址是10.10.92.2相关的记录。

我们知道，havana-01与havana-02的差别在于havana-02是计算节点，它接收到的dhcp server的包是从外部发送过来的，而havana-01中的dhcp server的包是从该节点内部的进程发送出来的。iptables对于不同来源的包处理流程是不一样的。我们再来具体分析，到底有什么差别，找到问题在哪！

首先明白从外部接收到的包和在内部发出去的包，iptables的处理流程都是怎么样的，可以先参考我之前的一篇简单描述iptables的文章，[OpenStack Network学习(五)](http://www.choudan.net/2013/09/06/OpenStack-Network学习%28五%29.html)，将其中最为关键的一张图截取出来，可以看iptables的处理流程。

<img src="/assets/img/openstack_network_iptables02.png">

从上图和结合iptables的日志，我们知道，若包是从外部接收的，那么就是从最初的raw表的PREROUTING链开始处理，若包是从内部发出的，则由Routing Decision之后，从raw表的OUTPUT链走，所以在havana-02节点上，包是从最顶上开始，然后走filter的链进行过滤，根据iptables的规则把包给过滤掉了，而在havana-02中，包从dnsmasq进程发出，首先通过的就是raw的 OUTPUT链，然后通过最后的nat进行了转换，也就是我们在抓包中看到的包的src地址变成了10.10.92.2的情况，最后发出去，由于包已经发出去，到了brq-xxx，则就直接转发到挂载在上面的虚拟机网卡，虚拟机则接收到了dhcp server的ack包，因此获取ip地址正常。

现在问题很清晰了，是由于计算节点的iptables将dhcp的包给拦截了，因此问题就在于为什么会被拦截了？这是什么原因导致的。我们就知道主要在iptables，那么怎样去改变这个iptables了，简单的手动删除或者修改某条规则肯定是不可取的。在创建虚拟机的时候，我们知道虚拟机会属于某个默认的security group，那么可以去修改这个security group去改变iptables的规则。

### 解决
------------------

在dashboard上就可以看到，如下图，我们让dhcp server的包改成允许通过，如下所示：

<img src="/assets/img/openstack_havana_neutron_linuxbridge_03.png">

到现在，我们已经找到了问题的原因和解决办法，那么果断来检查最终结果，愉快的搞定！

### 总结
---------------------

经常安装OpenStack，遇到的最多问题便是虚拟机网络问题，总是虚拟机无法获取ip地址，无法访问外网等等，其实很多的问题都是由于受到iptables的影响，简单的将iptables关闭掉往往都能先解决问题，但还是不是最可取的办法。 

在调试的过程中，发现如果虚拟机已经分配过ip地址，然后重新启动网卡让它再次去获取ip地址和虚拟机创建时去启动网卡获取ip地址的过程貌似略有不同，首次获取ip地址，则通过dhcp request，dhcp server会返回一个dhcp offer包；在已经分配过ip地址的情况下，重新获取ip地址，若再最后获取失败，虚拟机会尝试使用之前分配的ip地址，并且通过简单的ping来测试之前的ip地址是否可用。

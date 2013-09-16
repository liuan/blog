---
layout: post
category: OpenStack
tags: OpenStack
description: 主要介绍iptables的情况
---

在学习OpenStack网络的过程中，很多时候可能会涉及到防火墙，那么iptables到底是如何工作的了。这一篇主要记录防火墙的基本知识，这样有助于分析虚拟机的包在主机中到底经过了什么样的变换，处于vlan中的虚拟机通过NAT完成与外界通信的，NAT到底是如何完成了成private ip到public ip的相互转换，熟悉了iptables那么就知道此过程。

### iptables

ptables的基本概念包括：规则，链，表。

* 规则：就是网络管理员预定义的条件，规则一般定义为“如果数据包头符合这样的条件，就这样处理这个数据包”。规则存储在内核空间的信息包过滤表中，这些规则分别定义了源地址、目的地址、传输协议（如TCP,UDP,ICMP）和服务类型(HTTP,FTP,SMTP)等，当数据包与规则匹配时，iptables就根据规则所定义的方法来处理这些数据包，如放行(accept),拒绝(reject),和丢弃(drop)等。配置防火墙主要工作就是添加，修改和删除这些规则。
* 链：链（CHAINS）是数据包传播的途径，每一条链其实就是众多规则中的一个检查清单，每一条链可以有一条或数条规则。当一个数据包到达一条链时，iptables就从链中第一条规则开始检查，看该数据包是否满足规则定义的条件。如果满足，系统就会根据这条规则所定义的方法处理该数据包；否则iptables将继续检查下一条规则，如果该数据包不符合链中任一条规则，iptables将会按照默认策略来处理数据包。
* 表：表（tables）提供了特定的功能，iptables内建了4个表，即raw表，filter表、nat表、和mangle表。分别用于实现包过滤，网络地址转换和包重构的功能。
* Filter表：Filter表主要是用于过滤数据包，是iptables的默认表，包含了INPUT链（用于处理进入的数据包），FORWORD链（处理转发的数据包），OUTPUT链（处理本地生成的数据包）。在filter表中只允许对数据包进行接受、丢弃操作，而无法对数据包进行修改。
* Nat表：Nat表主要用于网络地址转换，可以实现一对一，一对多，多对多等NAT工作，包含了PREROUTING链（修改即将到来的数据包），OUTPUT链（修改路由之前本地生成的数据包）和POSTROUTING（修改即将出去的数据包）。
* Mangle表：Mangle表主要用于对指定包进行修改，如TTL,TOS等。用的很少。

### iptables处理过程

有了上面的基础知识之外，那么对于进入主机的包或者从主机发出去的包在iptables中经过了哪些工序了？先看下面这张简单的流程图。

![iptables](/assets/img/openstack_network_iptables01.png)

1. 当数据包进入网卡时，他首先进入PREROUTING链，内核根据数据包目的IP判断是否需要转发出去。
2. 如果数据包是进入本机的，就会向下到INPUT链。数据包到达INPUT链后，任何进程都会接受。本机上运行的程序也可以发出数据包，这些数据包会经过OUTPUT链，然后到达POSTROUTING链输出。
3. 如果数据包是要转发出去的，且内核允许，数据包就会经过forward链，然后POSTROUTING链输出。))))

这儿还有一张更加具体的图可以参考。

![iptables](/assets/img/openstack_network_iptables02.png)

### 总结

 知道iptables是如何处理数据包了，再看network controller节点的iptables，我们就知道虚拟机的包在iptables中经过了怎样的变化，所谓的NAT也就是采用iptables转换实现的。所以使用vlan方式使虚拟机上网则需要以下几个过程：

 1. 使用quantum创建private network,即一个vlan，创建了vlan则需要使用可以支持vlan的交换机来实现包在交换机间的转发
 2. 创建一个public network，该网络的ip地址是原本能够上网的
 3. 使用l3-agent，创建router将public network 与 private network连接起来。
 4. 将一个public nework的ip 与 vlan中虚拟机的ip关联起来。

这样，虚拟机的ip地址最终就可以通过iptables转换成public network中的ip地址，然后使用public ip地址上网，访问public ip的数据包最终也会转换成虚拟机的ip，然后路由到虚拟机来。

对于NAT的转换，可以部分参考上一篇文章[OpenStack-Network学习四](http://www.choudan.net/2013/09/05/OpenStack-Network学习%28四%29.html)。

### 参考文档

> 参考文档（一）：[http://6509e.blog.51cto.com/542252/124312](http://6509e.blog.51cto.com/542252/124312)

> 参考文档（二）：[http://blog.chinaunix.net/uid-24960107-id-204362.html](http://blog.chinaunix.net/uid-24960107-id-204362.html)

> 参考文档（三）：[http://www.ibm.com/developerworks/cn/cloud/library/cl-openstack-network/](http://www.ibm.com/developerworks/cn/cloud/library/cl-openstack-network/)

> 参考文档（四）：[http://blog.youlingman.info/debugging-iptables-with-raw-table/](http://blog.youlingman.info/debugging-iptables-with-raw-table/)

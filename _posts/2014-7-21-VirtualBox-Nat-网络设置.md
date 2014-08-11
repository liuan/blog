---
layout: post
category: OpenStack
tags: OpenStack
---

在windows下使用VirtualBox来运行虚拟机，十分方便，比Vmvare轻多了。

现在需要创建两台VirtualBox的CentOS 64虚拟机，网络需要如下配置：

* 虚拟机需要能够访问外网 
* 虚拟机之间能够互相访问 
* 主机能够访问虚拟机某些服务，例如ssh登录，访问http服务

根据OpenStack的经验，这些创建一个网络，通过NAT，就可以搞定，具体到VirtualBox的网络配置，需要注意：

1. 虚拟机需要创建两个网卡，第一个网卡选用NAT方式(上外网），第二网卡选用HostOnly方式（虚拟机互联）
2. NAT方式的网卡需要配置端口转发，在OpenStack里即iptables的NAT规则（主机访问虚拟机某些端口)
3. 需要给选用NAT方式的网卡配置静态IP，不然两个VM通过DHCP得到的IP都一样，具体原因可以google
4. HostOnly的网卡，不要设置网关，不然两个网卡的网关会覆盖掉另一个，route表中，只能有一个default规则，除非你清楚希望走哪个网卡出去 

这样，虚拟机就可以上外网了，虚拟机之间可以互联了，主机也能够ssh上了。

还存在一点不足的是，如果自己的电脑不关机(即不重启虚拟机，VirtualBox)，换了一个网络，虚拟机的网络就断了。估计主要是virtualbox没有定期的更新NAT的规则，因为电脑的IP地址变化了，规则可能还是映射老的IP地址，所以换了个网络，如果需要虚拟机连外网，就重启VirtualBox。





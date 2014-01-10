---
layout: post
category: OpenStack
tags: OpenStack
---

OpenStack的网络对于一个新手来说，绝对是一个大坑，何况现在更加复杂的网络组件neutron。在从Folsom版本使用quantum到现在使用Havana的neutron的血泪史中，网络的表现极其不稳定，所有的网络不稳定都体现在iptables总是被动态刷新的过程中。在前面的一篇介绍自己使用[quantum的网络](http://www.choudan.net/2013/09/05/OpenStack-Network%E5%AD%A6%E4%B9%A0%28%E5%9B%9B%29.html)博客中，我发现有条防火墙规则将流向虚拟机的流量给吞噬了，删掉之后就没任何问题了。现在，因为使用的neutron，以为在新版本中，会遇到的问题会更少，结果是图样图深破。

在配置的过程中，预期的结果是虚拟机能够正常获取ip地址 ，通过floating-ip实现内外双向访问。但实际操作的结果是获取ip地址只要处理好security group就行了，而访问外网这个问题还是需要删除一条防火墙规则和路由表规则。在实验的期间，曾短暂的出现过不需要对防火墙做任何处理的极愉快画面，可是没隔多久防火墙规则刷新变化了，又回到需要手动处理防火墙的情况了。

### 实验
---------------------

在部署之前可以参考官方贴出的网络文档，详细的描述了几种网络模型下网络的配置图，很具有参考价值；第二个链接是国际友人的实战经验，不过是在Folsom Quantum版本；第三个链接是IBM出品的介绍Linux上的基础网络设备，对于理解OpenStack网络模型的运转方式有极好的帮助，并且IBM的文章大都深入浅出；第四个链接是我的上一篇，主要着力于跟踪网络流量在iptables中的处理过程，较抽象，但对于跟踪问题还是很实用。

* [OpenStack Admin Guide Cloud Networking](http://docs.openstack.org/admin-guide-cloud/content/ch_networking.html)
* [QuickStart with RHOS(Red Hat OpenStack) Folsom Preview](http://d.hatena.ne.jp/enakai00/20121118/1353226066)
* [Linux上的基础网络详解](http://www.ibm.com/developerworks/cn/linux/1310_xiawc_networkdevice/index.html)
* [OpenStack Neutron LinuxBridge 网络配置](http://www.choudan.net/2014/01/02/OpenStack-Neutron-LinuxBridge-%E7%BD%91%E7%BB%9C%E9%85%8D%E7%BD%AE%E9%97%AE%E9%A2%98.html)

在有了上面这些准备之后，再来配置自己的环境，头脑中更加清晰一些。

我搭建的环境大致是这样的：

<img src="/assets/img/openstack-neutron-linuxbridge-11.png" width="600px">

在os01到os03的三台服务器上，每台服务器都是安装的CentOS6.2，其中每台机器的eth1都链接到一个支持vlan的交换机上，节点配置的组件如下：

* os01: keystone, nova, glance, cinder, horizon, neutron-server, neutron-l3-agent, neutron-metadata-agent, neutron-linuxbridge-plugin
* os02: nova-compute, neutron-linuxbridge-plugin
* os03: nova-compute, neutron-linuxbridge-plugin

除了neutron之外的其他组件配置，大家可以参考其他的文章。重点来看neutron的配置，os01节点，是主控的网络节点，最终配置正常的结果是网络组件应该是下面这张图的模样，计算节点os02, os03则是后面那张图的模样。 

<img src="/assets/img/openstack-neutron-linuxbridge-13.png" width="600px">

<img src="/assets/img/openstack-neutron-linuxbridge-12.png" width="600px">

#### Nova

首先，我们需要配置Nova，使nova使用neutron来提供网络服务，根据官方文档，在nova.conf中的配置如下：

    network_api_class=nova.network.neutronv2.api.API
    neutron_url=http://10.10.0.1:9696
    neutron_auth_strategy=keystone
    neutron_admin_tenant_name=service
    neutron_admin_username=neutron
    neutron_admin_password=password
    neutron_admin_auth_url=http://10.10.0.1:35357/v2.0

    security_group_api=neutron
    firewall_driver=nova.virt.firewall.NoopFirewallDriver

    service_neutron_metadata_proxy=true
    neutron_metadata_proxy_shared_secret=foo 

#### Neutron

好，根据官方文档，我们使用vlan来隔离租户的网络，那么将连接public network的physical network设置成physnet1，而内部的数据网络设置成physnet2，这些设置都在linuxbridge_conf.ini文件中配置，与官方文档稍有不同的是多了配置physnet1，不然创建public network是会出错。

    tenant_network_type = vlan
    network_vlan_ranges = physnet2:1000:2999, physnet1
    physical_interface_mappings = physnet1:eth0,physnet2:eth1

neutron.conf本身的配置不多，都是常见普通的配置，和其他的project一样。

现在可以通过neutron来创建路由器了，为后面配置l3-agent准备。

    $ tenant=$(keystone tenant-list | awk '/admin/ {print $2}')
    $ neutron router-create router01
    $ neutron net-create --tenant-id $tenant public01 \
    --provider:network_type flat \
    --provider:physical_network physnet1 \
    --router:external=True
    $ neutron subnet-create --tenant-id $tenant --name public01_subnet01 \
    --gateway 10.10.1.254 public01 10.10.1.0/24 --disable-dhcp
    $ neutron router-gateway-set router01 public01

因为centos6.2没有支持namespace，所以还需要再配置一下l3-agent，在l3_agent.ini中：

    interface_driver = neutron.agent.linux.interface.BridgeInterfaceDriver
    use_namespaces = False
    router_id = 335d9173-50d1-4f2d-af23-3a69b79c8991
    gateway_external_network_id = 322872c1-615f-4c79-83dd-075ffaaa7e1f

对于上面的配置选项的意义，可以参考官方文档。至此，我们的配置差不多了。可以愉快的将各个节点上的服务起来了。然后创建private network。 

    $ neutron net-create --tenant-id $tenant net01 \
    --provider:network_type vlan \
    --provider:physical_network physnet2 \
    --provider:segmentation_id 1000
    $ neutron subnet-create --tenant-id $tenant --name net01_subnet01 net01 192.168.101.0/24
    $ neutron router-interface-add router01 net01_subnet01

### 实验
-----------------------------------

现在可以去创建虚拟机了，创建两台，这样每个计算节点上都存在一台了，看看他们的网络情况是否和上面图中描述的一样。如果创建的虚拟机不能获取ip地址，记得更改security group中的规则。还需要配置eth1链接的交换机，让带有1000vlan tag的包顺利通过它的端口。

OK！如果一切正常的话，现在虚拟机是可以愉快的获取到ip地址的，但是忧伤会很快就来了。发现os01的网络断了，查看route表，结果是下面的情况（注意，这是我创建了两个网络之后的情况）：

    Kernel IP routing table
    Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
    192.168.101.0   *               255.255.255.0   U     0      0        0 ns-ea864f58-69
    192.168.101.0   *               255.255.255.0   U     0      0        0 qr-1be5fee8-08
    192.168.102.0   *               255.255.255.0   U     0      0        0 ns-6c1be5d8-02
    10.10.92.0      *               255.255.255.0   U     0      0        0 qg-455574b8-d7
    10.10.0.0       *               255.255.0.0     U     0      0        0 brq322872c1-61
    link-local      *               255.255.0.0     U     1003   0        0 eth1
    default         10.10.92.254    0.0.0.0         UG    96     0        0 qg-455574b8-d7
    default         10.10.0.254     0.0.0.0         UG    96     0        0 brq322872c1-61

默认的出口成了`qg-455574b8-d7`这个口了，根据官方帖的图，应该是没问题的吧! 现在删掉这条route规则，一切都OK了。

    route del -net default gw 10.10.92.254
    
在dashboard上绑定浮点ip地址，此时此刻，在虚拟机内部ping外网还是没法成功，通过查看iptables的处理过程，发现包都会一样规则给处理了。

    Chain POSTROUTING (policy ACCEPT)
    num  target     prot opt source               destination         
    1    neutron-l3-agent-POSTROUTING  all  --  0.0.0.0/0            0.0.0.0/0               
    2    neutron-postrouting-bottom  all  --  0.0.0.0/0            0.0.0.0/0        
    3    neutron-linuxbri-POSTROUTING  all  --  0.0.0.0/0            0.0.0.0/0      
    4    nova-api-POSTROUTING  all  --  0.0.0.0/0            0.0.0.0/0              
    5    nova-postrouting-bottom  all  --  0.0.0.0/0            0.0.0.0/0)

    Chain neutron-l3-agent-POSTROUTING (1 references)                           
    num  target     prot opt source               destination
    1    ACCEPT     all  --  0.0.0.0/0            0.0.0.0/0           ! ctstate     DNAT)

很奇怪，为什么neutron-l3-agent-POSTROUTING会出现在POSTROUTING链中第一条，此条规则会将包给接收，就没法完成DNAT，这样包最后就没法让虚拟机接收到。在偶然的一次短暂成功中，保存当时的iptables状态，`neutron-l3-agent-POSTROUTING` 而是处于第三条的位置，如下：

    Chain POSTROUTING (policy ACCEPT)
    num  target     prot opt source               destination         
    1    neutron-linuxbri-POSTROUTING  all  --  0.0.0.0/0            0.0.0.0/0      
    2    neutron-postrouting-bottom  all  --  0.0.0.0/0            0.0.0.0/0               
    3    neutron-l3-agent-POSTROUTING  all  --  0.0.0.0/0            0.0.0.0/0               
    4    nova-api-POSTROUTING  all  --  0.0.0.0/0            0.0.0.0/0              
    5    nova-network-POSTROUTING  all  --  0.0.0.0/0            0.0.0.0/0          
    6    nova-postrouting-bottom  all  --  0.0.0.0/0            0.0.0.0/0)

注意，其中的nova-network-POSTROUTING是因为最开始的时候使用了nova-network，据官方文档中介绍，如果之前使用过nova-network，最好将nova-network给删除掉，避免影响neutron，可以尝试执行`iptables -F`来清空防火墙，如果执行之后，发现防火墙里还有残余的信息，最后查看`/etc/sysconfig/iptables`文件，里面可能写了很多规则，删除和nova-network相关的。

对比上面的两种情况，我们知道ping外网的包，在出了本机之后，源IP地址都会被修改成public network ip地址，所以能够出去，响应的能回来，但是当响应回来之后，需要将目的ip地址修改成虚拟机的ip地址，否则虚拟机接收不到。对于失败情况下的防火墙规则，把包给提前接收了，还没来得及由后面的规则修改目的ip地址，而成功情况的防火墙规则是第一条规则就修改了目的ip地址。可惜，在实验的情况，iptables会定时刷新，然后`neutron-l3-agent-POSTROUTING`还是跳到第一的位置来了。 

目前我的处理方式是执行`iptables -t nat -D neutron-l3-agent-POSTROUTING 1`，删掉那条提前接收包的规则，现在世界清静了，网络畅通无阻。

### 总结 
--------------------

在Folosm版本使用quantum的时候，也是这个问题，需要定时的去删除这条规则，才能把网络搞定，现在在havana的neutron模式下，开始尝试突破这个问题，结果还是一样。深深感觉，是在配置上错过了哪个细节，不应该是neutron的一个bug吧，但是没有找出来。还是需要花一点时间来深入理解neutron，了解neutron设置iptables的基本原理。 

除了iptables方面的问题，在观察路由表时，多出的那条default规则同样很奇怪，按照官方的图网络流量应该是很愉快的可以通过`qg-455574b8-d7`最终从eth0口出去，但是情况却不是这样的，并且通过wireshark在各个网卡上抓不到从os01主机往外ping 8.8.8.8的包，很奇怪，结果在iptables的log中看到这个ping的包都走lo设备了，还需要再摸索下阿！要搞懂这一块，还多坑！ 

---
layout: post
category: OpenStack
tags: OpenStack
---

OpenStack的服务特别多，并且不同的主机根据部署规划的考虑会运行不同的服务，在安装调试或者VI环境下开发时，可能需要频繁的启动某个服务或者某组服务，这样就导致需要在shell下敲不少命令，今天特意写了一个脚本，方便对OpenStack服务的start，stop，restart和status操作。

脚本大致想处理好下面几个问题：

1. 可能在某时刻需要按照不同的粒度来启动服务，譬如需要重启所有nova的服务，譬如只需要重启nova-api服务，还可能重启所有OpenStack服务
2. 不同主机上需要安装的服务不一样，所以需要能够定制化该主机上的服务
3. 对服务的启动，可以通过service nova-api start这种方式，也能通过service openstack-nova-api start这中方式，还可能就是直接通过nova-api启动
4. 对于某些服务，还需要指定其配置文件，例如neutron-server --config-file /etc/neutron/api-paste.ini 等等配置文件

### 功能
************

有了上面的几个想法之后，就完成了脚本的第一版本，大概功能如下：

    
* 首先会在home目录创建一个.openstack_services.ini配置文件，该文件中会写入一些基本的内容，安装project来分组，每个project组下面都会有一些服务，参考下面的例子.

在home目录下自动生成的配置文件格式，这仅是一个参考：

    [cinder]
    cinder-api
    cinder-volume
    cinder-scheduler
    [nova]
    nova-api
    nova-cert
    nova-compute
    nova-conductor
    nova-scheduler
    [neutron]
    neutron-server=--config-file /etc/neutron/api-paste.ini --config-file /etc/neutron/neutron.conf



`注意最后一行，考虑到某些服务安装之后，不一定会在/etc/init.d/目录下创建可以直接通过service命令启动的脚本，并且这些服务还可能需要传指定的参数，所以就在后面添加了服务启动需要的参数，目前该脚本只能处理好这种格式。`


* 脚本提供了四个基本的方法来对此配置文件进行修改，以适应在安装不同服务的机器上方便运行。

* 脚本可以方便的对服务进行四个基本操作，例如nova restart，则重启所有nova的服务，nova-api restart则重启nova-api服务, all restart 则重启所有的服务

* 脚本还提供了一个简单的帮助信息， 可以直接运行脚本不指定任何参数或者-h,--help看到，还可以方便读取配置文件的内容

### 示例
************

    choudan@ubuntu:~/openstack-x.sh nova start
    choudan@ubuntu:~/openstack-x.sh nova-api start
    choudan@ubuntu:~/openstack-x.sh --add-service nova nova-conductor
    choudan@ubuntu:~/openstack-x.sh --add-service neutron neutron-dhcp-agent --config-file /etc/neutron/dhcp_agent.ini --config-file /etc/neutron/neutron.conf
    choudan@ubuntu:~/openstack-x.sh --del-service nova-conductor
    choudan@ubuntu:~/openstack-x.sh --del-project neutron

### 脚本
******************

可以在[github: openstack-x.sh](https://github.com/liuan/openstack-x/blob/master/openstack-x.sh)上看到这个脚本，目前自己在centos的系统上使用正常。

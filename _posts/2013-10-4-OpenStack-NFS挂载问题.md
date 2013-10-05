---
layout: post
category: OpenStack
tags: OpenStack
---

很早之前在做OpenStack虚拟机迁移实验时遇到了使用NFS作为共享存储创建虚拟机失败的问题，今天翻看自己记录的文档时，看到了这个问题，估计有很大的共性，就帖上来。

具体的情况是这样的：

> 在一台机器上搭建了nfs，然后在其他的计算节点mount了nfs共享的目录，用来作为虚拟机存储，然而在创建虚拟机失败了，从nova-compute的日志中可以看到错误的问题所在，libvirtError: internal error Process exited while reading console log output: chardev: opening backent "file" failed.

<img src="/assets/img/openstack_storagenfs01.png" width="770px" />

若将nfs存储改为本地磁盘存储，就可以创建成功，大概问题就定位在nfs上了。在搜索了解之后，发现一切根源都是挂在nfs之后，缺乏权限所致。具体的原因是因为，挂在nfs之后，映射到本地的目录的权限都变成了nobody，不管在/etc/exports如何配置，映射过来都是nobody的权限。

多次搜索之后，原来是因为在rhel6.2上默认安装的是nfs-v4版本，而v4中的/etc/idmapd.conf默认使用nis，没有nis它自动会将目录权限映射成nobody用户。所以不管在exports里写什么权限，最终都会成nobody用户。

解决的办法很简单：

> 使用v3版本挂载，可以参考下面的命令
> mount -t nfs -o vers=3 10.10.22.7:/export/instances /var/lib/nova/instances

在nfs的配置文件/etc/exports中，例如下图：

<img src="/assets/img/openstack_storagenfs02.png" width="300px" />

其中第三项，\*_squash的作用是当客户端以某种身份来访问这个共享目录，对于其身份做一定的处理，no_root_squash,即以root身份访问，root_squash，即压缩root用户的访问权限，使其成nobody用户。其他两个选项类似。

---
layout: post
category: OpenStack
tags: OpenStack
---

在这个帖子中将把所有遇到的dashboard安装问题记录在此。

### 1
*************

问题描述：

> OS是CentOS6.2，该系统之前安装过OpenStack Folsom版本。现在简单的卸载了OpenStack的主要Project文件，然后使用PackStack一键安装，安装显示成功。然后登录dashboard，但是出现错误，具体截图如下，大致意思是：报错，请刷新。

<img src="/assets/img/openstack_dashboard_error_01.png" width="300px" height="200px"/>

然后在`/var/log/httpd/error.log`文件中查看，只出现如下的错误提示：

    [Mon Nov 25 03:19:19 2013] [error] DeprecationWarning: Translations in the project 
    directory aren't supported anymore. Use the LOCALE_PATHS setting instead.
    DeprecationWarning: Authentication backends without a `supports_inactive_user` 
    attribute are deprecated. Please define it in 
    <class 'openstack_auth.backend.KeystoneBackend'>. 
    [Mon Nov 25 11:19:19 2013] [notice] caught SIGTERM, shutting down ]

从第一行来看，主要是一些东西过时，但还不至于导致出现第二行出现的shutting down错误。这些信息还不足以作出判断，我们需要更多的调试信息才能解决问题。

解决办法：

1. 将openstack改到调试模式，在`/etc/openstack-dashboard/local-seetings文件中，将DEBUG = False 改为DEBUG = True`，然后重启apache。
2. 再次登录到dashboard界面，现在出现了更全面的调试信息，截图如下，主要是`User Object has no attribute 'service_region'`,这个问题主要是系统中`django-openstack-auth`包的版本太低。

<img src="/assets/img/openstack_dashboard_error_02.png" width="700px"/>

查看系统中django-openstack-auth的版本：

    [root@choudan site-packages]# pip-python freeze | grep django
    django-appconf==0.5
    django-compressor==1.2
    django-openstack-auth==1.0.4 

3. 我们卸载掉django-openstack-auth包，然后重新安装
4. 再次登录dashboard，登录成功，至此解决问题，新安装的django-openstack-auth包为1.1.3版本

### 总结

话说有了PackStack以后，OpenStack一直让人诟病的安装问题一下子突进一大步，再也没那么复杂，对于新系统，估计可以一键之后，就立马成功了，已经被折腾过的系统，再使用PackStack安装，会存在很多包依赖错误的问题。

后续若还遇到与Dashboard相关的问题，都汇总到此处来，方便查阅。

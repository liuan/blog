---
layout: post
category: OpenStack
tags: OpenStack 
---

### OpenStack开发者入门
 

更新： 突然发现网上还是不少介绍如何向OpenStack贡献代码的文章,例如，[这篇文章,IBMer写的，强烈推荐作为参考](http://www.ibm.com/developerworks/cn/cloud/library/cl-contributecode-openstack/)


开发OpenStack有一段时间了，由于一直比较忙没有认真查看如何加入到开源社区参与开发,最近深受程辉大哥在[UnitedStack](http://www.ustack.com/unitedstack-need-opensource-engineer)上文章得影响，决定先摸清如何参与开源社区开发，感受开源社区分布式协作开发和巨神们coding的魅力，故特意在网上查了资料，直接指导的很少，还得的靠[官网上的文档](http://www.openstack.org/assets/welcome-guide/OpenStackWelcomeGuide.pdf)。当然，还有一篇Rackspace的Michael Still大牛对这个问题做了统一回答。[Getting Started With Openstack Development](http://www.stillhq.com/openstack/20130416-summit.pdf)。

1.  加入的前提是需要有一个Launchpad.net的账号，注册就OK了。
2.  作为一个个人开发者，需要签署一份Individual Contributor License Agreement （CLA）协议，选择同意即可。

有了上面两项之后，就需要了解openstack管理源码的工具-git和用来code review的工具 gerrit。


##### Git

Git在网上有很多介绍资料！

##### Gerrit


Gerrit是代码审核服务器，向git仓库推送的代码必须经过Gerrit服务器，然后需要经过全面的测试和审核才可以合并到版本库中对应的分支。了解Gerrit的工作原理，[可以点这](http://www.worldhello.net/2010/11/10/2059.html)。从这个链接借过来一张Android代码审核得流程图，因为这张图特别详细，可以从中清楚无比得了解到Gerrit的整个工作过程。

<img src="/assets/img/gerrit-workflow.png" atl="Gerrit Workflow for android" width="700px"/>

###### Openstack Gerrit Contribution Workflow


OpenStack大体上和上面一样，过程更简洁，中间还使用拉Jenkins来完成自动化测试。

<img src="/assets/img/openstack_gerrit_Contribution_path.png" alt="OpenStack Gerrit Workflow" width="700px"/>

了解到上面这些基本的信息之后，还需要进行下面得一系列配置，才可以参与到此过程来。

3. 设置review账号，review账号是gerrit review system用来确定身份的，可以直接使用launchpad.net的账号。访问review.openstack.org网页，在右上角有sign in链接，点击后直接用launchpad.net的账号登陆进去

4. 上传ssh public key，登陆进去之后在右上角有settings，点击登陆进去，在左侧有SSH Public Key，然后再自己的电脑上生成ssh public key，生成的步骤可以参考[github help](https://help.github.com/articles/generating-ssh-keys), 注意其中的passphrase，在后面的git review -s时将会用到。然后将id_rsa.pub中key添加到网站上

5.  配置git config --global  user.name user.email,这可以参照openstack gerrit workflow中的描述。

6. git clone 代码。

至此，整个本地的git仓库就建立起来了，为加入到开源社区开发铺平了道路。


#### Fix Bug


个人感觉，新手入门，应该都是从修复Bug开始，之前在捣鼓OpenStack的过程中，遇到几个小Bug，自己本地fix掉，但还是没了解到社区fix bug得严谨过程。下面就尝试介绍下整个流程，如果有更加理解整个过程的，欢迎补充或者修正我描述中得错误。

此部分稍候补上。
[OpenStack开发者入门(二)](http://www.choudan.net/2013/08/08/OpenStack开发者入门%28二%29.html)

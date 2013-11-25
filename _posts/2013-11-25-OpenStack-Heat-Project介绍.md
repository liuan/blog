---
layout: post
category: OpenStack
tags: OpenStack
---

之前一直在看云环境下应用自动化部署方面的论文，在虚拟化环境下已有VMVare联合其他大产商推出的应用打包发布标准格式OVF，目前应该是已经发布了2.0版本，VMVare的vSphere和IBM的虚拟化产品对此都有一系列的支持。OVF解决了应用在虚拟化平台自动部署的问题，可是在面对现在的云平台环境下，还是有些不足，需要将OVF格式更进一步推进。Amazon对复杂应用系统的部署给出了自己的解决方案AWS CloudFormation，目前OpenStack社区也给出了自己对应的项目Heat。

花了一点时间从网上收集了下Heat资料，整理在下，方面大家了解，其中从部分内容是从网友的博客中摘得，还有部分内容是从OpenStack官方的文档中翻译过来，最末统一给出所参考的资料链接。

### 背景
*******

##### AWS CloudFormation

Amazon 在云计算早期，提供了Amazon Machine Images即AMI的一站式展示，这些AMI都是单个虚拟机镜像模板，通过使用Amazon的EC2服务，可以依赖于这些镜像模板迅速的创建出单一服务器上的应用。当然，创建出来的单一实例还需要用户手动去设置，完成实现服务可用的最后一步。

直接的使用AMI的方式，在面对复杂的集群应用面前就显得更加无力。Amazon为了解决此问题，推出了Amazon CloudFormation Templates。CloudFormation Templates 专门为大规模集群应用而生，通过使用这样的模板，来简化AWS资源的准备和部署，解决了传统方式上需要手动管理各项资源之间依赖的问题，实现了在EC2中迅速创建大量实例部署集群应用。

CloudFormation的推出，让Amazon更好的拥抱了企业IT领域。

##### OpenStack Heat

OpenStack Heat 是由RedHat公司在Grizzly版放出的孵化项目。Heat项目直接对应于AWS的CloudFormation，在目前来看，还只是CloudFormation功能的一个子集。Heat可以对Amazon CloudFormation Templates进行配置，并运行在任何一个OpenStack生态环境上。

对于Heat的认识，我们可以直接从CloudFormation入手，Amazon在其官网上给出的CloudFormation介绍如下:

> “AWS CloudFormation 向开发人员和系统管理员提供了一种简便地创建和管理一批相关的 AWS 资源的方法，并通过有序且可预测的方式对其进行资源配置和更新。您可以使用AWS CloudFormation 的示例模板或自己创建模板来介绍 AWS 资源以及应用程序运行时所需的任何相关依赖项或运行时参数。您可以不需要了解 AWS 服务需要配置的顺序，也不必弄清楚让这些依赖项正常运行的细枝末节。CloudFormation 为您妥善处理。当设置完成后，您可通过按受控制、可预测的方式修改和更新 AWS 资源，您可像执行软件版本控制一样对您的 AWS 基础结构进行版本控制。您可以通过 AWS 管理控制台、CloudFormation 命令行工具或 API 对模板及其相关的资源集（称为“堆栈”）进行设置和更新。”

总之，Heat是来自于AWS的CloudFormation，设计与实现上都基本和Amazon保持一致。

对于Heat的功能和实现，简单来说就是用户可以预先定义一个规定格式的任务模版，任务模版中定义了一连串的相关任务（例如用某配置开几台虚拟机，然后在其中一台中安装一个mysql服务，设定相关数据库属性，然后再配置几台虚拟机安装web服务群集等等），然后将模版交由Heat执行，就会按一定的顺序执行heat模版中定义的一连串任务。

### Heat介绍
*******

基于预先定义的模板，Heat通过自身的orchestration Engine来实现复杂应用的创建启动。Heat原生的模板格式目前还在不停地演进中，但是对CloudFormation的格式具有良好的支持。存在的CloudFormation的模板可以在OpenStack平台通过heat来启动。 

##### Heat Client

Heat client是Heat project 提供的CLI工具，类似于其他项目的client。对于heat tools的使用，可以通过安装后查看，或者[通过此链接来查看。](http://docs.openstack.org/user-guide/content/heat_client_commands.html) 

##### heat-api

Heat-api 类似于nova-api，提供了原生的restful API对外使用。用户对API的调用，由heat-api处理之后，最终通过RPC传递给Heat-engine来进一步处理。 

##### heat-api-cfn

heat-api-cfn组件则提供了Amazon style 的查询 API，因此可以完全兼容于Amazon的CloudFormation，对于API的请求，同heat-api类似，处理之后，通过RPC传递给heat-engine进一步处理。

##### heat-engine

heat-engine是heat中的核心模块，主要的逻辑业务处理模块。此模块最终完成应用系统的创建和部署。

##### heat-cfntools

这个工具是一个单独的工具，代码没在heat project里面，可以单独下载。这个工具主要用来完成虚拟机实例内部的操作配置任务。在创建虚拟机竟像时，需要在镜像中安装heat-cfntools工具。

### 模板
*******

Heat templates支持多种模板内容展示格式，包括：HOT Syntax， YAML Syntax，JSON Syntax；每种格式表达的内容都一样，只是在表现形式上存在差别。关于模板的配置选项具体[可以参考此链接。](http://docs.openstack.org/developer/heat/template_guide/index.html)

同时，Heat templates还支持多种资源格式，包括OpenStack自身的资源格式，Amazon的资源格式和RackSpace的资源格式。目前，Heat templates所支持的内容还在进一步演变中，会越来越完善。

现在先参考官方贴出的一个简单例子，该模板中包含一个虚拟机实例，[链接在此。](https://github.com/openstack/)

* 虚拟机将使用的key-pair
* 虚拟机的flavor配置，heat的模板可以定义选项的默认值，还可以配置选项的值的选择范围。
* 虚拟机镜像的ID
* 虚拟机实例内部数据库的配置密码
* 虚拟机实例内部数据库的端口
* 指定创建虚拟机实例过程需要获取的输出，如实例的ip地址

从此模板中可以看出，该配置具有很大的灵活性，并且与OpenStack紧密融合，此点相比OVF来说，是天生的优势。下面再帖一个配置选项中的大致格式作为参考。

    parameters:
        KeyName:
            type: string
            description: Name of an existing key pair to use for the instance
        InstanceType:
            type: string
            description: Instance type for the instance to be created
            default: m1.small
            constraints:
                - allowed_values: [m1.tiny, m1.small, m1.large]
                description: Value must be one of 'm1.tiny', 'm1.small' or 'm1.large'
        ImageId:
            type: string
            description: ID of the image to use for the instance]

### 简单流程
*******

现在简单的描述heat是迅速创建一个复杂的应用并且完成最终的配置工作的整个流程。

##### 预处理

第一步，需要在虚拟机镜像中安装cloud-init和heat-cfntools两个工具。前者cloud-init是用来处理虚拟机实例早期的一些配置工作的，主要完成以下几方面的配置：

* 设置默认的locale
* 设置hostname
* 生成ssh private keys
* 添加ssh keys到用户的.ssh/authorized_keys文件中，方便用户登录。
* 设置ephemeral mount points

整个cloud-init可以通过创建应用时指定—user-data-file或者—user-data选项来设置。User-data的具体选项可以通过[此链接来查看](https://help.ubuntu.com/community/CloudInit)。

##### 创建应用

* 调用heat-api来创建，例如heat stack-create myApplication
* heat-engine生成cloud-init将会使用到的multipart data
* heat-engine调用nova-api，创建虚拟机实例，将cloud-init用到的数据随nova-api的调用传递。
* nova创建虚拟机实例。
* 当虚拟机实例启动时，将会执行cloud-init脚本，该脚本将做以下几件事：
** 从nova metadata server下载数据
** 将下载来的multipart data划分到/var/lib/cloud目录去
** 运行不同的cloud-init部分，例如resize the root filesystem, set the hostname
** 运行用户的脚本，脚本位于/var/lib/cloud/data/cfn-userdata目录，这些脚本必须调用cfn-init

### 蓝图 
*******

目前Heat没有放出一个完整的roadmap出来，所提供的blueprint均为lanuchpad上开发者注册提交的blueprint，实为一个个的feature。[可以参考此链接。](https://blueprints.launchpad.net/heat)

### 参考文档
*******

1. [http://www.infoq.com/cn/news/2011/02/aws-cloudformation](http://www.infoq.com/cn/news/2011/02/aws-cloudformation) Amazon AWS推出CloudFormation
2. [http://www.d1net.com/cloud/vendors/243228.html](http://www.d1net.com/cloud/vendors/243228.html) Amazon欲借CloudFormation切入企业IT市场
3. [http://aws.amazon.com/cn/cloudformation/](http://aws.amazon.com/cn/cloudformation/) AWS CloudFormation
4. [http://aws.typepad.com/aws/2012/04/aws-cloudformation-can-now-create-virtual-private-clouds.html](http://aws.typepad.com/aws/2012/04/aws-cloudformation-can-now-create-virtual-private-clouds.html) AWS CloudFormation 创建 私有云
5. [http://aws.amazon.com/cn/cloudformation/](http://aws.amazon.com/cn/cloudformation/) OpenStack Heat 介绍
6. [https://wiki.openstack.org/wiki/Heat](https://wiki.openstack.org/wiki/Heat) Heat Wiki
7. [https://wiki.openstack.org/wiki/Heat/ApplicationDeployment](https://wiki.openstack.org/wiki/Heat/ApplicationDeployment) 应用部署
8. [https://help.ubuntu.com/community/CloudInit](https://help.ubuntu.com/community/CloudInit) Cloud-init-介绍
9. [http://sdake.wordpress.com/2013/03/03/how-we-use-cloudinit-in-openstack-heat/](http://sdake.wordpress.com/2013/03/03/how-we-use-cloudinit-in-openstack-heat/) OpenStack Heat 与 CloudInit
10. [http://docs.aws.amazon.com/AWSCloudFormation/latest/UserGuide/deploying.applications.html](http://docs.aws.amazon.com/AWSCloudFormation/latest/UserGuide/deploying.applications.html) Amazon Deploying Application
11. [http://docs.openstack.org/developer/heat/getting_started/jeos_building.html](http://docs.openstack.org/developer/heat/getting_started/jeos_building.html) 创建一个heat-cfntools enabled image
12. [http://docs.openstack.org/developer/heat/template_guide/index.html](http://docs.openstack.org/developer/heat/template_guide/index.html) template 指南
13. [http://blog.csdn.net/gtt116/article/details/7878389](http://blog.csdn.net/gtt116/article/details/7878389) 详解heat-cfntools

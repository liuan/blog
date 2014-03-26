---
layout: post
category: OpenStack
tags: OpenStack
---

### 1

问题描述：原本正常运行好多天的平台从昨天磁盘满删掉些占空间的文件之后，出现各种奇怪的问题。其中之一就是，执行neutron命令提示`Authentication required`。其他project的命令没任何问题，奇了怪了！ 

查看keystone的日志，没有提示任何错误，再看neutron server的日志，发现其中提示`Token validation failure.` 再看traceback，提示`No JSON object could be decoded`，随后在代码中看了下，是jsonutils加载了一个从文件中读到的串，这个串是空的。读的文件是`/var/lib/neutron/keystone-signing/revoked.pem` ，空文件一个。

解决办法：
在`/var/lib/neutron/keystone-signing/revoked.pem`中添加下面的一行，从其他的project对应的文件中摘抄过来的。

    {"revoked":[]}

就这样，可以访问了。这个问题太诡异了！！！


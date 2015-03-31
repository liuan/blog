---
layout: post
category: Linux
tags: Linux
description: man 3 fmemopen
---

使用第三方库的时候，很可能遇到这样的问题，我们需要处理某个文件，而这个文件不一定是从本地磁盘上读取，可能是分布式文件系统或者其他地方，而第三方库的接口却只提供了一个`const char* path`参数，意味着只能从磁盘加载，没法直接处理已经加载到内存的数据。

这个时候，fmemopen就可以派上用场了，完美的将FILE对象映射到内存上，无需从磁盘上读取了。fmemopen的函数签名很简单，如下：

    FILE *fmemopen(void *buf, size_t size, const char *mode);

需要注意的是，在使用前最好仔细的阅读 man fmemopen的内容，尤其是在wa两种模式下，函数的行为。

有两个小问题需要注意下：

* 在glibc 2.9之前的版本暂不支持`b`模式，这个问题带来的影响是，若以binary模式打开，进行fseek(stream, 0L, SEEK_END)时，pos指针很可能不会到真正的尾，而是buffer中第一个 `\0` 处。对比glibc 2.5 和glibc 2.21的代码可以知道：


    //glibc 2.21
    case SEEK_END:
      np = (c->binmode ? c->size : c->maxpos) - *p;
      break;


    //glibc 2.5
    case SEEK_END:
      np = c->maxpos - *p;
      break;

在非`b`模式下，glib2.5是按maxpos处理的，而maxpos是通过strlen(buffer)得到的，glib 2.21则是c->size，即调用fmemopen时传入的值。


* 对于在glibc 2.10以下的版本中，使用fmemopen编译时会遇到这样的警告:

    warning: assignment makes pointer from integer without a cast

这个警告的解决方式是在include stdio.h头文件之前添加一行`#define _GNU_SOURCE`，正如man page上所说。

### 应用
--------------

有了fmemopen之后，譬如我们需要将从已经加载到内存的一个zip文件提取出文件列表，即这个`unzip -l file`，对于解压zip文件，我目前知道的库是minizip，简单好用，然而，这个minizip中提供的接口都是`const char *path`，通过path调用fopen得到一个FILE的对象，然后在这个FILE对象上封装一系列的操作，这些操作就复杂了，没必要进行深入了解。在fmemopen的协助下，我们可以内存中获取一个FILE对象出来，然后再给minizip使用，轻而易举的解决了这个问题。



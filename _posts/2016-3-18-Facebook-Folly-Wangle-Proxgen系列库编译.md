---
layout: post
category: facebook
description: 编译
---

为了编译FB的Folly，强行在自己的CentOS6.6 虚拟机上源码编译安装了gcc 4.8.5版本，结果编译过程踩到各种坑，源码安装gcc导致系统中存在两个版本。在链接过程中出现很多异想不到的问题。

重新在aliyun的虚拟机上编译FB的Folly, Wangle, Proxygen 三个库，因此顺利多了，现在将这个过程纪录下来，还有些问题待解决。

### Folly 编译
-----------------

Folly的编译需要支持c++11 的gcc，aliyun上选择的CentOS 7.0 安装的gcc 是4.8.5版本，可以顺利编译。在编译Folly 之前需要安装几个依赖库，

* gflags
* glog
* double-conversion
* libevent

这几个库，我均采用源码编译的方式编译，注意若将folly编译成动态库时，编译以上几个库需要指定`-fPIC`。以上的库都编译到指定的目录，编译好之后，开始编译folly库，

* cd  folly/folly
* LDFLAGS="-L/home/choudan/code/common/lib" CPPFLAGS="-I/home/choudan/code/common/include -I/home/choudan/code/common/include/libevent -fPIC -g -lpthread" ./configure  --prefix=/home/choudan/code/common/folly/ --exec-prefix=/home/choudan/code/common/folly/ --disable-shared 
* make
* make install

### Wangle 编译
-----------------

Wangle 是采用的cmake来编译，查看Wangle的CMakeLists.txt会发现，Wangle 依赖于Folly，需要提供一个查找folly的方式。

* find_package(Folly REQUIRED)

因此创建一个FindFolly.cmake文件，内容如下：

    CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)

    INCLUDE(FindPackageHandleStandardArgs)

    set(MY_FOLLY_INCLUDEDIR "/home/choudan/code/common/include")
    set(MY_FOLLY_LIBRARY "/home/choudan/code/common/lib")

    FIND_LIBRARY(FOLLY_LIBRARY folly
        HINTS ${MY_FOLLY_LIBRARY})

    FIND_PATH(FOLLY_INCLUDE_DIR "folly/String.h"
        HINTS ${MY_FOLLY_INCLUDEDIR})

    SET(FOLLY_LIBRARIES ${FOLLY_LIBRARY})

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Folly
      REQUIRED_ARGS FOLLY_INCLUDE_DIR FOLLY_LIBRARIES))})})})))))

将此文件放到当前目录下，然后再修改wangle的CMakeLists.txt，添加当前目录为搜索目录

* set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} "./")

添加链接目录，和头文件查找目录，

    LINK_DIRECTORIES(/home/choudan/code/common/lib)

    set(COM_INCLUDE_DIR "/home/choudan/code/common/include")
    set(EVENT_INCLUDE_DIR "/home/choudan/code/common/include/libevent")
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /home/choudan/code/common/lib)

注意还需要在target_link_libraries中添加依赖的库，链接的时候会提示这些坑的。

最后，由于CMakeLists.txt 中开启了test的话，会下载gmock，这个目前我是直接注释的，不处理gmock，编译代码中的examples。然后开始编译,

* cmake -DCMAKE_INSTALL_PREFIX=/home/choudan/code/common/wangle -DBUILD_EXAMPLES=ON 
* make
* make install

### Proxygen 编译
-----------------

解决wangle编译之后，开始处理Proxygen，由于已经安装好了folly和wangle，因此不需要完全按照Proxygen下的deps.sh 来操作，直接到deps.sh的末尾就可以了，编译的过程也会下载gmock，默认的下载路径在googlecode上，因此我直接先注释处理，在lib/test/Makefile中，注释gmock的两行

* autoreconf -ivf
* LDFLAGS="-L/home/choudan/code/common/lib" CPPFLAGS="-I/home/choudan/code/common/include -I/home/choudan/code/common/include/libevent -fPIC -g -lpthread -levent -lglog -lgflags" ./configure  --prefix=/home/choudan/code/common/proxygen/ --exec-prefix=/home/choudan/code/common/proxygen/  --disable-shared
* make && make install

在链接的过程中间，遇到一个奇怪的问题，找不到event库中的符号，后面查看编译时的命令，发现时event库在链接的先后顺序中先于folly库，因此folly库没法找到相应符号。只有强行在configure 文件中把wangle和folly提前，再编译搞定。


### 后述
-----------------

之所以需要编译这三个库，是因为Folly是基础的C++11库，提供了底层的网络封装，Wangle则基于folly，实现了一套异步的类似于Netty的网络库，Proxygen则基于wangle，实现了功能强劲的HTTP相关的库。

基于这几个库，可以自己强行玩一玩，学习C++11，Folly中很多可以学习的东西，弄个C++11的爬虫试试，当然开发量会比Python多不少。



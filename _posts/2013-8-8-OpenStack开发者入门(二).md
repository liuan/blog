---
layout: post
category: OpenStack
tags: OpenStack 
---

从去年的十月份下旬到现在，都已经大半年了，接触OpenStack的时间也不算少了，在昨天之前，一直以为自己对OpenStack了解的还不错，可是不到考验时候，真的不知道自己有多肤浅，对其中的很多基本功能，都只能给出一个语焉不详的答复，其实这源于不熟悉，不了解。若真的熟练理解了，并可以头头是道！ 还有需要努力提高自己的表达能力，有条理，清晰，言简意赅！

以后的文章，都把它写的层次分明，条理清晰，大家看了就能够明白，也是对我自己的思维训练！骚年，还是有努力！

进入正题！

下面的内容还不够完善，会在后续自己经验丰富之后，渐渐补充！

接着上一篇[OpenStack开发者入门一](http://www.choudan.net/2013/07/24/OpenStack开发者入门%28一%29.html)的内容,上一篇讲了一个轮廓，若要成为一名OpenStack社区开发者需要做哪些前提工作，然后是OpenStack采取的gerrit的工作方式,将project source code下载到本地来，准备好git review环境。这一篇，就详细描述report Bug,assign Bug, commit Patch的过程。

### Assign Bug

第一步就是给自己分配一个bug，可以是自己提交的也可以他人提交的。

> 1.在[nova bug](https://bugs.launchpad.net/nova)页面，列出了目前存在的所有Bug
> 2.可以在右上角点击Report a bug 或者 查看任何一个Bug
> 3.Report a bug首先需要对这个bug进行一个简短的描述，就是标题，下一步，确认是否有类似的Bug，然后确认是提交新的bug，最后对bug进行详细的描述，描述完后提交
> 4.在Bug的详细描述页面，有一个列表，可以看到该Bug是否已经分配，可以点击图标将Bug分配给自己！

<img src="/assets/img/openstack_develop_assign_bug.png" width="700px"/>

### Commit Patch

每个Bug都有一个7位数的id 例如，1210043，可以在Bug的详细描述页面的url中看到。例如我们已经将nova的源码clone下来，准备好本地的git review环境了（前一篇提到），我们需要做如下的操作来完成对bug的修复。

1.在python-novaclient中为此bug创建一个分支，分支的名称为Bug1210043
> 1.进入到nova目录
> 2.git checkout master确保python-novaclient在主版本中
> 3.git checkout -b Bug1210043

2.在分支Bug1210043中修改python-novaclient的代码。
3.修改完成之后，貌似还需要自己做好功能测试和继承测试，由于我个人修复的bug十分简单，自己做了充分的验证，略过了这一步。
4.将代码提交给Gerrit。
> 1.进入到python-novaclient的目录 
> 2.git commit -a
> 3.对这个提交输入注释，注释是讲究格式的，分为三部分。第一段，简短的描述；第二段，详细说明(可选)；第三段，如果此分支修复一个But或者一个blueprint，则添加Fixes: Bug 1210043或者蓝图BlueprintXXX。
> 4.运行ctrl+o命令，然后按下Enter键，然后运行ctrl+x.
> 5.git review

对于第三处，初次尝试的可能会产生困惑，有的文章描述注释部分需要添加change-Id，这个change-Id是commit之后才有的，第一次提交不需要填写change-Id,下面是我初次提交的注释内容，给大家截个图，作为参考！其中的change-Id是系统自动添加的。*其中的注释方式，内容太简陋了，没有通过review，将再次修改的方式补充后文章尾。*

<img src="/assets/img/openstack_develop_gitreview02.png" width="700px"/>

在执行git review之后，我得到了一个fatal error，原因是自己在review上的contact information不够完善。

<img src="/assets/img/openstack_develop_gitreview01.png" width="700px"/>

需要完善下图中关于自己的一些信息，可以不用填全，如果还提示要到openstac.org/register上去注册，则过去注册就好了！

<img src="/assets/img/openstack_develop_contactinformation.png" width="700px"/>

补充完这些信息之后，再次执行git review之后就提交成功了，随后就是上文描述的gerrit将执行的流程了。

### 总结

我提交的第一个bug很简单，只需要修改一行代码，平时也不会影响到系统的正常运行，但是还是一个隐患。很快jenkins完成了build，然后再等待code-review和Approved。后续若有更加全面的经验，再补充过来！

第一次review没有通过，并且给出了一个意见，要我[参考openstack wiki如何写好git commit message](https://wiki.openstack.org/wiki/GitCommitMessages#Information_in_commit_messages)。之所以没有通过是commit message写的太简陋了，无法准确的表述解决的问题，所以我重新提交了commit message!

> 1.确保当前代码是最新的，git fetch origin master
> 2.git rebase FETCH_HEAD
> 3.git add.
> 4.git commit --amend，然后重新编写commit messages，上面的wiki给出了详细的描述
> 5.git review

其中在执行git commit --amend之后，就会出现之前commit 的messages，末尾已经添加了change-Id，只需要专注于把messages写清晰就好！老外做事真是够严谨！不得不佩服！

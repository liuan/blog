---
layout: post
category: Linux
tags: 网络
---

上一篇文章中提到了[Unix的5种网络IO模型](http://www.choudan.net/2013/08/17/Unix网络IO模型)，提到了select系统调用。select函数允许进程指示内核等待多个事件中的任何一个发生，并只在有一个或多个事件发生或经历一段指定的时间后才唤醒它。

首先看select函数的原型。

    #include<sys/select.h>
    #include<sys/time.h>

    int select(int maxfdp1,fd_set *readset, fd_set *writeset,fd_set *exceptset, const struct timeval *timeout);

其中`fd_set`便是我们需要监听的文件描述符集合，譬如需要监听标准输出和标准错误输出描述符，则通过下面的语句：

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(1,&rset);
    FD_SET(2,&rset);

select的第一个参数指定待测试的描述符个数，所以便是fd_set集合中文件描述符值最大的数再加1。例如上面的fd_set中maxfdp1 = 2 + 1 = 3,因为描述都死从0开始，所有有三个。`没有细看select的具体原理，为什么需要测试0-n所有的描述符`。readset集合是测试其中的任何描述符是否可读，writeset则是测试其中的任何描述符是否可写了。

select函数修改由指针readset,writeset和exceptset所指向的描述符集，因而这三个参数都是值，结果参数，函数返回之后，就可以从其中读取结果。调用select函数时，我们指定所关心的描述符的值，该函数返回时，结果将指示哪些描述符已经就绪。该函数返回之后，可以使用FD_ISSET宏来测试fd_set数据类型中的描述符。`描述符集内任何于未就绪描述符对应的位返回时均清0，为此，每次重新调用select函数时，我们都要再次吧所有描述符集内所关心的位均置1。`

select模式存在一个问题，就是fd_set的大小有限制，操作系统对每个进程可用的最大描述符数设置了上限，另一个问题是，每次select返回之后，需要遍历所有FD_SET中的描述符，这样就导致此模型下，处理时间会与FD_SET的规模成正比，O(N)的复杂度。

下面给出一个关于select的例子。

#### server端

    #include  <unistd.h>
    #include  <sys/types.h>       /* basic system data types */
    #include  <sys/socket.h>      /* basic socket definitions */
    #include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
    #include  <arpa/inet.h>       /* inet(3) functions */
    #include <sys/select.h>       /* select function*/

    #include <stdlib.h>
    #include <errno.h>
    #include <stdio.h>
    #include <string.h>

    #define MAXLINE 10240

    void handle(int * clientSockFds, int maxFds, fd_set* pRset, fd_set* pAllset);

    int  main(int argc, char **argv)
    {
        int  servPort = 6888;
        int listenq = 1024;

        int  listenfd, connfd;
        struct sockaddr_in cliaddr, servaddr;
        socklen_t socklen = sizeof(struct sockaddr_in);
        int nready, nread;
        char buf[MAXLINE];
        int clientSockFds[FD_SETSIZE];
        fd_set allset, rset;
        int maxfd;

        listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (listenfd < 0) {
            perror("socket error");
            return -1;
        }

        int opt = 1;
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("setsockopt error");    
        }  

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(servPort);


        if(bind(listenfd, (struct sockaddr*)&servaddr, socklen) == -1) {
            perror("bind error");
            exit(-1);
        }

        if (listen(listenfd, listenq) < 0) {
            perror("listen error");
            return -1;
        }

        int i = 0;
        for (i = 0; i< FD_SETSIZE; i++) 
            clientSockFds[i] = -1; 
        FD_ZERO(&allset);
        FD_SET(listenfd, &allset); 
        //FD_SET(0, &allset); 
        maxfd = listenfd;    

        printf("echo server use select startup, listen on port %d\n", servPort);
        printf("max connection: %d\n", FD_SETSIZE);

        printf("maxfd is :%d\n",maxfd + 1);
        for ( ; ; )  {
            rset = allset;
            nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
            if (nready < 0) {
                perror("select error");
                continue;
            }

            if (FD_ISSET(listenfd, &rset)) {
                connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &socklen);
                if (connfd < 0) {
                    perror("accept error");
                    continue;
                }

                sprintf(buf, "accept form %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
                printf(buf, "");

                for (i = 0; i< FD_SETSIZE; i++) {
                    if (clientSockFds[i] == -1) {
                        clientSockFds[i] = connfd;
                        break;
                    }
                }
                if (i == FD_SETSIZE) {
                    fprintf(stderr, "too many connection, more than %d\n", FD_SETSIZE);
                    close(connfd);
                    continue;
                }
                if (connfd > maxfd)
                    maxfd = connfd;

                FD_SET(connfd, &allset);
                if (--nready <= 0)
                    continue;
            }

            handle(clientSockFds, maxfd, &rset, &allset);
        }
    }


    void handle(int * clientSockFds, int maxFds, fd_set* pRset, fd_set* pAllset) {
        int nread;
        int i;
        char buf[MAXLINE];
        for (i = 0; i< maxFds; i++) {
            if (clientSockFds[i] != -1) {
                if (FD_ISSET(clientSockFds[i], pRset)) {
                    nread = read(clientSockFds[i], buf, MAXLINE);//读取客户端socket流
                    if (nread < 0) {
                        perror("read error");
                        close(clientSockFds[i]);
                        FD_CLR(clientSockFds[i], pAllset);
                        clientSockFds[i] = -1;
                        continue;
                    }
                    if (nread == 0) {
                        printf("client close the connection\n");
                        close(clientSockFds[i]);
                        FD_CLR(clientSockFds[i], pAllset);
                        clientSockFds[i] = -1;
                        continue;
                    } 

                    write(clientSockFds[i], buf, nread);//响应客户端  有可能失败，暂不处理
                }
            }
        }

    }

在sever端调用完select之后，就开始判断监听的socket是否状态发生变化，若是的，则是有新的连接请求接进来，程序然后就把新的连接请求建立的socket保存起来，并且保证连接请求不超过最大值。然后把连接请求加入到fd_set中去，下次调用select，则可以监听更多的连接请求，这与普通的accept相比，不会阻塞成每次只能处理一个请求，处理完这个请求之后才能去做下一个请求，在select模式下，可以并发的处理多个请求，最大数量不超过fd_set的最大大小。

#### client端 

    #include  <unistd.h>
    #include  <sys/types.h>       /* basic system data types */
    #include  <sys/socket.h>      /* basic socket definitions */
    #include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
    #include  <arpa/inet.h>       /* inet(3) functions */
    #include <sys/select.h>       /* select function*/

    #include <stdlib.h>
    #include <errno.h>
    #include <stdio.h>
    #include <string.h>

    #define MAXLINE 10240
    #define max(a,b)    ((a) > (b) ? (a) : (b))
    //typedef struct sockaddr  SA;

    void handle(int sockfd);

    int main(int argc, char **argv)
    {
        char * servInetAddr = "127.0.0.1";
        int servPort = 6888;
        char buf[MAXLINE];
        int connfd;
        struct sockaddr_in servaddr;

        if (argc == 2) {
            servInetAddr = argv[1];
        }
        if (argc == 3) {
            servInetAddr = argv[1];
            servPort = atoi(argv[2]);
        }
        if (argc > 3) {
            printf("usage: selectechoclient <IPaddress> <Port>\n");
            return -1;
        }

        connfd = socket(AF_INET, SOCK_STREAM, 0);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(servPort);
        inet_pton(AF_INET, servInetAddr, &servaddr.sin_addr);
        
        if (connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            perror("connect error");
            return -1;
        }
        printf("welcome to selectechoclient\n");
        handle(connfd);     /* do it all */
        close(connfd);
        printf("exit\n");
        exit(0);
    }


    void handle(int connfd)
    {
        FILE* fp = stdin;
        char sendline[MAXLINE], recvline[MAXLINE];
        fd_set rset;
        FD_ZERO(&rset);
        int maxfds = max(fileno(fp), connfd) + 1;
        int nread;
        for (;;) {
            FD_SET(fileno(fp), &rset);
            FD_SET(connfd, &rset);

            if (select(maxfds, &rset, NULL, NULL, NULL) == -1) {
                perror("select error");
                continue;
            }

            if (FD_ISSET(connfd, &rset)) {
                //接收到服务器响应
                nread = read(connfd, recvline, MAXLINE);
                if (nread == 0) {
                    printf("server close the connection\n");
                    break;
                } 
                else if (nread == -1) {
                    perror("read error");
                    break;    
                }
                else {
                    //server response
                    write(STDOUT_FILENO, recvline, nread);    
                }  
            }

            if (FD_ISSET(fileno(fp), &rset)) {
                //标准输入可读
                if (fgets(sendline, MAXLINE, fp) == NULL) {
                    //eof exit
                    break;   
                } 
                else {
                    write(connfd, sendline, strlen(sendline));  
                }
            }

        } 
    }


### 总结

关于select函数，在Unix网络编程一书上面还详细介绍了各种类型的描述符就绪的条件，通信中客户端服务端关闭的问题和详细的关于select的例子。select具有两个较大的硬伤，处理的并发请求有限制，需要每次都遍历集合，这样性能必然受到影响，那就有一种更加好的模式来处理网络IO。下一篇简单的介绍epoll,把学习python eventlet前的基础知识准备到。

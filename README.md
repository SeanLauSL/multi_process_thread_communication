# 多进程传输数据

## 一、FIFO管道文件

### 1. 相关概念

```text
ulimit -a 查看管道大小 pipe size 512*8=4kb
PIPE_BUF也是4kb
实际pipe capacity 为65536即16个缓存块 64kb
```

* 1、页缓冲区大小：4K
* 2、总缓冲区大小：64K
* 3、<4K的数据立即发送，以页为单位 写入具有原子性
* 4、>4K的数据，将会分成多个页的数据，分批发送。

```text
函数 write要么阻塞，要么成功（copy全部数据到内核缓冲区，不存在只copy部分数据的情况），异常换回-1；当数据大于64K 也是通过分页处理，不过此时不能使用非阻塞方式，下面有介绍。
注意：当单个数据小于4K时，缓冲区会存下多个数据，即使停止写入，还可以读取未读取的数据，也就是数据不是最新的；
当数据大于64K时，当前数据基本是最新的；（因为存不下整个数据）
```

### Concept Reference

[管道读写规则和Pipe Capacity、PIPE_BUF](https://www.cnblogs.com/alantu2018/p/8477339.html)

```text
    分析一下：现在的情况是有两个子进程在对管道进行阻塞写入各68k，即每个子进程完全写入68k才返回，而父进程对管道进行阻塞读取，每次读取4k，打印每4k中的最后一个字符，如果没有数据到达就阻塞等待，如果管道剩余数据不足4k，read 很可能返回 < 4k，但因为我们写入68k是4k整数倍，故不存在这种情况。需要注意的是是边写边读，因为前面说过管道的容量只有64k，当管道被写满时子进程就阻塞等待父进程读取后再写入。由上面输出可以看出B进程先写入64k的B，然后A进程写入68k的A之后B进程接着写完最后4K的B，然后write返回。由A进程write完毕输出的提示可知此时A进程已经写完成了，但父进程还没读取A完毕，当两个子进程全部写完退出时关闭写端文件描述符，则父进程read就会返回0，退出while循环。可以得出结论：当多个进程对管道进行写入，且一次性写入数据量大于PIPE_BUF时，则不能保证写入的原子性，即可能数据是穿插着的。man 手册的解释如下：
```

```text
O_NONBLOCK disabled, n > PIPE_BUFThe write is nonatomic:
the data given to write(2) may be interleaved with write(2)s by other process;  
the write(2) blocks until n bytes have been written.
```

```text
    注意我们这里设定了size=68k，则写端不能设置成非阻塞，因为Pipe Capacity 只有64k，不能一次性写入68k，如果此时管道是满的（64k)，则只能返回-1并置错误码为EAGAIN，且一个字符也不写入，若不是满的，则写入的字节数是不确定的，需要检查write的返回值，而且这些字节很可能也与其他进程写入的数据穿插着。读端也不能设置为非阻塞，如果此时尚未有数据写入（管道为空）则返回-1并置错误码为EAGAIN，如果有部分数据已经写入，则读取的数据字节数也是不确定的，需要检查read的返回值。总之测试4种不同情形下的情况也应设置不同的条件。
```

```text
O_NONBLOCK disabled, n <= PIPE_BUF
All n bytes are written atomically;
write(2) may block if there is not room for n bytes to be written immediately

O_NONBLOCK enabled, n <= PIPE_BUF
If  there  is  room  to write n bytes to the pipe, then write(2) succeeds immediately, writing all n bytes;otherwise write(2) fails, with errno set to EAGAIN.

O_NONBLOCK disabled, n > PIPE_BUF
The write is nonatomic:
the data given to write(2) may be interleaved with write(2)s by other process;  
thewrite(2) blocks until n bytes have been written.

O_NONBLOCK enabled, n > PIPE_BUF
If  the  pipe  is full, then write(2)fails, with errno set to EAGAIN.  
Otherwise, from 1 to n bytes may be written (i.e., a "partial write" may occur;
the caller should check the return value from write(2) to seehow many bytes were actually written),and these bytes may be interleaved with writes by other processes.
```

### 2. 传输cv::Mat

```text
由于cv::Mat一般都大于4Kb，所以是分页传输。如果是一个进程写入管道，多个进程读取管道，则可能数据交错。

可以通过一个进程把数据写入多个管道，再由读进程分别读各自的管道，但是必须多个管道同时工作，有一个挂起，另一个也挂起了；且管道写入有先后。另外如果有多个进程需要读取管道，就要写多个管道，不太方便。

如果使用多进程fork(),好像不能同时操作一个相机设备。

后续应该选择使用共享内存，把整个任务队列都写入共享内存。
```

### 3. Code

* Class FIFOIO

```text
src/fifofile.cpp
include/fifofile.h
```

* Samples

```text
samples/readfifo.cpp
samples/writefifo.cpp
```

* Issue

a)、`src/fifofile.cpp` 函数push_back()里的nbytes未检查；

b)、`src/fifofile.cpp` 函数push_back()里write失败后应该关闭管道；

c)、FIFOIO的成员buffer还没有free()。

### FIFOIO Reference

[linux下的有名管道文件读写操作](https://blog.csdn.net/tiramisu_L/article/details/80176350)

[半双工管道](https://blog.csdn.net/qq_41261736/article/details/99686651)

[read和write函数](https://blog.csdn.net/u013774102/article/details/79083698)

[c++ fork进程与同步锁](https://www.cnblogs.com/sssblog/p/10457114.html)

[fork()函数详解](https://www.cnblogs.com/love-jelly-pig/p/8471206.html)

[fork()使用场景](https://blog.csdn.net/sodino/article/details/45146001)

## 二、任务队列

### 使用场景

```text
数据生产速度不可忽略，如嵌入式平台的生产图像和处理图像：图像压入队列，队列过长时，主动清除队列旧数据，保证获取图像较新。如果使用单一共享变量，数据消费线程可能会读取到重复数据，使用队列好一点，虽然队列数据会有一点延迟，比不上使用单一共享变量，但一旦单一共享变量重复，且数据处理过程较长，那么这一重复帧造成的延迟更高。
```

### Code

* Class CaptureQueue

```text
src/capThread.cpp
include/capThread.h
```

* Samples

```text
samples/queue_test.cpp
```

### 泛式模板类deque

* Class TQueueConcurrent

```text
include/deque.hpp
```

* Samples

```text
samples/deque_tempate_test.cpp
```

### Queue Reference

[linux线程与进程同步锁机制](https://blog.csdn.net/zhuoyue01/article/details/105984882/)

[C++多线程，多线程通信，队列](https://blog.csdn.net/m0_37542524/article/details/93642813)

## 三、共享内存

一个进程把图像写入共享内存，其他进程读取该内存，这个需要抛弃任务队列的做法，否则队列里的数据可能是重复的。

### Share memory Reference

[1、进程间通信：共享内存+读写锁 实现多个进程间同步通信](https://www.csdn.net/gather_2a/NtzaQgxsMTItYmxvZwO0O0OO0O0O.html)

[2、进程同步：共享内存](https://www.cnblogs.com/ducong/p/6590544.html)

### Code of Share memory

#### 1）、共享内存+读写锁

实现整个共享内存的读共享，写独占

* Samples

```text
include/shm_common.h
samples/producer.cpp
samples/consumer.cpp
```

#### 2）、共享内存+循环队列

共享内存中的缓存数据为循环队列结构，有多个数据元素。没有锁，使用占用标志位进行同步，效果类似互斥锁，一个写者，多个读者，全部互斥，也就是当前元素被占用，无论是写还是读都无法访问，寻找下一个可访问元素；有待改进为单元素的读共享，写独占。

* Class ShmCirQueue

```text
src/shmcirqueue.cpp
include/shmcirqueue.h
```

* Samples

```text
samples/producer_shmcq.cpp
samples/consumer_shmcq.cpp
```

### Issue

* a)、 共享内存已经存在的情况下，会导致无法创建共享内存；应该参考 [Linux进程间的循环队列内存共享](https://blog.csdn.net/oHanTanYanYing/java/article/details/84988851)进行修正：

```cpp
shmdata=new unsigned char*[datanum];//指针数据
// 共享内存对象映射
pp = (unsigned int *)p;
exist=pp+5;//内存存在标志位

if (*exist==99)//原先内存没有销毁，先销毁掉再创建
{
    printf("原先内存没有销毁，先销毁掉再创建\n");
    if(-1 == shmctl(shmid, IPC_RMID, NULL))
    {
        perror("shmctl failed");
        exit(4);
    }
}

```

```text
    另外：调试时可以通过系统命令ipcs查看共享内存；ipcrm -m id删除共享内存。
```

* b)、 当写进程中断后，读进程一直在读重复的缓存区域，待修正。

    使用线程清理，防止死锁。查看有道云笔记《线程、线程锁、进程通信、进程同步》

* c)、 共享内存结构补充  

```text
共享内存结构体应该添加引用计数标志位(构造时+1，析构时-1)，记录有多少进程访问该共享内存，当访问该共享内存的进程退出时，若此时引用计数==1，则删除共享内存，否则仅解除映射。
```

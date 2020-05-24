#ifndef __SHM_CIR_QUEUE_H__
#define __SHM_CIR_QUEUE_H__

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>

#define DATA_SZ 1920*1080*3 //元素最大尺寸 sizeof(unsigned char) = 1
#define DATA_NUM 4 //元素个数

//如果使用结构图，必须内部初始化，使得其与连续的共享内存对应
//或者仅使用char*  共享内存为[head + data]模式，以head来进行同步
struct shareData
{
    unsigned int rPtr; //读索引下标
    unsigned int wPtr; //写索引下标
    bool busy[DATA_NUM]; //占位标志，读写保护
    char data[DATA_NUM][DATA_SZ]; //数据
};

class ShmCirQueue
{
public:
    ShmCirQueue();
    ~ShmCirQueue();

    ShmCirQueue(key_t shmKey, int _mode);

public:
    /*
    @brief 创建共享内存并附加到本进程
    @param shmKey 共享内存标志键值
    */
    void prepareSharedMemory(key_t shmKey, int _mode);

    /*
    @brief 释放共享内存
    */
    void releaseSharedMemory();

    /*
    @brief 初始化共享内存的值
    */
    void initShmParam();

    /*
    @brief 设置数据尺寸
    @param width height nchannel 
    @param bytes_per_element 每单元字节数 如sizeof(unsigned char)
    */
    void setDataSize(unsigned int width, 
        unsigned int height, 
        unsigned int nchannel,
        size_t bytes_per_element);

    /*
    @brief 设置写入帧率
    */
    void setWrFps(unsigned int fps);

     /*
    @brief 元素数据压入队列
    @param buffer 输入数据
    @return 压入数据bytes
    */
    size_t push(char* buffer);

    /*
    弹出未被占用的一个队列元素，
    */
    char* pop();

private:
    /*
    移动索引下标
    */
    void movePtr(unsigned int& ptr);

private:
    struct shareData *shm; //共享变量
    int shmid; //共享内存获取返回标志shmid
    void *shared_memory = (void *)0;; //原始共享内存
    int mode; //访问共享内存方式 READ、WRITE
    //char* buffer = NULL; //元素数据(读取进程缓存)
    char* buffer = (char*)malloc(this->nbytes);

public:
     enum Mode
    {
        READ        = 0, //!< value, open the file for reading
        WRITE       = 1, //!< value, open the file for writing
    };

    //数据元素尺寸
    int width = 640;
    int height = 480;
    int nchannel = 3;
    size_t bytes_per_element = sizeof(unsigned char); //元素的一个单元的字节数，如sizeof(unsigned char)
    //size_t nbytes = 0; //一个数据元素总字节数 width*height*nchannel*bytes_per_element
    size_t nbytes = width*height*nchannel*bytes_per_element;

    //队列写入帧率
    unsigned int wfps = 100;


};


#endif
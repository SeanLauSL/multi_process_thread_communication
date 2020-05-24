#ifndef _SHMCOM_H_HEADER  
#define _SHMCOM_H_HEADER

const int width = 640;
const int height = 480;
#define DATA_SZ 640*480*3
//如果使用结构图，必须内部初始化，使得其与连续的共享内存对应
//或者仅使用char*  共享内存为[head + data]模式，以head来进行同步
struct shareData
{
    pthread_rwlock_t rwlock;//读写锁
    char data[DATA_SZ];
    ///*unsigned*/ char* data;
};

#endif
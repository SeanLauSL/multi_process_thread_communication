#include "shmcirqueue.h"

ShmCirQueue::ShmCirQueue()
{

}

ShmCirQueue::~ShmCirQueue()
{
    free(this->buffer);
    releaseSharedMemory();
}

ShmCirQueue::ShmCirQueue(key_t shmKey, int _mode)
{
    //创建共享内存并附加到本进程
    prepareSharedMemory(shmKey, _mode);
    if(this->mode == WRITE)
    {
        //初始化共享内存值
        initShmParam();
    }
    //初始化元素数据(读取进程缓存)
    buffer = (char*)malloc(this->nbytes);
}


/*
    @brief 创建共享内存并附加到本进程
    @param shmKey 共享内存标志键值
    */
void ShmCirQueue::prepareSharedMemory(key_t shmKey, int _mode)
{
    this->mode = _mode;
     /*IPC_CREAT表示在key标识的共享内存不存在时，创建共享内存*/
    this->shmid = shmget(shmKey, sizeof(struct shareData), 0666 | IPC_CREAT);
    if (this->shmid == -1){
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    /*将共享内存附加到本进程,调用成功时返回一个指向共享内存第一个字节的指针*/
    this->shared_memory = shmat(this->shmid, (void *)0, 0);
    if (this->shared_memory == (void *)-1){
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Memory attached at %X\n", (long)(this->shared_memory));

    this->shm = (struct shareData *)(this->shared_memory);
}


/*
    @brief 释放共享内存
    */
void ShmCirQueue::releaseSharedMemory()
{
     // 解除映射
    if (shmdt(this->shared_memory) == -1) 
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }

    if(this->mode == WRITE)
    {
        // 删除共享内存
        if (shmctl(this->shmid, IPC_RMID, 0) == -1)
        {
            printf("shmctl(IPC_RMID) failed\n");
            exit(EXIT_FAILURE);//exit(1)??
        }
    }
    
}


/*
    @brief 初始化共享内存的值
    */
void ShmCirQueue::initShmParam()
{
    this->shm->rPtr = 0;
    this->shm->wPtr = 0;
    memset(this->shm->busy, false, DATA_NUM);
    for(int i = 0; i < DATA_NUM; i++)
        memset(this->shm->data[i], NULL, DATA_NUM);

    printf("initialized shared memory \n");
}


/*
    @brief 设置数据尺寸
    @param width height nchannel 
    @param bytes_per_element 每单元字节数 如sizeof(unsigned char)
    */
void ShmCirQueue::setDataSize(unsigned int width, 
    unsigned int height, 
    unsigned int nchannel,
    size_t bytes_per_element)
{
    
    this->width = width;
    this->height = height;
    this->nchannel = nchannel;
    this->bytes_per_element = bytes_per_element;
    this->nbytes = width*height*nchannel*bytes_per_element;

    if(this->buffer != NULL)
        free(this->buffer);
    this->buffer = (char*)malloc(this->nbytes);
}


/*
    @brief 设置写入帧率
    */
void ShmCirQueue::setWrFps(unsigned int fps)
{
    this->wfps = fps;
}


/*
    移动索引下标
    */
void ShmCirQueue::movePtr(unsigned int& ptr)
{
    ptr++;
    if(ptr >= DATA_NUM)
    {
        ptr = 0;
    }
}


 /*
    @brief 元素数据压入队列
    @param data 输入数据
    @return 压入数据bytes
    */
size_t ShmCirQueue::push(char* buffer)
{
    if(buffer == NULL)
    {
        releaseSharedMemory();
        fprintf(stderr, "[%s line:%d] push data failed\n", __FILE__, __LINE__);
        //return 0;
        throw buffer;
    }
    if (this->nbytes == 0)
    {
        releaseSharedMemory();
        fprintf(stderr, "[%s line:%d] size of data is not set.\n", __FILE__, __LINE__);
        throw this->nbytes;
    }

    //忙线检测
    while(shm->busy[shm->wPtr])
    {
        /*shm->wPtr++;
        if(shm->wPtr >= DATA_NUM)
        {
            shm->wPtr = 0;
        }*/

        movePtr(shm->wPtr);
    }
    //占线写内存
    shm->busy[shm->wPtr] = true;
    memcpy(shm->data[shm->wPtr], buffer, this->nbytes);
    shm->busy[shm->wPtr] = false;
    movePtr(shm->wPtr);

    usleep(1000 * 1000/this->wfps);
    return this->nbytes;
}


/*
    弹出未被占用的一个队列元素，
    */
char* ShmCirQueue::pop()
{
    if (this->nbytes == 0)
    {
        releaseSharedMemory();
        fprintf(stderr, "[%s line:%d] size of data is not set.\n", __FILE__, __LINE__);
        throw this->nbytes;
    }
    //忙线检测
    while(shm->busy[shm->rPtr])
    {
        movePtr(shm->rPtr);
    }
    //占线读内存
    shm->busy[shm->rPtr] = true;

    //要求写进程要先启动一段时间
    if(shm->data[shm->rPtr] == NULL)
    {
        releaseSharedMemory();
        fprintf(stderr, "[%s line:%d] pop data failed\n", __FILE__, __LINE__);
        shm->busy[shm->rPtr] = false;//释放占线标志，退出读进程
        throw shm->data[shm->rPtr];
    }
    memcpy(this->buffer, shm->data[shm->rPtr], this->nbytes);
    shm->busy[shm->rPtr] = false;
    movePtr(shm->rPtr);

    return this->buffer;

}

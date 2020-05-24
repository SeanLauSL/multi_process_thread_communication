#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <pthread.h>
#include "shm_common.h"


int main()
{
    size_t dataSZ = width * height * sizeof(unsigned char);
    /*IPC_CREAT表示在key标识的共享内存不存在时，创建共享内存*/
    int shmid = shmget((key_t)1234, sizeof(struct shareData), 0666 | IPC_CREAT);
    if (shmid == -1){
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    void *shared_memory = (void *)0;
    /*将共享内存附加到本进程,调用成功时返回一个指向共享内存第一个字节的指针*/
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1){
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Memory attached at %X\n", (long)shared_memory);

    struct shareData *shared_stuff;
    shared_stuff = (struct shareData *)shared_memory;
    pthread_rwlockattr_t rwlockattr;//读写锁进程共享属性
    pthread_rwlockattr_init(&rwlockattr);//初始化读写锁进程共享属性
    pthread_rwlockattr_setpshared(&rwlockattr, PTHREAD_PROCESS_SHARED);//设置读写锁进程共享属性
    pthread_rwlockattr_setkind_np(&rwlockattr,PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);//设置写优先，否则默认读优先PTHREAD_RWLOCK_PREFER_READER_NP
    pthread_rwlock_init(&(shared_stuff->rwlock), &rwlockattr);//初始化读写锁
    pthread_rwlockattr_destroy(&rwlockattr);//属性已经使用，可以销毁

    //char buffer[BUFSIZ];
    char* buffer = (char*)malloc(dataSZ);
    while(true)
    {
        pthread_rwlock_wrlock(&(shared_stuff->rwlock));
        printf("Enter some text: ");
        fflush(stdin);
        fgets(buffer, BUFSIZ, stdin);
        //strncpy(shared_stuff->data, buffer, TEXT_SZ);
        memcpy(shared_stuff->data, buffer, dataSZ);
        pthread_rwlock_unlock(&(shared_stuff->rwlock));
        usleep(20*1000);

        if (strncmp(buffer, "end", 3) == 0) 
        {
            break;
        }
    }

    pthread_rwlock_destroy(&(shared_stuff->rwlock));
    
    // 分离
    if (shmdt(shared_memory) == -1) 
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    // 删除共享内存
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        printf("shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);//exit(1)??
    }
    free(buffer);
    exit(EXIT_SUCCESS);

}
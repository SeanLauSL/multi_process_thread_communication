#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <pthread.h>
#include "shm_common.h"
#include <iostream>

int main()
{
    srand((unsigned int)getpid());
    int shmid = shmget((key_t)1234, sizeof(struct shareData), 0666 | IPC_CREAT);
    if (shmid == -1) {
      fprintf(stderr, "shmget failed\n");
      exit(EXIT_FAILURE);
    }
    void *shared_memory = (void *)0;
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
      fprintf(stderr, "shmat failed\n");
      exit(EXIT_FAILURE);
    }
    printf("Memory attached at %X\n", (long)shared_memory);
    struct shareData *shared_stuff;
    shared_stuff = (struct shareData *)shared_memory;

    int running = 1;
    while(running)
    {
        pthread_rwlock_rdlock(&(shared_stuff->rwlock));//读
        //fflush(stdout);
        printf("You wrote: %s", shared_stuff->data);
#if 0
        /*
        添加这块代码后，开启多个读进程，可以看到读进程是共享的，
        但是由于这里锁内部的耗时过长，如果打开多个读进程，
        很容易导致读锁长时间占线，写锁一直等待。
        (这里有问题，读写锁应该在写锁请求之后不再接受其他锁的请求)
        通过设置写优先属性解决：PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP
        */
        int rt = rand() % 10;
        while(rt)
        {
            fprintf(stderr, "sleep: %d", rt);
            rt--;
            sleep(1);
        }
        printf("\n");
#endif
        pthread_rwlock_unlock(&(shared_stuff->rwlock));
        usleep(1);

        if (strncmp(shared_stuff->data, "end", 3) == 0) 
        {
            running = 0;
        }

        usleep(40*1000);//因为while里没有其他内容
    }

    //分离
    if (shmdt(shared_memory) == -1)
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}
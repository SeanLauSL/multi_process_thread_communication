#ifndef __FIFO_FILE_H__
#define __FIFO_FILE_H__

//#include "opencv2/opencv.hpp"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>

class FIFOIO
{
    public:
    FIFOIO();
    FIFOIO(const char* p_fifo_name, int mode);
    ~FIFOIO();
    /*
    设置管道文件名
    */
    void setFIFOName(const char* p_fifo_name);

    /*
    设置数据尺寸：长、宽、每个元素的字节数
    */
    void setDataSize(unsigned int width, unsigned int height, size_t perbyte);

    /*
    @brief 打开管道文件
    @param mode: 0-read, 1-write
    */
    void openFIFOFile(int mode);

    /*
    关闭管道文件
    */
    void closeFIFOFile();

    /*
    @brief 压入管道数据
    @param data 输入数据
    @return 压入数据bytes
    */
    size_t push_back(unsigned char* data);

    /*
    弹出管道数据，先进先出
    */
    char* pop_front();


    int readFIFOTest();
    int writeFIFOTest();
    
    public: 
    int pipe_fd = -1;
    char fifo_name[1000];
    //char* fifo_name = "/home/jc-acc/pipeline_file_queue/tmp/imgFIFO";
    //data size
    unsigned int data_width = 0;
    unsigned int data_height = 0;
    //size_t bytes_per_element = 0;
    size_t nbytes = 0;//管道目标数据总字节数

    int rec_bytes = -1;//读取到的管道数据大小
    char* buffer;//管道目标数据

    public:

    enum Mode
    {
        READ        = 0, //!< value, open the file for reading
        WRITE       = 1, //!< value, open the file for writing
    };
};

#endif
#include "fifofile.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include "common.h"
int main()
{
    //fifo_land
    FIFOIO fifo_land("/home/jc-acc/pipeline_file_queue/tmp/FIFO_land", FIFOIO::WRITE);
    //fifo_land.writeFIFOTest();
#if TEST_IMAGE
    fifo_land.setDataSize(640, 480, 3*sizeof(unsigned char));
#elif TEST_FRESH_DATA
    fifo_land.setDataSize(1, 1, sizeof(int));
#endif
    //fcntl(fifo_land.pipe_fd, F_SETFL, O_NONBLOCK);//设置为非阻塞(数据大于64k不能使用这个)
/*
    //fifo_line
    FIFOIO fifo_line("/home/jc-acc/pipeline_file_queue/tmp/FIFO_line", FIFOIO::WRITE);
    fifo_line.setDataSize(640, 480, 3*sizeof(unsigned char));
    */

#if TEST_IMAGE
    cv::VideoCapture cap(0);
    if(!cap.isOpened())
    {
        std::cout<<"Failed to open camera."<<std::endl;
        return -1;
    }
    cv::Mat frame;
    cap >> frame;
    if(NULL == frame.data)
    {
        std::cout<<"Failed to capture images."<<std::endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
#endif

    int i = 0;
    char itc[1000];
    while(1)
    {
#if TEST_IMAGE
        cap >> frame;
        fifo_land.push_back(frame.data);
        //fifo_line.push_back(frame.data);

#elif TEST_FRESH_DATA
        i++;
        sprintf(itc, "%d", i);
        unsigned char* itcn = reinterpret_cast< unsigned char* >(itc);
        fifo_land.push_back(itcn);
#endif

        //usleep(2*1000*1000);//2s
    }

    return 0;
}
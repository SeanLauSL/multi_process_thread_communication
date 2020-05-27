#include "shmcirqueue.h"
#include "opencv2/opencv.hpp"
#include <chrono>

int main()
{
    ShmCirQueue shmcq((key_t)1234, ShmCirQueue::WRITE);
    //shmcq.setDataSize(640, 480, 3, sizeof(unsigned char));
    shmcq.setWrFps(1000);

    cv::VideoCapture cap(0);
    if(!cap.isOpened())
    {
        std::cout<<"Failed to open camera."<<std::endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    cv::Mat frame;
    cap >> frame;
    if(NULL == frame.data)
    {
        std::cout<<"Failed to capture images."<<std::endl;
        return -1;
    }
    int nframe = 0;
    while(1)
    {
        auto startLoop = std::chrono::high_resolution_clock::now();

        if(nframe <= DATA_NUM)
        {
            //nframe++;
            if(++nframe == DATA_NUM+1)
            {
                fprintf(stderr, "shared memory circle queue is Ready.");
                //shmcq.test();
            }
        }
        cap >> frame;
        shmcq.push((char*)frame.data);

        auto endLoop = std::chrono::high_resolution_clock::now();
        auto duration_dis = std::chrono::duration_cast<std::chrono::nanoseconds>(endLoop - startLoop);
        float oneloop = duration_dis.count() * 1.0e-6;//时间间隔ms
        std::cout<<"time: "<<oneloop<<std::endl;
    }
    if(cap.isOpened())
        cap.release();
    
    return 0;
}
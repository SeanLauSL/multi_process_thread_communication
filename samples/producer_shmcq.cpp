#include "shmcirqueue.h"
#include "opencv2/opencv.hpp"
#include <chrono>

int main()
{
    int width = 640;
    int height = 480;
    int nchannel = 3;
    ShmCirQueue shmcq((key_t)1234, ShmCirQueue::WRITE);
    shmcq.setDataSize(width, height, nchannel, sizeof(unsigned char));

    cv::VideoCapture cap(0);
    if(!cap.isOpened())
    {
        std::cout<<"Failed to open camera."<<std::endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    //cap.set(cv::CAP_PROP_FPS, 30);//一般usb相机帧率，调大无法获取图像
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
                fprintf(stderr, "shared memory circle queue is Ready.\n");
                //shmcq.test();
            }
        }
        cap >> frame;
        //auto startLoop = std::chrono::high_resolution_clock::now();
        shmcq.push((char*)frame.data);

        auto endLoop = std::chrono::high_resolution_clock::now();
        auto duration_dis = std::chrono::duration_cast<std::chrono::nanoseconds>(endLoop - startLoop);
        float oneloop = duration_dis.count() * 1.0e-6;//时间间隔ms
        std::cout<<"time: "<<oneloop<<std::endl;
        usleep(1000 * 10);

    }
    if(cap.isOpened())
        cap.release();
    
    return 0;
}
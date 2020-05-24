#include "shmcirqueue.h"
#include "opencv2/opencv.hpp"

int main()
{
    ShmCirQueue shmcq((key_t)1234, ShmCirQueue::WRITE);
    //shmcq.setDataSize(640, 480, 3, sizeof(unsigned char));
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
        if(nframe < DATA_NUM)
        {
            //nframe++;
            if(++nframe == DATA_NUM)
                fprintf(stderr, "shared memory circle queue is Ready.");
        }
        cap >> frame;
        shmcq.push((char*)frame.data);

    }
    if(cap.isOpened())
        cap.release();
    
    return 0;
}
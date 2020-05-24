#include "shmcirqueue.h"
#include "opencv2/opencv.hpp"
#include <chrono>

int main()
{
    ShmCirQueue shmcq((key_t)1234, ShmCirQueue::READ);
    cv::Mat frame(480, 640, CV_8UC3);

    int key = 0;
    fprintf(stderr, "[%s line:%d].\n", __FILE__, __LINE__);
    while(1)
    {
        auto startLoop = std::chrono::high_resolution_clock::now();

        char* buffer = shmcq.pop();
        memcpy(frame.data, buffer, shmcq.nbytes);

        auto endLoop = std::chrono::high_resolution_clock::now();
        auto duration_dis = std::chrono::duration_cast<std::chrono::nanoseconds>(endLoop - startLoop);
        float oneloop = duration_dis.count() * 1.0e-6;//时间间隔ms
        std::cout<<"time: "<<oneloop<<std::endl;

        //cv::imshow("shmcq", frame);
        //key = cv::waitKey(10);
        if(key == 'q' || key == 'Q')
			break;

        usleep(40*1000);

    }
    return 0;
}
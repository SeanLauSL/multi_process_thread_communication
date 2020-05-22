#include "fifofile.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include <chrono>
#include "common.h"
int main()
{
    //fifo
    FIFOIO fifo("/home/jc-acc/pipeline_file_queue/tmp/FIFO_land", FIFOIO::READ);
    //fifo.writeFIFOTest();
#if TEST_IMAGE
    fifo.setDataSize(640, 480, 3*sizeof(unsigned char));
    cv::Mat frame(480, 640, CV_8UC3);
    cv::Mat last;
#elif TEST_FRESH_DATA
    fifo.setDataSize(1, 1, sizeof(int));
#endif
    int key = 0;
    while(1)
    {
        auto startLoop = std::chrono::high_resolution_clock::now();

        //read
        char* buffer_land = fifo.pop_front();
        std::cout<<"rec: "<<fifo.rec_bytes<<std::endl;
        if(fifo.rec_bytes != fifo.nbytes)
        {
            fprintf(stderr, "[%s line:%d] data is broken.\n", __FILE__, __LINE__);
            throw fifo.rec_bytes;
        }
#if TEST_IMAGE
        memcpy(frame.data, buffer_land, fifo.nbytes);
#endif

        auto endLoop = std::chrono::high_resolution_clock::now();
        auto duration_dis = std::chrono::duration_cast<std::chrono::nanoseconds>(endLoop - startLoop);
        float oneloop = duration_dis.count() * 1.0e-6;//时间间隔ms
        std::cout<<"time: "<<oneloop<<std::endl;

#if TEST_IMAGE
        if(last.data)
        {
            cv::Scalar dif = cv::sum(frame - last);
            std::cout<<"dif: "<<dif[0]<<std::endl;
        }
        last = frame.clone();
        
#elif TEST_FRESH_DATA
        std::cout<<"val: "<<buffer_land<<std::endl;
#endif
        cv::imshow("fifo", frame);
        key = cv::waitKey(10);
        if(key == 'q' || key == 'Q')
			break;
        //usleep(2*1000*1000);
    }

    return 0;
}
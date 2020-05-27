#include "capThread.h"
#include <iostream>
#include <chrono>

int main()
{
	cv::VideoCapture* cap = new cv::VideoCapture(0);
	cap->set(3, 640);
	cap->set(4, 480);
	CaptureQueue* imgQueue = new CaptureQueue(cap);
	imgQueue->setQueueLength(5);//set length of queue
	imgQueue->setQueueFps(50);//set fps of queue
	imgQueue->run();
	cout<<"image queue runnig."<<endl;
	cv::Mat srcImg;
	int key = 0;
	int wt = 1;
	cv::Mat last;
	while(1)
	{
		std::cout<<__LINE__<<" queue size: "<<imgQueue->size()<<std::endl;
		auto startLoop = std::chrono::high_resolution_clock::now();

		srcImg = imgQueue->pop_front();

		auto endLoop = std::chrono::high_resolution_clock::now();
		auto duration_dis = std::chrono::duration_cast<std::chrono::nanoseconds>(endLoop - startLoop);
		float oneloop = duration_dis.count() * 1.0e-6;//时间间隔ms
		std::cout<<__LINE__<<" cap time: "<<oneloop<<std::endl;
		if(last.data)
		{
			cv::Scalar diff = cv::sum(srcImg - last);
			std::cout<<__LINE__<<" diff: "<<diff[0]<<std::endl;
			if(diff[0] == 0)
				wt = 0;
			else
				wt = 1;
		}
		//usleep(50*1000);//50ms
		//cv::imshow("te", srcImg);
		//key = cv::waitKey(wt);

		last = srcImg.clone();

		
		if(key == 'q' || key == 'Q')
			break;

	}
	if(cap->isOpened())
		cap->release();
	delete imgQueue;
	return 0;
}

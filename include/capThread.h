#pragma once
#include<opencv2/opencv.hpp>
using namespace std;
//using namespace cv;

#ifdef _WIN32
#include<windows.h>
#include<process.h>
#else
#include <unistd.h>
#endif

class CaptureQueue
{
public:
	CaptureQueue();
	CaptureQueue(cv::VideoCapture *cap);
	void setQueueLength(int _length);//set length of queue
	void setQueueFps(int _fps);//set fps of queue
	void run();//run 
	cv::Mat pop_front();
	int size();//return queue size


	~CaptureQueue();


private:
	//unsigned int __stdcall producer(void* para);

	//unsigned int __stdcall consumer(void* para);//test for reading queue
	void* consumer(void* para);//test for reading queue

public:
#ifdef _WIN32
	HANDLE tid1;
	CRITICAL_SECTION g_cs;
#else
	pthread_t imgCapThreadPt;
	pthread_mutex_t f_mutex = PTHREAD_MUTEX_INITIALIZER;//互斥量
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;//条件变量
#endif
	

	cv::VideoCapture *cap;

	std::queue<cv::Mat> myQ;
	int queueLength = 10;
	int queueFps = 50;
	int sz = 0;//queue size

};


	void* producer(void* para);





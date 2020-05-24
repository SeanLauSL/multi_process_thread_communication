#include "capThread.h"
#include<iostream>
#include<queue>
#include<stdlib.h>
#include <chrono>



CaptureQueue::CaptureQueue()
{
}

CaptureQueue::CaptureQueue(cv::VideoCapture *cap)
{
	this->cap = cap;

}

CaptureQueue::~CaptureQueue()
{
	
#ifdef _WIN32
	CloseHandle(tid1);
	//CloseHandle(tid2);
	DeleteCriticalSection(&g_cs);
#else
	pthread_cancel(imgCapThreadPt);
	pthread_mutex_destroy(&f_mutex); //销毁锁
#endif

	if(this->cap->isOpened())
		this->cap->release();

}

void CaptureQueue::setQueueLength(int _length)
{
	this->queueLength = _length;
}

void CaptureQueue::setQueueFps(int _fps)
{
	this->queueFps = _fps;
}


void* producer(void* para)
{
	
	CaptureQueue* cq = (CaptureQueue*)para;
	int i = 0;
	cv::Mat frame;
	while (1)
	{
		auto startLoop = std::chrono::high_resolution_clock::now();
		
		(*(cq->cap)) >> frame;

		auto endLoop = std::chrono::high_resolution_clock::now();
		auto duration_dis = std::chrono::duration_cast<std::chrono::nanoseconds>(endLoop - startLoop);
		float oneloop = duration_dis.count() * 1.0e-6;//时间间隔ms
		std::cout<<__LINE__<<" produce time: "<<oneloop<<"sz:"<<cq->sz<<std::endl;

#ifdef _WIN32
		EnterCriticalSection(&(cq->g_cs));
		(cq->myQ).push(frame.clone());//left
		LeaveCriticalSection(&(cq->g_cs));
		Sleep(2);
#else
		pthread_mutex_lock(&(cq->f_mutex));//上锁
		(cq->myQ).push(frame.clone());//left
		(cq->sz)++;
		if(cq->sz == 1)
			pthread_cond_signal(&(cq->cond));//将等待在该条件变量上的一个线程唤醒
		pthread_mutex_unlock(&(cq->f_mutex));//解锁
		usleep(1000 * 1000 / cq->queueFps);
#endif
		//ensure the data in queue are fresh enough
		if((cq->myQ).size() >= cq->queueLength)
		{

			pthread_mutex_lock(&(cq->f_mutex));//上锁
			while ((cq->myQ).size() - 1)
			{
				(cq->myQ).pop();
				(cq->sz)--;
			}
			pthread_mutex_unlock(&(cq->f_mutex));//解锁
			usleep(1000 * 2);
		}
		
		
	}
}

cv::Mat CaptureQueue::pop_front()
{
	cv::Mat frame;
#ifdef _WIN32
	while (myQ.empty())
		Sleep(2);

	EnterCriticalSection(&g_cs);
	//cout << myQ.front()<<"  ";
	frame = myQ.front();//right
	myQ.pop();

	LeaveCriticalSection(&g_cs);
	Sleep(2);
#else

	//cout<<__LINE__<<" queue size: "<<myQ.size()<<endl;
	pthread_mutex_lock(&f_mutex);//上锁
	while (myQ.empty())
	{
		pthread_cond_wait(&cond, &f_mutex);
        //usleep(1000*2);
	}
	frame = myQ.front();//right
	myQ.pop();
	(this->sz)--;
	pthread_mutex_unlock(&f_mutex);//解锁
	//usleep(1000 * 2);

#endif

	return frame;
}

void* CaptureQueue::consumer(void* para)
{
	cv::Mat frame;
	while (1)
	{
		frame = this->pop_front();
		usleep(1000 * 2);//腾空闲给写线程

		cv::imshow("frame", frame);
		cv::waitKey(1);
	}


}

void CaptureQueue::run()
{
#ifdef _WIN32
	//初始化关键段
	InitializeCriticalSection(&(this->g_cs));

	this->tid1 = (HANDLE)_beginthreadex(NULL, 0, this->producer, NULL, 0, NULL);
#else
	pthread_create(&(this->imgCapThreadPt), NULL, producer, (void*)this);
#endif
}


int CaptureQueue::size()
{
	return this->sz;
}
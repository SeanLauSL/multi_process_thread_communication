#include <iostream>
#include "deque.hpp"
#include "opencv2/opencv.hpp"

using namespace std;

class Human {
public:
	Human(int age = 5)
		: mage(age)
	{
		this->name = new char[10];
		cout << "Constructor with age！" << endl;
	}

	Human(int age, const char* name) {
		mage = age;
		//this->name = name;
		this->name = new char[10];
		strcpy(this->name, name);
		cout << "Constructor with parameters！" << endl;
	}

	Human(const Human& human)
		:mage(human.mage)
	{
		//this->name = human.name;
		this->name = new char[10];
		strcpy(this->name, human.name);
		cout << "copy Constructor！" << endl;
	}

	Human& operator=(const Human &src)
	{
		cout << "operator=" << endl;
		if (this == &src)
			return *this;

		delete[]name;

		mage = src.mage;
		name = new char[10];
		strcpy(this->name, src.name);

		return *this;
	}

	Human(Human&& human)
		:mage(human.mage)
	{
		/*此处没有重新开辟内存拷贝数据，把human的资源直接给当前对象，再把human置空*/
		this->name = human.name;
		human.name = nullptr;
		cout << "move Constructor！" << endl;
	}

	Human& operator=(Human&& src)
	{
		cout << "operator= (Human&&)" << endl;
		if (this == &src)
			return *this;

		delete name;

		mage = src.mage;
		/*此处没有重新开辟内存拷贝数据，把src的资源直接给当前对象，再把src置空*/
		name = src.name;
		src.name = nullptr;

		return *this;
	}

	~Human()
	{
		cout << "~Stack()" << endl;
		delete[] name;
		name = nullptr;
	};

	void print(void) {
		cout << "age：" << mage << " name：" << name << endl;
	}

	int getAge()
	{
		return mage;
	}

private:
	int mage;
	char* name;
};


int main()
{
    //std::string
	TQueueConcurrent<std::string> dequeTS;//deque_thread_save
	std::string t0 = "t0";
	std::string t1 = "t1";
	dequeTS.emplace_back("tt");
	dequeTS.emplace_back(t0);
	dequeTS.emplace_back(t1);
	dequeTS.insert(dequeTS.locate(2), "loc");
	std::cout << "dequeTS size: " << dequeTS.size() << std::endl;
	while (dequeTS.size())
	{
		std::cout << "dequeTS val: " << dequeTS.pop_front() << std::endl;
	}

	//cv::Mat
	TQueueConcurrent<cv::Mat> fifo_mat;
	cv::Mat mt = cv::Mat::ones(3, 3, CV_8UC1);
	fifo_mat.emplace_back(mt);
	std::cout << "fifo_mat size: " << fifo_mat.size() << std::endl;
	std::cout << "fifo_mat val: "<< std::endl << fifo_mat.pop_front() << std::endl;
	
	//class 多参数
	TQueueConcurrent<Human> dequeSN;
	const char* name = "man";
	dequeSN.emplace_back(10, name);//call Human(10, "man");
	std::cout << "dequeSN size: " << dequeSN.size() << std::endl;
	while (dequeSN.size())
	{
		dequeSN.pop_front().print();
	}

	return 0;	
}
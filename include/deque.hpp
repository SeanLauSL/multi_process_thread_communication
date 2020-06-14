#ifndef DEQUE_H
#define DEQUE_H
#include <deque>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <initializer_list>
#include <chrono>

template< typename T >
class TQueueConcurrent {
	using const_iterator = typename std::deque<T>::const_iterator;
	using iterator = typename std::deque<T>::iterator;
public:
	//! \brief Emplaces a new instance of T in front of the deque
	template<typename... Args>
	void emplace_front(Args&&... args) //input elements of a class, while T is a class name.
	{
		//lambda expression
		addData_protected([&] {
			_collection.emplace_front(std::forward<Args>(args)...);  //std::deque<T>::emplace_front()
		});
	}

	//! \brief Emplaces a new instance of T at the back of the deque
	template<typename... Args>
	void emplace_back(Args&&... args) 
	{
		addData_protected([&] {
			_collection.emplace_back(std::forward<Args>(args)...);  //std::deque<T>::emplace_back()
		});
	}

	//! \brief Emplaces a new instance of T at any place of the deque
	//! \param First param is a iterator
	//! \note should use std::list<T>::emplace() for instead
	template<typename... Args>
	void insert(const_iterator itc, Args&&... args)
	{
		addData_protected([&] {
			//should use std::list<T>::emplace() for instead
			_collection.emplace(itc, std::forward<Args>(args)...);  //std::deque<T>::emplace()
		});
	}
	const_iterator locate(unsigned int label)
	{
		return (this->_collection.begin() + label);
	}

	//! \brief Returns the front element and removes it from the collection
	//!
	//!        No exception is ever returned as we garanty that the deque 
	//is not empty
	//!        before trying to return data.
	T pop_front(void) noexcept 
	{
		std::unique_lock<std::mutex> lock{ _mutex };
		while (_collection.empty()) {
			_condNewData.wait(lock);
		}
		auto elem = std::move(_collection.front());
		_collection.pop_front();   // std::deque<T>::pop_front()
		return elem;
	}
	T pop_front_not_wait(void) noexcept 
	{
		std::unique_lock<std::mutex> lock{ _mutex };
		auto elem = std::move(_collection.front());
		_collection.pop_front();
		lock.unlock();
		return elem;
	}
	bool is_empty() {
		std::unique_lock<std::mutex> lock{ _mutex };
		bool is_empty = _collection.empty();
		lock.unlock();
		return is_empty;
	}
	unsigned int size() {
		std::unique_lock<std::mutex> lock{ _mutex };
		unsigned int size = _collection.size();
		lock.unlock();
		return size;
	}

private:

	//! \brief Protects the deque, calls the provided function and 
	//notifies the presence of new data
	//! \param The concrete operation to be used. It MUST be an operation 
	//which will add data to the deque,
	//!        as it will notify that new data are available!
	template<class F>
	void addData_protected(F&& fct) //添加参数
	{
		std::unique_lock<std::mutex> lock{ _mutex };
		fct();
		//initializer_list<T>{(fct(), 0)...};
		lock.unlock();
		_condNewData.notify_one();
	}

	std::deque<T> _collection;  ///< Concrete, not thread safe, storage.
	std::mutex   _mutex;        ///< Mutex protecting the concrete storage
	std::condition_variable _condNewData;   ///< Condition used to notify 
											///that new data are available.
};

#endif //DEQUE_H


//https://blog.csdn.net/m0_37542524/article/details/93642813
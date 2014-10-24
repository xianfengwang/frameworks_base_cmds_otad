/*
 * MutexQueue.h
 *
 *  Created on: 2014-5-20
 *      Author: manson
 */

#ifndef MUTEXQUEUE_H_
#define MUTEXQUEUE_H_

#include <queue>
#include <pthread.h>

template<class T>
class MutexQueue {
public:
	MutexQueue(unsigned int size);
	virtual ~MutexQueue();

	void Push(T * t);
	bool Pop();
	T * Front();
	int Size();

private:
	std::queue<T*> *m_qInnerQueue;
	pthread_mutex_t m_mutexLock;
	pthread_cond_t m_condFull; //after pop send this cond
	pthread_cond_t m_condEmpty; //after push send this cond
	unsigned int m_nSize;
};

template<class T>
int MutexQueue<T>::Size() {
	return m_qInnerQueue->size();
}

template<class T>
MutexQueue<T>::MutexQueue(unsigned int size) {
	m_qInnerQueue = new std::queue<T*>();
	m_nSize = size;
}

template<class T>
MutexQueue<T>::~MutexQueue() {
	pthread_mutex_lock(&m_mutexLock);
	while (m_qInnerQueue->size() > 0) {
		T* t = m_qInnerQueue->front();
		delete t;
		m_qInnerQueue->pop();
	}
	delete m_qInnerQueue;
	pthread_mutex_unlock(&m_mutexLock);
}

template<class T>
void MutexQueue<T>::Push(T *t) {
	pthread_mutex_lock(&m_mutexLock); //lock

	while (m_qInnerQueue->size() == m_nSize) {
		int ret = pthread_cond_wait(&m_condFull, &m_mutexLock);
		if (ret != 0) {
			pthread_mutex_unlock(&m_mutexLock);
			return;
		}
	}

	m_qInnerQueue->push(t);
	pthread_cond_signal(&m_condEmpty);
	pthread_mutex_unlock(&m_mutexLock); //unlock
}

template<class T>
bool MutexQueue<T>::Pop() {
	pthread_mutex_lock(&m_mutexLock);

	if (m_qInnerQueue->size() == 0) {
		return false; // empty queue cannot pop
	}

	m_qInnerQueue->pop();
	pthread_mutex_unlock(&m_mutexLock);
	return true;
}

template<class T>
T* MutexQueue<T>::Front() {
	T* t = NULL;
	pthread_mutex_lock(&m_mutexLock);

	if (m_qInnerQueue->size() == 0) {
		int ret = pthread_cond_wait(&m_condEmpty, &m_mutexLock);
		if (ret != 0) {
			pthread_mutex_unlock(&m_mutexLock);
			return NULL;
		}
	}

	t = m_qInnerQueue->front();
	pthread_cond_signal(&m_condFull);
	pthread_mutex_unlock(&m_mutexLock);
	return t;
}

#endif /* MUTEXQUEUE_H_ */

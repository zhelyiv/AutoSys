#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <string>
#include <mutex>
#include <atomic>
//#include <condition_variable>

#define LURA_DEF_TH_POOL_SIZE 200
 
template <class T>
class CLuraThPool
{
public:
	CLuraThPool(int nMaxThreads = LURA_DEF_TH_POOL_SIZE);
	virtual ~CLuraThPool();
	  
	bool Initialize();
	std::string GetErr() { return m_err; }

	bool Start(bool bAutoJoin = false);
	void Stop();
	  
	void Enqueue(const T& obj, int nPriority);
	T*   Dequeue();
	bool IsStarted() { return m_bIsStarted; } 

protected: 
	virtual int Process(T& obj) { return 0; };
	virtual void AfterProcess(int nProcessResult) {};

private:
	void Sentry(); 
	T*   Dequeue(std::queue<T>* pQueue); //help method

public: 
	std::atomic<bool> m_bStop; 

private:
	bool m_bInitialized;
	std::atomic<bool> m_bIsStarted; 
	int m_nPoolSize;
	 
	CRITICAL_SECTION m_csQueue;  

	std::string m_err; 
	std::vector<std::thread> m_vSentryPool;

	std::queue<T> m_qObjToProcessPriority1;
	std::queue<T> m_qObjToProcessPriority2;
	std::queue<T> m_qObjToProcessPriority3;
	std::queue<T> m_qObjToProcessPriority4;
};

// CLuraThPool class members implementation
template <class T>
CLuraThPool<T>::CLuraThPool(int nMaxThreads)
	: m_bInitialized(false), m_nPoolSize(nMaxThreads), m_bStop(false), m_bIsStarted(false)
{ 
}
 
template <class T>
CLuraThPool<T>::~CLuraThPool()
{
	DeleteCriticalSection(&m_csQueue);
}

template <class T>
bool CLuraThPool<T>::Initialize() 
{
	m_bInitialized = false;
	if (!InitializeCriticalSectionAndSpinCount(&m_csQueue, 0x00000400))
	{
		m_err = GetLastErrorString(GetLastError());
		return false;
	}

	m_bInitialized = true;
	return m_bInitialized;
}

template <class T>
bool CLuraThPool<T>::Start(bool bAutoJoin)
{
	if (!m_bInitialized || m_bIsStarted || m_nPoolSize <= 0 )
		return false;

	try
	{
		for (int i = 0; i < m_nPoolSize; i++) 
			m_vSentryPool.emplace_back(std::thread(&CLuraThPool::Sentry, this)); 

		if (bAutoJoin)
		{
			for (int i = 0; i < m_nPoolSize; i++)
				m_vSentryPool[i].join();
		}

		m_bIsStarted = true;
	}
	catch (...)
	{
		m_bIsStarted = false;
	}

	return m_bIsStarted;
}
   
template <class T>
void CLuraThPool<T>::Stop()
{
	if(m_bIsStarted) 
		m_bStop = true;
}

template <class T>
void CLuraThPool<T>::Sentry()
{
	while(!m_bStop)
	{ 
		try
		{ 
			T* pQItem = Dequeue();

			if (pQItem) 
				AfterProcess(Process(*pQItem));
		}
		catch (const std::exception& ex)
		{
			const char* pszMessage = ex.what();
		}
		catch (...)
		{
		}
	}
}
 
template <class T>
void CLuraThPool<T>::Enqueue(const T& obj, int nPriority)
{
	EnterCriticalSection(&m_csQueue);

	switch (nPriority)
	{ 
		case 1: m_qObjToProcessPriority1.push(obj); break;
		case 2: m_qObjToProcessPriority2.push(obj); break;
		case 3: m_qObjToProcessPriority3.push(obj); break;
		case 4: m_qObjToProcessPriority4.push(obj); break;

		default: 
			m_qObjToProcessPriority1.push(obj); 
			break;
	}
	 
	LeaveCriticalSection(&m_csQueue); 
} 

 
template <class T>
T*  CLuraThPool<T>::Dequeue(std::queue<T>* pQueue)
{
	T* pQItem = NULL;

	if (!pQueue->empty())
	{
		pQItem = &pQueue->front();
		pQueue->pop();
	}

	return pQItem;
}
 
template <class T>
T*  CLuraThPool<T>::Dequeue()
{
	T* pQItem = NULL;
 
	if (TryEnterCriticalSection(&m_csQueue))
	{ 
		pQItem = Dequeue(&m_qObjToProcessPriority4);  
		
		if(!pQItem)
			pQItem = Dequeue(&m_qObjToProcessPriority3);

		if (!pQItem)
			pQItem = Dequeue(&m_qObjToProcessPriority2);

		if (!pQItem)
			pQItem = Dequeue(&m_qObjToProcessPriority1);
		 
		LeaveCriticalSection(&m_csQueue);
	}
	 
	return pQItem;
}
 
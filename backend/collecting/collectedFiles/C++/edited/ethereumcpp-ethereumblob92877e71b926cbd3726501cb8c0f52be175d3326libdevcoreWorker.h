


#pragma once

#include <string>
#include <thread>
#include <atomic>
#include "Guards.h"

namespace dev
{

enum class IfRunning
{
	Fail,
	Join,
	Detach
};

enum class WorkerState
{
	Starting,
	Started,
	Stopping,
	Stopped,
	Killing
};

class Worker
{
protected:
	Worker(std::string const& _name = "anon", unsigned _idleWaitMs = 30): m_name(_name), m_idleWaitMs(_idleWaitMs) {}

		Worker(Worker&& _m) { std::swap(m_name, _m.m_name); }

		Worker& operator=(Worker&& _m)
	{
		assert(&_m != this);
		std::swap(m_name, _m.m_name);
		return *this;
	}

	virtual ~Worker() { terminate(); }

		void setName(std::string _n) { if (!isWorking()) m_name = _n; }

		void startWorking();
	
		void stopWorking();

		bool isWorking() const { Guard l(x_work); return m_state == WorkerState::Started; }
	
		virtual void startedWorking() {}
	
		virtual void doWork() {}

		virtual void workLoop();
	bool shouldStop() const { return m_state != WorkerState::Started; }
	
		virtual void doneWorking() {}

	
				void terminate();

private:

	std::string m_name;

	unsigned m_idleWaitMs = 0;
	
	mutable Mutex x_work;							std::unique_ptr<std::thread> m_work;		    mutable std::condition_variable m_state_notifier; 	std::atomic<WorkerState> m_state = {WorkerState::Starting};
};

}

#pragma once
#include <map>
#include <utility>
#include <iostream>
#include <list>
#include <sys/wait.h>
#include "IJobManager.h"

struct Job {
	pid_t processID;
	int jobID;
	bool isStopped;
	Job() = delete;
	Job(pid_t pid, int jid, bool stopped) : processID(pid), jobID(jid), isStopped(stopped)
	{}
};

class JobManager : public IJobManager{
	std::map<int, Job> jobMap;
	std::list<int> recycleJobID;
	int nextJobID = -1;
private:
	int getJobID() {
		if (recycleJobID.empty()) {
			return ++nextJobID;
		}
		int jobID = recycleJobID.back();
		recycleJobID.pop_back();
		return jobID;
	}
public:
	void addJob(pid_t processID, bool isStopped)
	{
		int jobID = getJobID();
		Job job(processID, jobID, isStopped);
		jobMap.emplace(jobID, job);
		std::cout << "Adding job:" << jobID << " pid:" << processID << std::endl;
	}

	void modifyJob(int jobID, bool isStopped)
	{
		auto it = jobMap.find(jobID);
		if (it != jobMap.end()) {
			it->second.isStopped = isStopped;
		}
	}

	void removeJob(int jobID)
	{
		jobMap.erase(jobID);
		recycleJobID.push_back(jobID);
	}

	pid_t getProcessID(int jobID)
	{
		auto it = jobMap.find(jobID);
		if (it != jobMap.end()) {
			return it->second.processID;
		}
		return -1;
	}

	void printJobs()
	{
		for (const auto& job : jobMap) {
			std::cout << "JobID: " << job.first << " PID: " << job.second.processID;
			std::cout << " Stopped: " << job.second.isStopped << std::endl;
		}
	}

	void checkJobStatus()
	{
		int status;
		for (const auto& job : jobMap) {
			auto jobID = job.first;
			auto processID = job.second.processID;

			ProcessStatus processStatus = processManager->waitForProcess(processID, WNOHANG | WUNTRACED | WCONTINUED);

			if (processID != processStatus.processID){
				continue;
			}

			if (processStatus.processState == PROCESS_STATE::PROCESS_EXITED) {
				std::cout << "Process " << processID << " exited with status: " << processStatus.processStatus << std::endl;
				removeJob(jobID);
			}
			else if (processStatus.processState == PROCESS_STATE::PROCESS_STOPPED) {
				modifyJob(jobID, true);
			}
			else if (processStatus.processState == PROCESS_STATE::PROCESS_SIGNALED) {
				auto sig = processStatus.processStatus;
				if (sig == SIGKILL || sig == SIGTERM || sig == SIGQUIT || sig == SIGINT) {
					std::cout << "Job " << jobID << " terminated by signal: "
						<< sig << std::endl;
					removeJob(jobID);
				}
			}
			else if (processStatus.processState == PROCESS_STATE::PROCESS_CONTINUED) {
				modifyJob(jobID, false);
			}
		}
	}
};
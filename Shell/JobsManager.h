#pragma once
#include <map>
#include <utility>
#include <iostream>
#include <list>

struct Job {
	int processID;
	int jobID;
	bool isStopped;
	Job() = default;
	Job(int pid, int jid, bool stopped) : processID(pid), jobID(jid), isStopped(stopped) 
	{}
};

class JobManager {
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
	void addJob(int processID, bool isStopped)
	{
		int jobID = getJobID();
		Job job(processID, jobID, isStopped);
		jobMap.emplace(jobID, job);
		std::cout << "Adding job:" << jobID << " pid:" << processID << std::endl;
	}

	void modifyJob(int jobID, bool isStopped)
	{
		Job& job = jobMap[jobID];
		job.isStopped = isStopped;
	}

	void removeJob(int jobID)
	{
		jobMap.erase(jobID);
		recycleJobID.push_back(jobID);
	}

	int getProcessID(int jobID)
	{
		Job& job = jobMap[jobID];
		return job.processID;
	}

	void printJobs()
	{
		for (const auto& job : jobMap) {
			std::cout << "JobID: " << job.first << " PID: " << job.second.processID;
			std::cout << " Stopped: " << job.second.isStopped << std::endl;
		}
	}
};
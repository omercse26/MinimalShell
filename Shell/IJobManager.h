#pragma once
class IJobManager {
public:
	virtual void addJob(pid_t processID, bool isStopped) = 0;

	virtual void modifyJob(int jobID, bool isStopped)=0;

	virtual void removeJob(int jobID)=0;

	virtual pid_t getProcessID(int jobID)=0;

	virtual void printJobs()=0;

	virtual void checkJobStatus()=0;
};
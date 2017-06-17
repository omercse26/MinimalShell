#include <iostream>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <memory>

#include <signal.h>
#include <unistd.h>
#include "JobsManager.h"
#include "LinuxProcessManager.h"
#include "Util.h"

class MinimalShell
{
	std::unique_ptr<IJobManager> jobManager;
	std::unique_ptr<IProcessManager> processManager;

	int isBuiltInCommand(const std::string& command)
	{
		if (command == "fg") {
			return 0;
		}
		else if (command == "bg") {
			return 1;
		}
		else if (command == "kill") {
			return 2;
		}
		else {
			return -1;
		}
	}

	std::vector<std::string> splitCmd(const std::string& command)
	{
		std::vector<std::string> subCmds = Util::splitStringToVector(command);

		// single out the ampersand.
		std::string last = subCmds.back();
		auto index = last.find('&');
		if (index != std::string::npos && last != "&") {
			subCmds.pop_back();
			subCmds.emplace_back(last.substr(0, index));
			subCmds.emplace_back("&");
		}

		return subCmds;
	}

	void waitForProcess(int processID)
	{
		ProcessStatus processStatus = processManager->waitForProcess(processID, WUNTRACED);

		if (processStatus.processState == PROCESS_STATE::PROCESS_EXITED) {
			std::cout << "Process " << processID << " exited with status: " << processStatus.processStatus << std::endl;
		}
		else if (processStatus.processState == PROCESS_STATE::PROCESS_STOPPED) {
			jobManager->addJob(processID, true);
		}
		else if (processStatus.processState == PROCESS_STATE::PROCESS_SIGNALED){
			std::cout << "Process " << processID << " terminated by signal: " << processStatus.processStatus << std::endl;
		}
	}

public:
	MinimalShell(std::unique_ptr<IJobManager> jobManager,
		         std::unique_ptr<IProcessManager> processMgr) : 
		jobManager(std::move(jobManager)), processManager(std::move(processMgr))
	{
		this->jobManager->setProcessManager(processManager.get());
	}

	void checkJobStatus()
	{
		jobManager->checkJobStatus();
	}
	void executeCommand(const std::string& command)
	{
		if (command == "jobs") {
			jobManager->printJobs();
			return;
		}

		std::vector<std::string> subCmds = splitCmd(command);
		/*for (auto& i : subCmds) {
		std::cout << i << std::endl;
		}*/
		int builtinCommand = isBuiltInCommand(subCmds[0]);

		if (builtinCommand != -1) {
			int jobID = atoi(subCmds[1].c_str());
			int processID = jobManager->getProcessID(jobID);
			if (processID == -1) {
				std::cout << "JobID " << jobID << " is invalid" << std::endl;
				return;
			}
			switch (builtinCommand) {
			case 0:
				jobManager->removeJob(jobID);
				processManager->sendSignalToProcess(processID, SIGNAL::SIG_FG);
				waitForProcess(processID);
				break;
			case 1:
				jobManager->modifyJob(jobID, false);
				processManager->sendSignalToProcess(processID, SIGNAL::SIG_BG);
				break;
			case 2:
				processManager->sendSignalToProcess(processID, SIGNAL::SIG_INT);
				break;
			}
		}
		else {
			// Execute the command in a child process.
			ChildProcessInfo childProcessInfo;
			processManager->executeCmdInChildProcess(subCmds, childProcessInfo);
			
			// In Parent process.
			if (!childProcessInfo.isBackGround) {
				waitForProcess(childProcessInfo.childPID);
			}
			else {
				jobManager->addJob(childProcessInfo.childPID, false);
			}
		}
	}
};

int main()
{
	std::string command;
	MinimalShell shell(std::unique_ptr<IJobManager>(new JobManager), 
					   std::unique_ptr<IProcessManager>(new LinuxProcessManager));
	while (true) {
		std::cout << "$:" << std::flush;
		std::getline(std::cin, command);
		if (!command.empty()) {
			shell.executeCommand(command);
		}
		shell.checkJobStatus();
	}
    return 0;
}
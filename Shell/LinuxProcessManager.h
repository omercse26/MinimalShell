#pragma once
#include <vector>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <map>
#include "JobsManager.h"
#include "IProcessManager.h"

class LinuxProcessManager : public IProcessManager
{
	std::map<SIGNAL, int> signalToLinuxSignal = { {SIGNAL::SIG_FG, SIGCONT},
												  {SIGNAL::SIG_BG, SIGCONT},
												  {SIGNAL::SIG_INT,SIGINT}
	};
public:
	ProcessStatus waitForProcess(int processID, int args)
	{
		ProcessStatus processStatus;
		int status;
		processStatus.processID = waitpid(processID, &status, args);

		if (WIFEXITED(status)){
			processStatus.processState = PROCESS_STATE::PROCESS_EXITED;
			processStatus.processStatus = WEXITSTATUS(status);
		}
		else if (WIFSTOPPED(status)) {
			processStatus.processState = PROCESS_STATE::PROCESS_STOPPED;
			processStatus.processStatus = -1;
		}
		else if (WIFSIGNALED(status)) {
			processStatus.processState = PROCESS_STATE::PROCESS_SIGNALED;
			processStatus.processStatus = WTERMSIG(status);
		}
		else if (WIFCONTINUED(status)) {
			processStatus.processState = PROCESS_STATE::PROCESS_CONTINUED;
			processStatus.processStatus = -1;
		}
		return processStatus;
	}
	void sendSignalToProcess(int processID, SIGNAL sig) 
	{
		kill(processID, signalToLinuxSignal[sig]);
	}
	void fillExecveArgs(std::vector<std::string>& subCmds, char** argv, bool bg)
	{
		int i = 0;
		for (auto& subcmd : subCmds) {
			argv[i++] = &subcmd[0];
		}

		argv[bg ? (i - 1) : i] = NULL;
	}
	void executeCmdInChildProcess(std::vector<std::string>& vCmds, ChildProcessInfo& childProcessInfo)
	{
		// Execute the command in a child process.
		std::string amp = vCmds.back();
		bool backgroundProcess = (amp == "&");
		pid_t childPID = fork();
		if (childPID == 0) {
			// In Child Process
			char** argv = new char*[vCmds.size() + 1];
			char* env[] = { NULL };
			fillExecveArgs(vCmds, argv, backgroundProcess);
			execve(argv[0], argv, env);
			std::cout << "Error Executing the command " << argv[0] << std::endl;
			delete[] argv;
			exit(-1);
		}
		childProcessInfo.childPID = childPID;
		childProcessInfo.isBackGround = backgroundProcess;
	}
};

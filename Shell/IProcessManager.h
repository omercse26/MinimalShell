#pragma once
#include <vector>
#include <string>
enum class SIGNAL {
	SIG_FG,
	SIG_BG,
	SIG_INT
};

enum class PROCESS_STATE {
	PROCESS_NONE,
	PROCESS_EXITED,
	PROCESS_STOPPED,
	PROCESS_SIGNALED,
	PROCESS_CONTINUED
};

struct ProcessStatus {
	PROCESS_STATE processState;
	int processStatus;
	int processID;

	ProcessStatus() 
	{
		processState = PROCESS_STATE::PROCESS_NONE;
	}
};

struct ChildProcessInfo {
	int childPID;
	bool isBackGround;
};

class IProcessManager
{
public:
	virtual ProcessStatus waitForProcess(int processID, int args)=0;
	virtual void sendSignalToProcess(int processID, SIGNAL) = 0;
	virtual void executeCmdInChildProcess(std::vector<std::string>& vCmds, ChildProcessInfo&) =0;
};

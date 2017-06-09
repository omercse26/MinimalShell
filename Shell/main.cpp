#include <iostream>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include "JobsManager.h"

class MinimalShell
{
	JobManager jobManager;

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
		std::vector<std::string> subCmds;
		size_t i = 0;

		while (i < command.size()) {
			while (i < command.size() && std::isspace(command[i])) { ++i; }
			auto j = i;
			std::string temp;
			while (i < command.size() && !std::isspace(command[i])) {
				temp.append(1, command[i++]);
			}

			if (i > j) {
				subCmds.push_back(temp);
			}
		}

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
		int status;
		waitpid(processID, &status, WUNTRACED);

		if (WIFEXITED(status)) {
			std::cout << "Process " << processID << " exited with status: " << WEXITSTATUS(status) << std::endl;
		}
		else if (WIFSTOPPED(status)) {
			jobManager.addJob(processID, true);
		}
		else if (WIFSIGNALED(status)){
			std::cout << "Process " << processID << " terminated by signal: " << WTERMSIG(status) << std::endl;
		}
	}

	void fillExecveArgs(std::vector<std::string>& subCmds, char** argv, bool bg)
	{
		int i = 0;
		for (auto& subcmd : subCmds) {
			argv[i++] = &subcmd[0];
		}

		argv[bg? (i - 1): i] = NULL;
	}

public:
	void checkJobStatus()
	{
		jobManager.checkJobStatus();
	}
	void executeCommand(const std::string& command)
	{
		if (command == "jobs") {
			jobManager.printJobs();
			return;
		}

		std::vector<std::string> subCmds = splitCmd(command);
		/*for (auto& i : subCmds) {
		std::cout << i << std::endl;
		}*/
		int builtinCommand = isBuiltInCommand(subCmds[0]);

		if (builtinCommand != -1) {
			int jobID = atoi(subCmds[1].c_str());
			int processID = jobManager.getProcessID(jobID);
			if (processID == -1) {
				std::cout << "JobID " << jobID << " is invalid" << std::endl;
			}
			switch (builtinCommand) {
			case 0:
				std::cout << "fg" << std::endl;
				jobManager.removeJob(jobID);
				kill(processID, SIGCONT);
				waitForProcess(processID);
				break;
			case 1:
				std::cout << "bg" << std::endl;
				jobManager.modifyJob(jobID, false);
				kill(processID, SIGCONT);
				break;
			case 2:
				std::cout << "kill" << std::endl;
				kill(processID, SIGINT);
				break;
			}
		}
		else {
			// Execute the command in a child process.
			bool backgroundProcess = (subCmds.back() == "&");
			pid_t childPID = fork();
			if (childPID == 0)	{
				// In Child Process
				char** argv = new char*[subCmds.size()+1];
				char* env[] = { NULL };
				fillExecveArgs(subCmds, argv, backgroundProcess);
				execve(argv[0], argv, env);
				std::cout << "Error Executing the command " << argv[0] << std::endl;
				delete[] argv;
				exit(-1);
			}
			else  {
				// In Parent process.
				if (!backgroundProcess) {
					waitForProcess(childPID);
				}
				else {
					jobManager.addJob(childPID, false);
				}
			}
		}
	}
};

int main()
{
	std::string command;
	MinimalShell shell;
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
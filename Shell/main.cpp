#include <cstdio>
#include <iostream>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <signal.h>
#include <sys/wait.h>
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
		int status; pid_t tid;
		tid = waitpid(processID, &status, WUNTRACED);

		if (WIFEXITED(status)) {
			printf("Process %d exited with status:%d\n", processID, WEXITSTATUS(status));
		}
		else if (WIFSTOPPED(status)) {
			jobManager.addJob(processID, 1);
		}
		else if (WIFSIGNALED(status)){
			printf("Process %d terminated by signal:%d\n", processID, WTERMSIG(status));
		}
	}

	void checkJobStatus()
	{
		int i, status, sig;
		pid_t  p;
		for (i = 0; i <= njobs; ++i)
		{
			status = 0;
			if (jobs[i].pid != -1)
			{
				p = waitpid(jobs[i].pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
				if (p == jobs[i].pid)
				{
					if (WIFEXITED(status))
					{
						printf("Job %d exited with status:%d\n", jobs[i].jid, WEXITSTATUS(status));
						removejob(jobs[i].jid);
					}
					else if (WIFSTOPPED(status))
					{
						modifyjob(jobs[i].jid, 1);
					}
					else if (WIFSIGNALED(status))
					{
						sig = WTERMSIG(status);
						if (sig == SIGKILL || sig == SIGTERM || sig == SIGQUIT || sig == SIGINT) {
							printf("Job %d terminated by signal %d\n", jobs[i].jid, sig);
							removejob(jobs[i].jid);
						}
					}
					else if (WIFCONTINUED(status))
					{
						modifyjob(jobs[i].jid, 0);
					}
				}
			}
		}
	}

	void getfilename(const string& command, char** argv)
	{
		char* delim = " \t\n";
		int i = 0, dist;
		char *ind;
		char *tok = strtok(command, delim);
		argv[i++] = tok;
		while (1)
		{
			tok = strtok(NULL, delim);
			if (tok == NULL) break;
			argv[i++] = tok;
		}
		ind = strchr(argv[i - 1], '&');
		if (ind != NULL)
		{
			dist = ind - argv[i - 1];
			argv[i - 1] = strndup(argv[i - 1], dist);
		}
		if (argv[i - 1][0] == '\0')
			argv[i - 1] = NULL;
		else
			argv[i] = NULL;
	}

public:
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
			bool backgroundProcess = (subCmds.back() == "&");
			pid_t childPID = fork();
			if (childPID == 0)
			{
				char** argv = (char**)malloc(20 * sizeof(char*));
				getFileName(command, argv);
				execve(argv[0], argv, env);
				printf("Error executing the command %s\n", duplicate);
				exit(-1);
			}
			else
			{
				if (bg == NULL) {
					waitforprocess(childPID);
				}
				else {
					addjob(childPID, 0);
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
		shell.executeCommand(command);
	}
    return 0;
}
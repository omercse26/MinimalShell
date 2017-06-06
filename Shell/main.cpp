#include <cstdio>
#include <iostream>
#include <vector>

void printJobs() 
{
	std::cout << "Printed Jobs" << std::endl;
}

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
	size_t start = 0;
	while (true) {
		auto index = command.find(' ', start);
		if (index == std::string::npos) {
			std::string last = command.substr(start);
			auto amp = last.find('&');
			if (amp != std::string::npos) {
				
			}
			else {
				subCmds.push_back(last);
			}
			break;
		}
		subCmds.push_back(command.substr(start, index));
		start = index;
	}

	return subCmds;
}

void executeCommand(const std::string& command)
{
	if (command == "jobs") {
		printJobs();
		return;
	}

	std::vector<std::string> subCmds = splitCmd(command);
	int builtinCommand = isBuiltInCommand(subCmds[0]);

	if (builtinCommand != -1) {

	}
}

int main()
{
	std::string command;
	while (true) {
		std::cout << "$:";
		std::cin >> command;
		executeCommand(command);
	}
    return 0;
}
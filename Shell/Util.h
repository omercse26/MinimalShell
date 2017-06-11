#pragma once
#include <vector>
#include <string>
struct Util
{
	static std::vector<std::string> splitStringToVector(const std::string& command)
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
		return subCmds;
	}
};

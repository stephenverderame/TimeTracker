#include <stdio.h>
#include <AppTimes.h>
#include <iostream>
#include <TimeTracker.h>

std::ostream& displayHelp(std::ostream& s);
/// @return true if the yes option was selected, otherwise false
bool yesNoConfirmation(std::string&& msg);
int main(int argc, char ** argv)
{
	auto tracker = getTracker(TrackerType::live);
	if (argc <= 1 || strcmp(argv[1], "help") == 0) {
		std::cout << displayHelp;
	}
	else if (argc == 2 && strcmp(argv[1], "ls") == 0) {
		std::cout << "Tasks:\n";
		const auto v = tracker->listTasks();
		for (const auto e : v) {
			std::cout << e << "\n";
		}
	}
	else if (argc == 3 && !strcmp(argv[1], "start")) {
		auto t = tracker->startTask(argv[2]);
		std::cout << argv[2] << " started at " << printTimestamp(t) << "\n";
	}
	else if (argc == 3 && !strcmp(argv[1], "pause")) {
		auto t = tracker->pauseTask(argv[2]);
		std::cout << argv[2] << " paused at " << printTimestamp(t) << "\n";
	}
	else if (argc == 5 && !strcmp(argv[1], "dur_per_day")) {
		auto stFmt = guessDateFormat(argv[3]);
		auto enFmt = guessDateFormat(argv[4]);
		auto times = timeEachPeriod(*tracker, argv[2], readTimestamp(argv[3], stFmt.c_str()),
			readTimestamp(argv[4], enFmt.c_str()), std::chrono::duration_cast<Duration>(std::chrono::hours(24)));
		for (auto& p : times)
		{
			std::cout << printTimestamp(p.first, "%D") << " " << printDuration(p.second) << "\n";
		}
	}
	else if (argc == 5 && !strcmp(argv[1], "dur")) {
		auto stFmt = guessDateFormat(argv[3]);
		auto enFmt = guessDateFormat(argv[4]);
		const auto dur = elapsedTime(*tracker, argv[2], readTimestamp(argv[3], stFmt.c_str()),
			readTimestamp(argv[4], enFmt.c_str()));
		std::cout << printDuration(dur) << "\n";
	}
	else if (argc == 3 && !strcmp(argv[1], "del")) {		
		if (yesNoConfirmation("Are you sure you want to delete all information about " 
			+ std::string(argv[2]) + "?")) {
			std::cout << "Deleted " << argv[2] << "\n";
			tracker->delTask(argv[2]);
		}
	}
	else {
		std::cout << "Unknown command or list of arguments: \n";
		for (int i = 1; i < argc; ++i)
			std::cout << '"' << argv[i] << (i < argc - 1 ? "\" " : "\"");
		std::cout << '\n';
		std::cout << displayHelp << "\n";
	}
	return 0;
}

std::ostream& displayHelp(std::ostream& s)
{
	s << "TaskTracker Help \n";
	s << "help - show this message\n";
	s << "ls - list all tasks\n";
	s << "start [name] - start a task\n";
	s << "pause [name] - pause a task\n";
	s << "dur [name] [start] [end] - time spent from start to end\n";
	s << "dur_per_day [name] [start] [end] - time spent from start to end each day\n";
	s << "del [name] - delete a task\n";
	return s;
}

bool yesNoConfirmation(std::string&& msg)
{
	std::cout << msg << " (Y/n)\n";
	return tolower(getchar()) == 'y';
}


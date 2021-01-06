#define _CRT_SECURE_NO_WARNINGS
#include <AppTimes.h>
#include <iostream>
#include <locale>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <regex>
const char* default_fmt = "%D %I:%M:%S %p";
const std::regex mdy("^(((1[0-2])|(0?[0-9]))\\/(([1-3][0-9])|(0[0-9]))\\/[0-9]{2,})($| )");
const std::regex dmy("^((([1-3][0-9])|(0[0-9]))\\/((1[0-2])|(0?[0-9]))\\/[0-9]{2,})($| )");
const std::regex posix("^([0-9]{4})-((1[0-2])|(0?[0-9]))-(([1-3][0-9])|(0[0-9]))($| )");

const std::regex period("(\\/| |^)(am|pm)(\\/| |$)", std::regex_constants::icase);
const std::regex hhmm("(^| )([0-9]+:(([0-5]{1}[0-9]{1})|(0?[0-9]{1})))($| |:)");
const std::regex sec(":(([1-5][0-9])|(0?[0-9]))(($| )|(\\.\\d+)?($| ))");

const std::regex month("((^| )((0?[0-9])|(1[0-2]))\\/)|(\\/((0?[0-9])|(1[0-2]))\\/)");
const std::regex twoDigitYear("((\\/| )([0-9]{2,})($| ))");
const std::regex day("([1-3][0-9](\\/| |$))|((0?[0-9]{1})(\\/| |$))");

const std::regex monthName("[a-zA-Z]{3,}");
const std::regex fourDigitYear("[0-9]{4}");

const std::regex delim("( |\\/|:)");
template<typename T>
void zeroMemory(T& object)
{
	memset(&object, 0, sizeof(T));
}
/**
 * A general constructor for a POD struct that initializes memory to 0
 */
template<typename T>
T init()
{
	T ret;
	zeroMemory(ret);
	return ret;
}

std::string printTimestamp(Timestamp t, const char * fmt)
{
	if (fmt == nullptr) fmt = default_fmt;
	const auto time = Timestamp::clock::to_time_t(
		std::chrono::time_point_cast<Timestamp::clock::duration>(t));
	auto tm = std::localtime(&time);
	std::stringstream ss;
//	ss.imbue(std::locale());
	ss << std::put_time(tm, fmt);
	return ss.str();

}
/// @return localTime - utcTime
Duration localOffsetFromUTC()
{
	auto utc = now();
	auto utcTime = Timestamp::clock::to_time_t(
		std::chrono::time_point_cast<Timestamp::clock::duration>(utc)
	);
	auto localTm = localtime(&utcTime);
	localTm->tm_isdst = -1;
	auto localTime = mktime(localTm);
	auto local = Timestamp::clock::from_time_t(localTime);
	return local - utc;
}

Timestamp readTimestamp(const std::string& s, const char * fmt)
{
	if (fmt == nullptr) fmt = default_fmt;
	auto tm = init<struct tm>();
	std::stringstream ss(s);
	ss >> std::get_time(&tm, fmt);
	tm.tm_isdst = -1; //let mktime figure out daylight savings
	auto localTime = mktime(&tm);
//	std::cout << std::put_time(&utcStruct, fmt) << " UTC \n";*/
	return Timestamp::clock::from_time_t(localTime) - localOffsetFromUTC();

}
/// @return true if the character is a date delimiter (' ', '/', or ':')
inline bool isDelim(char c)
{
	return c == ' ' || c == '/' || c == ':';
}
/**
 * Constructs a new string where target is replaced by replacement in s with the exclusion of the first and last character of target
 * if those characters delim characters (' ', '/', or ':')
 */
std::string replaceAndKeepDelim(const std::string& s, decltype(std::string().cbegin()) start, 
	decltype(std::string().cbegin()) end, const char* replacement)
{
	std::stringstream ss;
	std::copy(s.begin(), start, std::ostream_iterator<char>(ss));
	if (isDelim(*start)) ss << *start;
	ss << replacement;
	if (isDelim(*(end - 1))) ss << *(end - 1);
	std::copy(end, s.end(), std::ostream_iterator<char>(ss));
	return ss.str();
}
/// Replaces time patterns with their posix specifier
std::string replaceTime(const std::string& s)
{
	bool twelveHour = false;
	std::smatch matches;
	std::string res = s;
	if (std::regex_search(res, matches, period)) {
		res = replaceAndKeepDelim(res, matches[0].first, matches[0].first + matches[0].length(),
			"%p");
		twelveHour = true;
	}
	if (std::regex_search(res, matches, hhmm)) {
		const char* replacement = twelveHour ? "%I:%M" : "%H:%M";
		res = replaceAndKeepDelim(res, matches[0].first, matches[0].first + matches[0].length(), replacement);
	}
	if (std::regex_search(res, matches, sec)) {
		res = replaceAndKeepDelim(res, matches[0].first, matches[0].first + matches[0].length(), 
			"%S");

	}
	return res;
}
/// Replaces year patterns with their posix specifier
std::string replaceYear(const std::string& s)
{
	std::string res = s;
	std::smatch matches;
	if (std::regex_search(res, fourDigitYear)) {
		res = std::regex_replace(res, fourDigitYear, "%Y");
	}
	else if (std::regex_search(res, matches, twoDigitYear)) {
		auto preferredMatch = &matches[0];
		for (auto i = 1; i < matches.size(); ++i)
		{
			const auto str = std::regex_replace(matches[i].str(), delim, "");
			try {
				if (std::stoi(str) > 31) {
					preferredMatch = &matches[i];
					break;
				}
			}
			catch (std::exception&) {}
		}
		res = replaceAndKeepDelim(res, preferredMatch->first, 
			preferredMatch->first + preferredMatch->length(), "%y");
	}
	return res;
}
/// Replaces month patterns with their posix specifier
std::string replaceMonth(const std::string& s)
{
	std::string res = s;
	std::smatch matches;
	if (std::regex_search(res, monthName)) {
		res = std::regex_replace(res, monthName, "%b");
	}
	else if (std::regex_search(res, matches, month))
	{
		res = replaceAndKeepDelim(res, matches[0].first, matches[0].first + matches[0].length(), "%m");
	}
	return res;
}
/// Replaces day pattern with its posix specifier
std::string replaceDay(const std::string& s)
{
	std::string res = s;
	std::smatch matches;
	if (std::regex_search(res, matches, day)) {
		res = replaceAndKeepDelim(res, matches[0].first, matches[0].first + matches[0].length(), "%d");
	}
	return res;
}
std::string guessDateFormat(const std::string& s)
{
	auto res = std::regex_replace(s, std::regex("-"), "/");
	const std::string dateDelim = s.find('-') != std::string::npos ? "-" : "/";

	std::smatch matches;
	res = replaceTime(res);
	res = replaceMonth(res);
	res = replaceYear(res);
	res = replaceDay(res);
	return std::regex_replace(res, std::regex("/"), dateDelim);
}

std::string printDuration(Duration d)
{
	std::stringstream ss;
	auto time = Timestamp::clock::to_time_t(std::chrono::duration_cast<Timestamp::clock::duration>(d) 
		+ Timestamp::clock::time_point());
	auto tm = *gmtime(&time);
	ss << std::put_time(&tm, "%T");
	return ss.str();
}

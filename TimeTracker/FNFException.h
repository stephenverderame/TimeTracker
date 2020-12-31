#pragma once
#include <exception>
#include <string>
class FnfException : public std::exception {
private:
	std::string fileName;
	mutable std::string msg;
public:
	FnfException(const char* fileName) : fileName(fileName) {}
	const char* what() const noexcept override;
	inline const std::string& getFileName() const noexcept { return fileName; }
};
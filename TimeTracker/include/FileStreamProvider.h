#pragma once
#include "StreamProvider.h"
class FileStreamProvider : public StreamProvider {
	struct impl;
	std::unique_ptr<impl> pimpl;
public:
	std::shared_ptr<StreamWrapper> getStream(const char* name) override;
	bool doesStreamExist(const char* name) const override;
	void clearStreamData(const char* name) override;
	std::vector<std::string> getAllStreamNames() const override;
	~FileStreamProvider();
	/**
	 * @param streamNames, the name of the folder that all file streams will be located in
	 */
	FileStreamProvider(const char * streamNames);
};
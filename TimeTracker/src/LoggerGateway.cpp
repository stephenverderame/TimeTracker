#include <LoggerGateway.h>
#include <StreamLogger.h>
std::unique_ptr<LoggerGateway> makeDBGateway(DBType type)
{
	StreamType t;
	switch (type) {
	case DBType::test:
		t = StreamType::memory;
		break;
	case DBType::local:
		t = StreamType::file;
		break;
	default:
		throw std::runtime_error("Unknown db type");
	}
	return std::make_unique<StreamLogger>(makeStreamProvider(t, "tasks"));
}
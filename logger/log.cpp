#include "log.h"
#include <vector>
#include <functional>
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "LogConfigMgr.h"

Logger& Logger::Instance()
{
	// TODO: 在此处插入 return 语句
	static Logger logger;
	return logger;
}

bool Logger::Start(spdlog::custom_log_callback callback)
{
	auto pool_page = LogConfigMgr::Instance().GetCfg().poolPage;
	auto th_num = LogConfigMgr::Instance().GetCfg().thNum;
	spdlog::init_thread_pool(pool_page, th_num);

	auto rotate_path = LogConfigMgr::Instance().GetCfg().syslogPath;
	auto rotate_size = LogConfigMgr::Instance().GetCfg().maxFileSize;
	auto rotate_files = LogConfigMgr::Instance().GetCfg().maxFileNum;
	auto syslevel = LogConfigMgr::Instance().GetCfg().syslevel;

	std::vector<spdlog::sink_ptr> sinks;
	sys_rotate_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(rotate_path, rotate_size, rotate_files);
	sinks.emplace_back(sys_rotate_sink);
	sys_rotate_sink->set_pattern("[%Y-%m-%d %H:%M:%S:%e][%n][thread:%t][%l][%s:%!:%#]:%v");
	sys_rotate_sink->set_level(static_cast<spdlog::level::level_enum>(syslevel));

	if (callback != nullptr) {
		callback_sink = std::make_shared<spdlog::sinks::callback_sink_mt>(callback);
		callback_sink->set_level(spdlog::level::info);
		sinks.emplace_back(callback_sink);
	}
	
	sys_logger = std::make_shared<spdlog::async_logger>("sys_logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	//need register to use everywhere	
	spdlog::register_logger(sys_logger);
	return true;
}

std::shared_ptr<spdlog::logger> Logger::GetSysLogger()
{
	return sys_logger;
}

Logger::Logger()
{
}

Logger::~Logger()
{
	spdlog::drop_all();
}
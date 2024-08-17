#pragma once
#include <iostream>
#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/callback_sink.h"

class Logger
{
public:
	static Logger& Instance();
	bool Start(spdlog::custom_log_callback callback = nullptr);
	std::shared_ptr<spdlog::logger> GetSysLogger();

private:
	Logger();
	~Logger();
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	//sink
	spdlog::sink_ptr sys_rotate_sink;
	//spdlog::sink_ptr qt_sink;
	spdlog::sink_ptr callback_sink;

	//logger
	std::shared_ptr<spdlog::logger> sys_logger;
};

#define SYS_LOG_TRACE(...) SPDLOG_LOGGER_CALL(Logger::Instance().GetSysLogger(), spdlog::level::trace, __VA_ARGS__)
#define SYS_LOG_DEBUG(...) SPDLOG_LOGGER_CALL(Logger::Instance().GetSysLogger(), spdlog::level::debug, __VA_ARGS__)
#define SYS_LOG_INFO(...) SPDLOG_LOGGER_CALL(Logger::Instance().GetSysLogger(), spdlog::level::info, __VA_ARGS__)
#define SYS_LOG_WARN(...) SPDLOG_LOGGER_CALL(Logger::Instance().GetSysLogger(), spdlog::level::warn, __VA_ARGS__)
#define SYS_LOG_ERROR(...) SPDLOG_LOGGER_CALL(Logger::Instance().GetSysLogger(), spdlog::level::err, __VA_ARGS__)
#define SYS_LOG_CRITI(...) SPDLOG_LOGGER_CALL(Logger::Instance().GetSysLogger(), spdlog::level::critical, __VA_ARGS__)


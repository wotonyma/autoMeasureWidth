#pragma once
#include <iostream>

class LogCfg {
public:
	std::string syslogPath = "logs/syslog.log";
	int poolPage = 8192;
	int thNum = 1;
	int maxFileSize = 8192;
	int maxFileNum = 10;
	int syslevel = 3;
};

class LogConfigMgr
{
public:		
	static LogConfigMgr& Instance();
	bool Load();
	const LogCfg& GetCfg();

private:
	LogConfigMgr();
	~LogConfigMgr();
	LogConfigMgr(const LogConfigMgr&) = delete;
	LogConfigMgr& operator=(const LogConfigMgr&) = delete;

	std::string cfgPath;
	LogCfg cfg;
};


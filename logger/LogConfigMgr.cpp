#include "LogConfigMgr.h"
#include <filesystem>
#include <QSettings>

LogConfigMgr& LogConfigMgr::Instance()
{
	static LogConfigMgr mgr;
	return mgr;
}

bool LogConfigMgr::Load()
{
	if (!std::filesystem::exists(cfgPath))
		return false;

	auto iniFile = QSettings(cfgPath.c_str(), QSettings::IniFormat);
	cfg.syslogPath = iniFile.value("path/syslog").toString().toStdString();
	cfg.poolPage = iniFile.value("threadpool/page").toInt();
	cfg.thNum = iniFile.value("threadpool/num").toInt();
	cfg.maxFileSize = iniFile.value("rotating/maxSize").toInt();
	cfg.maxFileNum = iniFile.value("rotating/maxFiles").toInt();
	cfg.syslevel = iniFile.value("level/syslevel").toInt();

	return true;
}

const LogCfg& LogConfigMgr::GetCfg()
{
	// TODO: 在此处插入 return 语句
	return cfg;
}

LogConfigMgr::LogConfigMgr()
{
	auto curPath = std::filesystem::current_path();
	auto logPath = curPath / "config" / "log.ini";
	cfgPath = logPath.string();
}

LogConfigMgr::~LogConfigMgr()
{
	//使用单例请在程序结束前调用sqdlog::shutdown
	//原因:main可能先于静态变量logger释放内存导致crashed
}
#pragma once

#include <iostream>
#include <memory>
#include <QThread>
#include <vector>
#include <optional>
#include <QImage>

#define BASLER_CAMERA //define camera types
//#define HIK_CAMERA //define camera types

#ifdef BASLER_CAMERA
#include "basler_obj.h"
using CamOBJ = BaslerObject;
#endif // BASLER_CAMERA

#ifdef HIK_CAMERA
#include "hik_obj.h"
using CamOBJ = HikObject;
#endif // HIK_CAMERA

using CamPtr = std::shared_ptr<CamOBJ>;

class CamWorker
{
public:
	static CamWorker& instance();
	void start();
	void stop();
	void openCameras();
	void startFreeGrab();

	std::vector<CamPtr> cam_vec;
	std::vector<std::string> userIdList{ "c1" }; //可写入配置文件
private:
	CamWorker();
	~CamWorker();
	CamWorker(const CamWorker& src) = delete;
	CamWorker& operator=(const CamWorker& src) = delete;

	std::shared_ptr<QThread> _th;
};

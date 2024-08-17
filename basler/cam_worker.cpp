#include "cam_worker.h"
#include "basler_obj.h"
#include "log.h"

CamWorker::CamWorker()
{
}

CamWorker::~CamWorker()
{
	stop();
}

CamWorker& CamWorker::instance()
{
	static CamWorker inst;
	return inst;
}

void CamWorker::start()
{
	if (_th != nullptr) //is working
	{
		stop();
	}

	_th = std::make_shared<QThread>();
	cam_vec.clear();
	for (auto& id : userIdList)
	{
		//move camobj to threaq
		auto obj = std::make_shared<CamOBJ>();
		obj->moveToThread(_th.get());
		cam_vec.emplace_back(obj);
	}

	_th->start();
}

void CamWorker::stop()
{
	for (auto& obj : cam_vec)
	{
		if (obj == nullptr) continue;
		obj->closeCamera();
		obj.reset();
	}

	if (_th != nullptr)
	{
		_th->quit();
		_th->wait();
		_th.reset();
	}
}

void CamWorker::openCameras()
{
	auto open_task = [this] {
		for (size_t i = 0; i < cam_vec.size(); ++i)
		{
			if (!cam_vec[i]->openCameraByUserID(userIdList[i]))
				SYS_LOG_ERROR("open camera [{}] failed!", userIdList[i]);
			SYS_LOG_INFO("open camera[{}] success.", userIdList[i]);
		}
	};

	if (cam_vec.empty()) return;
	QMetaObject::invokeMethod(cam_vec[0].get(), open_task);		//相机线程open
}

void CamWorker::startFreeGrab()
{
	auto grab_task = [this] {
		for (size_t i = 0; i < cam_vec.size(); ++i)
		{
			cam_vec[i]->setFreeRunMode();
			cam_vec[i]->startGrab();
		}
	};
	QMetaObject::invokeMethod(cam_vec[0].get(), grab_task);		//相机线程grab
}

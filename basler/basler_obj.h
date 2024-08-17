#pragma once

#include <QObject>
#include <QImage>
#include <memory>
#include <optional>

class Basler;

class BaslerObject : public QObject
{
	Q_OBJECT

public:
	BaslerObject(QObject *parent = nullptr);
	~BaslerObject();

	std::shared_ptr<Basler> getCam();
	bool openCameraByUserID(const std::string& id);
	void closeCamera();
	bool isOpen() { return active; }
	//触发模式
	bool setFreeRunMode();
	bool setSoftwareMode();
	bool setHardwareMode();
	//快捷指令
	void executeTrigger();	//软触发
	void setExposureTimeRaw(int val);
	void setGainRaw(int val);
	void startGrab();
	void stopGrab();

	std::optional<QImage> getCameraCurrentImage();
	
signals:
	void grabbedImageSig(QImage img);

private:
	static void grabImageCallBack(void* pOwner, int* width, int* height, unsigned char* pBuffer, bool isColor);
	void setCamPara();

private:
	std::shared_ptr<Basler> cam;
	bool active;
};

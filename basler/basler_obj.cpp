#include "basler_obj.h"
#include "Basler.h"

BaslerObject::BaslerObject(QObject *parent)
	: QObject(parent)
{
	cam = std::make_shared<Basler>();
	active = false;
}

BaslerObject::~BaslerObject()
{
}

std::shared_ptr<Basler> BaslerObject::getCam()
{
	return cam;
}

bool BaslerObject::openCameraByUserID(const std::string& id)
{
	if (cam->OpenCameraByUserID(id.c_str())) {
		setCamPara();
		active = true;
	}
	return active;
}

void BaslerObject::closeCamera()
{
	if (cam != nullptr)
	{
		cam->StopGrab();
		cam->CloseCamera();
	}
	active = false;
}

bool BaslerObject::setFreeRunMode()
{
	bool br0 = cam->SetEnumPara("TriggerSelector", "FrameStart");
	bool br1 = cam->SetEnumPara("TriggerMode", "Off");
	return br0 && br1;
}

bool BaslerObject::setSoftwareMode()
{
	bool br0 = cam->SetEnumPara("TriggerSelector", "FrameStart");
	bool br1 = cam->SetEnumPara("TriggerSource", "Software");
	bool br2 = cam->SetEnumPara("TriggerMode", "On");
	return br0 && br1 && br2;
}

bool BaslerObject::setHardwareMode()
{
	bool br0 = cam->SetEnumPara("TriggerSelector", "FrameStart");
	bool br1 = cam->SetEnumPara("TriggerSource", "Line1");
	bool br2 = cam->SetEnumPara("TriggerMode", "On");
	return br0 && br1 && br2;
}

void BaslerObject::executeTrigger()
{
	cam->SetCmd("TriggerSoftware");
}

void BaslerObject::setExposureTimeRaw(int val)
{
	cam->SetIntPara("ExposureTimeRaw", val);
	cam->GetIntPara("ExposureTimeRaw", cam->intExp); //update
}

void BaslerObject::setGainRaw(int val)
{
	cam->SetIntPara("GainRaw", val);
	cam->GetIntPara("GainRaw", cam->intExp); //update
}

void BaslerObject::startGrab()
{
	cam->StartGrab();
}

void BaslerObject::stopGrab()
{
	cam->StopGrab();
}

/// <summary>
/// camera grab one image callback
/// </summary>
/// <param name="pOwner">user pointer</param>
/// <param name="width">image width</param>
/// <param name="height">image height</param>
/// <param name="pBuffer">image buffer</param>
/// <param name="isColor">image format is colorful</param>
void BaslerObject::grabImageCallBack(void* pOwner, int* width, int* height, unsigned char* pBuffer, bool isColor)
{
	BaslerObject* obj = (BaslerObject*)pOwner;
	QImage::Format qImgFmt = isColor? QImage::Format_BGR888 : QImage::Format_Grayscale8;
	QImage wapper = { pBuffer, *width, *height, qImgFmt};
	//QImage qimg = wapper.copy(); //no clone; if you want to process, clone the wrapper
	emit obj->grabbedImageSig(wapper);
}

void BaslerObject::setCamPara()
{
	cam->SetIntPara("GevHeartbeatTimeout", 6000);			// 如果是网口相机，则可以将心跳超时设置短点。默认3分钟，即程序异常退出，相机3分钟后自动释放资源。
	cam->SetFloatPara("AcquisitionFrameRateAbs", 90);		// 限制相机帧率为90fps
	cam->SetBoolPara("AcquisitionFrameRateEnable", true);	// 限制相机帧率使能

	//设置曝光
	//cam.SetStringPara("ExposureAuto", "off");
	//cam.SetIntPara("ExposureTimeRaw", 5000);

	// 以下两步骤函数用于将SDK采集回调中的图像数据传出相机类
	BaslerGrabbedCallback callback = (BaslerGrabbedCallback)grabImageCallBack;
	cam->SetOwner(this, callback);
}

std::optional<QImage> BaslerObject::getCameraCurrentImage()
{
	if (cam == nullptr)
		return std::nullopt;
	if (cam->mCamera == nullptr)
		return std::nullopt;
	if (cam->pImageBufferMono == nullptr
		&& cam->pImageBufferColor == nullptr)
	{
		return std::nullopt;
	}

	QImage::Format qImgFmt = cam->isColor ? QImage::Format_BGR888 : QImage::Format_Grayscale8;
	uchar* pBuffer = cam->isColor ? cam->pImageBufferColor : cam->pImageBufferMono;
	QImage wapper = { pBuffer, (int)cam->width, (int)cam->height, qImgFmt };

	return wapper;
}
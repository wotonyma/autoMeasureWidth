#pragma once

#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

typedef void ( *BaslerGrabbedCallback)(void* pOwner, int* width, int* height, unsigned char* pBuffer, bool isColor);

class Basler: public Pylon::CImageEventHandler,public Pylon::CConfigurationEventHandler
{
public:
	Basler(void);
	~Basler(void);

	Pylon::CInstantCamera *mCamera;
	GenApi::INodeMap*  nodemap;
	INT64       intExp;		// 曝光时间值
	INT64      intGain;		// 增益值
	INT64        width;		// 图像宽
	INT64       height;		// 图像高
	std::string   strModelName;		// 相机型号
	std::string    strSnNumber;		// 相机SN号
	std::string      strUserID;		// 相机UserID

	bool       isColor;		// 判断是否彩色相机
	void*     m_pOwner;
	unsigned char * pImageBufferMono;	// 自定义黑白图像指针
	unsigned char * pImageBufferColor;	// 自定义彩色图像指针
	BaslerGrabbedCallback m_DisplayImageCallBack;	// 传递图像用函数指针方法
	void SetOwner(void* pOwner, BaslerGrabbedCallback pDisplayImageCallBack);

	static std::vector<std::string> DetectCameras();		  //检测所有相机并返回userid列表
	bool OpenCamera();                                        // 打开默认相机
	bool OpenCameraBySn(Pylon::String_t serialNumber);               // 通过SN号打开相机
	bool OpenCameraByUserID(Pylon::String_t UserDefinedName);        // 通过UserID打开相机
	void GrabOne();											  // 采集单张
	void StartGrab();                                         // 开始采集
	void StopGrab();										  // 停止采集
	void CloseCamera();										  // 关闭相机
	bool GetFloatPara(const char * nameNode, double& para);   // 获取浮点型参数
	bool GetIntPara(const char * nameNode, int64_t&  para);	  // 获取整形参数
	bool GetStringPara(const char * nameNode, std::string& para);  // 获取字符串型参数
	bool GetEnumPara(const char * nameNode, std::string& para);    // 获取字符串参数
	bool GetBoolPara(const char * nameNode, bool& para);	  // 获取布尔型参数
	bool SetFloatPara(const char * nameNode, double para);    // 设置浮点型参数
	bool SetIntPara(const char * nameNode, INT64 para);       // 设置整形参数
	bool SetEnumPara(const char * nameNode, Pylon::String_t para);   // 设置字符串参数
	bool SetStringPara(const char * nameNode, Pylon::String_t para);   // 设置字符串参数
	bool SetBoolPara(const char * nameNode, bool para);	      // 设置布尔型参数
	bool SetCmd(const char *name);                            // 执行命令
	void RemovedDeviceReconnect();                            // 掉线重连函数

	// 取像回调函数
	virtual void OnImageGrabbed(Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& ptrGrabResult);
	// 掉线重连回调函数
	virtual void OnCameraDeviceRemoved(Pylon::CInstantCamera& camera);

private:
	int64_t Adjust(int64_t val, int64_t minimum, int64_t maximum, int64_t inc);
	
};


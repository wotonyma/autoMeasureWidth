#include "Basler.h"
#include <iostream>

using namespace std;
using namespace Pylon;
using namespace GenApi;

Basler::Basler(void)
{
	// Before using any pylon methods, the pylon runtime must be initialized. 
	PylonInitialize();

	mCamera = nullptr;
	nodemap = nullptr;
	pImageBufferMono = nullptr;
	pImageBufferColor = nullptr;

	nodemap = nullptr;
	intExp = 0;
	intGain = 0;
	width = 0;
	height = 0;
	isColor = false;
	m_pOwner = nullptr;
	m_DisplayImageCallBack = nullptr;
}


Basler::~Basler(void)
{
	if( mCamera != NULL ) 
	{
		mCamera = NULL;
		nodemap = NULL;
	}

	pImageBufferMono = NULL;
	if (pImageBufferColor != NULL) {
		free(pImageBufferColor);
	}
		

	PylonTerminate();
}

std::vector<std::string> Basler::DetectCameras()
{
	PylonInitialize(); 
	std::vector<std::string> id_list;
	try
	{
		// Get the transport layer factory.
		CTlFactory& tlFactory = CTlFactory::GetInstance();

		// Get all attached devices and exit application if no device is found.
		DeviceInfoList_t devices;
		if (tlFactory.EnumerateDevices(devices) == 0)
		{
			throw RUNTIME_EXCEPTION("No camera present.");
		}

		// Create an array of instant cameras for the found devices and avoid exceeding a maximum number of devices.
		CInstantCameraArray cameras(min(devices.size(), 8));

		// Create and attach all Pylon Devices.
		for (size_t i = 0; i < cameras.GetSize(); ++i)
		{
			cameras[i].Attach(tlFactory.CreateDevice(devices[i]));

			// Print the model name of the camera.
			//cout << "Using device " << cameras[i].GetDeviceInfo().GetModelName() << endl;
			if (cameras[i].IsGigE()) {
				id_list.emplace_back(cameras[i].GetDeviceInfo().GetUserDefinedName().c_str());
			}
			//cameras[i].IsUsb();
			//cameras[i].IsCameraLink()
		}
		return id_list;
	}
	catch (const GenericException& e) {
		cerr << "An exception occurred." << endl << e.GetDescription() << endl;
		return id_list;
	}
}

bool Basler::OpenCamera()
{
	try
	{
		// Create an instant camera object for the camera device found first.
		mCamera = new CInstantCamera( CTlFactory::GetInstance().CreateFirstDevice());

		if(mCamera != NULL) 
		{
			nodemap = &mCamera->GetNodeMap();
			mCamera->RegisterConfiguration(this, RegistrationMode_Append, Cleanup_None);
			mCamera->RegisterImageEventHandler( this, RegistrationMode_Append, Cleanup_None);
			mCamera->Open();

			// 设置图像缓存区大小
			mCamera->MaxNumBuffer = 10;
			GetIntPara("Width", width);					// 获取相机图像宽高
			GetIntPara("Height", height);
			GetStringPara("DeviceModelName", strModelName);
			GetStringPara("DeviceID", strSnNumber);
			GetStringPara("DeviceUserID", strUserID);
			GetIntPara("ExposureTimeRaw", intExp);
			GetIntPara("GainRaw", intGain);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	catch (const GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl << e.GetDescription() << endl;
		return FALSE;		
	}

	
}

bool Basler::OpenCameraByUserID(String_t UserDefinedName)
{
	try
	{
		CDeviceInfo di;
		di.SetUserDefinedName(UserDefinedName);
		mCamera = new CInstantCamera(CTlFactory::GetInstance().CreateFirstDevice(di));

		if(mCamera != NULL) 
		{
			nodemap = &mCamera->GetNodeMap();
			mCamera->RegisterConfiguration(this, RegistrationMode_Append, Cleanup_None);
			mCamera->RegisterImageEventHandler( this, RegistrationMode_Append, Cleanup_None);
			mCamera->Open();

			// 设置图像缓存区大小
			mCamera->MaxNumBuffer = 10;
			GetIntPara("Width", width);					// 获取相机图像宽高
			GetIntPara("Height", height);
			GetStringPara("DeviceModelName", strModelName);
			GetStringPara("DeviceID", strSnNumber);		// USB相机为 DeviceSerialNumber
			GetStringPara("DeviceUserID", strUserID);
			GetIntPara("ExposureTimeRaw", intExp);
			GetIntPara("GainRaw", intGain);

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	catch (const GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl << e.GetDescription() << endl;
		return FALSE;		
	}
}

bool Basler::OpenCameraBySn(String_t serialNumber)
{
	try
	{
		CDeviceInfo info;
		info.SetSerialNumber(serialNumber); 
		mCamera = new CInstantCamera(CTlFactory::GetInstance().CreateFirstDevice(info));

		if(mCamera != NULL) 
		{
			nodemap = &mCamera->GetNodeMap();
			mCamera->RegisterConfiguration(this, RegistrationMode_Append, Cleanup_None);
			mCamera->RegisterImageEventHandler( this, RegistrationMode_Append, Cleanup_None);
			mCamera->Open();

			// 设置图像缓存区大小
			mCamera->MaxNumBuffer = 10;
			GetIntPara("Width", width);					// 获取相机图像宽高
			GetIntPara("Height", height);
			GetStringPara("DeviceModelName", strModelName);
			GetStringPara("DeviceID", strSnNumber);
			GetStringPara("DeviceUserID", strUserID);
			GetIntPara("ExposureTimeRaw", intExp);
			GetIntPara("GainRaw", intGain);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	catch (const GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl << e.GetDescription() << endl;
		return FALSE;		
	}
}

void Basler::GrabOne()
{
	if (!mCamera->IsGrabbing())
	{
		mCamera->StartGrabbing(1, GrabStrategy_LatestImageOnly, GrabLoop_ProvidedByInstantCamera);
	}
	else
	{
		std::cerr << "grab one failed! camera is grabbing now." << std::endl;
	}
}

void Basler::StartGrab()
{
	if (!mCamera->IsGrabbing())
	{
		mCamera->StartGrabbing( GrabStrategy_LatestImageOnly, GrabLoop_ProvidedByInstantCamera);
	}
	else
	{
		std::cerr << "start grab failed! camera is grabbing now." << std::endl;
	}
}

void Basler::StopGrab()
{
	if(mCamera) {
		if (mCamera->IsGrabbing())
			mCamera->StopGrabbing();
	}
}

void Basler::CloseCamera()
{
	if( mCamera != NULL ) 
	{
		mCamera->Close();
		mCamera->DeregisterConfiguration(this);
		mCamera->DeregisterImageEventHandler(this);
		delete mCamera;
		mCamera = nullptr;
	}
}

bool Basler::GetFloatPara(const char * nameNode, double& para)
{
	try {
		if( nodemap ) {
			CFloatPtr tmp( nodemap->GetNode( nameNode ));

			para = tmp->GetMin();

			return TRUE;
		}
	} catch (const GenericException &e) {
		cerr << e.GetDescription();
	}
	

	return FALSE;
}

bool Basler::GetIntPara(const char * nameNode, int64_t& para)
{
	try {
		if( nodemap ) {
			CIntegerPtr tmp( nodemap->GetNode( nameNode));
			para = tmp->GetValue();
			return true;
		}	
	} catch (const GenericException &e) {
		cerr << e.GetDescription();
	}

	return false;
}

bool Basler::GetStringPara(const char * nameNode, string& para)
{
	try {
		if( nodemap ) {
			CStringPtr tmp( nodemap->GetNode( nameNode));
			para = tmp->GetValue();

			return true;
		}	
	} catch (const GenericException &e) {
		cerr << e.GetDescription();
	}

	return false;
}

bool Basler::GetEnumPara(const char * nameNode, string& para)
{
	try {
		if( nodemap ) {
			CEnumerationPtr tmp( nodemap->GetNode( nameNode));
			para = tmp->ToString();

			return true;
		}	
	} catch (const GenericException &e) {
		cerr << e.GetDescription();
	}

	return false;
}

bool Basler::GetBoolPara(const char * nameNode, bool& para)
{
	try {
		if( nodemap ) {
			CBooleanPtr tmp( nodemap->GetNode( nameNode));
			para = tmp->GetValue();

			return true;
		}	
	} catch (const GenericException &e) {
		cerr << e.GetDescription();
	}

	return false;
}

bool Basler::SetFloatPara(const char * nameNode, double para)
{
	try {
	if( nodemap ) {
		CFloatPtr tmp( nodemap->GetNode( nameNode));
		
		if ( IsWritable(tmp) ) {
			tmp->SetValue(para);
			return true;
		} 
	}	
	} catch (GenericException &e) {
		cerr << e.GetDescription();
	}

	return false;
}

bool Basler::SetIntPara(const char * nameNode, INT64 para)
{
	try {
		if( nodemap ) {
			CIntegerPtr tmp( nodemap->GetNode( nameNode ));

			if ( IsWritable(tmp) ) {
				para = Adjust(para, tmp->GetMin(), tmp->GetMax(), tmp->GetInc());
				tmp->SetValue(para);
				return true;
			}
		}	
	} catch (GenericException &e) {
		cerr << e.GetDescription();
	}

	return false;
}

bool Basler::SetStringPara(const char * nameNode, String_t para)
{
	try {
	if( nodemap ) {
		CStringPtr tmp( nodemap->GetNode( nameNode));
		
		if ( IsWritable(tmp) ) {
			tmp->SetValue(para);
			return true;
		} 
	}	
	} catch (GenericException &e) {
		cerr << e.GetDescription();
	}

	return false;
}

bool Basler::SetEnumPara(const char * nameNode, String_t para)
{
	try {
		if( nodemap ) {
			CEnumerationPtr tmp( nodemap->GetNode( nameNode ));

			if ( IsWritable(tmp) && IsAvailable( tmp->GetEntryByName( para) ) ) {

				tmp->FromString(para);

				return true;
			} 
		}	
	} catch (GenericException &e) {
		cerr << e.GetDescription();
	}
	
	return false;
}

bool Basler::SetBoolPara(const char * nameNode, bool para)
{
	try {
		if( nodemap ) {
			CBooleanPtr tmp( nodemap->GetNode( nameNode ));

			if ( IsWritable(tmp) ) {
			tmp->SetValue(para);
			return true;
			}
		}	
	} catch (GenericException &e) {
		cerr << e.GetDescription();
	}
	
	return false;
}

bool Basler::SetCmd(const char *nameNode)
{
	try {
		if( nodemap ) {
			CCommandPtr tmp( nodemap->GetNode( nameNode ));
			if( IsWritable(tmp)) {
				tmp->Execute();
				return true;
			}
		}	
	} catch (GenericException &e) {
		cerr << e.GetDescription();
	}
	
	return false;
}

void Basler::SetOwner(void* pOwner, BaslerGrabbedCallback pDisplayImageCallBack)
{
	try
	{
		m_pOwner = pOwner;
		m_DisplayImageCallBack = pDisplayImageCallBack;
	}
	catch (GenICam::GenericException &e)
	{	
		string str(e.GetDescription());
		std::cerr << str << std::endl;
	}
}

void Basler::OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
{
	try
	{
		bool lineStatus = false;
		GetBoolPara("LineStatus", lineStatus);

		if (ptrGrabResult->GrabSucceeded())
		{
			// 判断是哪个相机进入采集图像回调函数
			//if(camera.GetDeviceInfo().GetUserDefinedName() == "c1")
			if (ptrGrabResult->GetPixelType() == PixelType_Mono8)
			{
				int x = (int)width;
				int y = (int)height;
				pImageBufferMono = (uint8_t *)ptrGrabResult->GetBuffer();

				isColor = false;
				// 1、通过函数指针传出，转bitmap显示
				m_DisplayImageCallBack( m_pOwner, &x, &y, pImageBufferMono, isColor);
			
				//// 2、pylon自带显示窗体
				//Pylon::DisplayImage(0, ptrGrabResult);
			}
			else if (ptrGrabResult->GetPixelType() == PixelType_BayerGR8 || ptrGrabResult->GetPixelType() == PixelType_BayerRG8 
				  || ptrGrabResult->GetPixelType() == PixelType_BayerGB8 || ptrGrabResult->GetPixelType() == PixelType_BayerBG8
				  || ptrGrabResult->GetPixelType() == PixelType_YUV422packed)
			{
				int x,y;
				x = ptrGrabResult->GetWidth();
				y = ptrGrabResult->GetHeight();
				if(pImageBufferColor == NULL)
				{
					pImageBufferColor = (unsigned char*) malloc(ptrGrabResult->GetPayloadSize() * 3);
				}
				// 彩色相机进行RGB转换
				CImageFormatConverter converter;
				converter.OutputPixelFormat = PixelType_BGR8packed;
				converter.Convert( pImageBufferColor, ptrGrabResult->GetPayloadSize() * 3, ptrGrabResult);

				isColor = true;
				m_DisplayImageCallBack( m_pOwner, &x, &y, pImageBufferColor, isColor);
			}
		}
		else
		{
			std::string strMsg ="Grab faild!!" + ptrGrabResult->GetErrorDescription();
			std::cerr << strMsg << std::endl;
		}
	}
	catch (GenICam::GenericException &e)
	{	
		string  str(e.GetDescription());
		std::cerr << str << std::endl;
	}
}

void Basler::OnCameraDeviceRemoved(CInstantCamera& camera)
{
	try
	{
	    RemovedDeviceReconnect();
	}
	catch (GenICam::GenericException &e)
	{	
		string  str(e.GetDescription());
		std::cerr << str << std::endl;
	}
}

void Basler::RemovedDeviceReconnect()
{
	try
	{
		if ( mCamera->IsCameraDeviceRemoved())
		{
			// 提示相机掉线
			std::string strMsg = "UserID为“"+ mCamera->GetDeviceInfo().GetUserDefinedName() + "”的相机已掉线，请在25s内重连相机！！！";
			std::cerr << strMsg << std::endl;

		   // Now try to find the detached camera after it has been attached again:
			// Create a device info object for remembering the camera properties.
			CDeviceInfo info;

			// Remember the camera properties that allow detecting the same camera again.
			info.SetDeviceClass( mCamera->GetDeviceInfo().GetDeviceClass());
			info.SetSerialNumber( mCamera->GetDeviceInfo().GetSerialNumber());

			// Destroy the Pylon Device representing the detached camera device.
			// It cannot be used anymore.
			mCamera->DestroyDevice();

			// Ask the user to connect the same device.
			int loopCount = 100;	// 遍历相机100次，每次间隔250ms
		   // cout << endl << "Please connect the same device to the PC again (timeout " << loopCount / 4 << "s) " << endl;

			// Create a filter containing the CDeviceInfo object info which describes the properties of the device we are looking for.
			DeviceInfoList_t filter;
			filter.push_back( info);

			for ( ; loopCount > 0; --loopCount)
			{ 
				// Try to find the camera we are looking for.
				DeviceInfoList_t devices;
				if (  CTlFactory::GetInstance().EnumerateDevices(devices, filter) > 0 )
				{
					// Print two new lines, just for improving printed output.
					cout << endl << endl;

					// The camera has been found. Create and attach it to the Instant Camera object.
					mCamera->Attach( CTlFactory::GetInstance().CreateDevice( devices[0]));
					//Exit waiting
					break;
				}

				WaitObject::Sleep(250);
			}

			// If the camera has been found.
			if ( mCamera->IsPylonDeviceAttached())
			{
				// All configuration objects and other event handler objects are still registered.
				// The configuration objects will parameterize the camera device and the instant
				// camera will be ready for operation again.

				// Open the camera.
				nodemap = &mCamera->GetNodeMap();
				mCamera->RegisterConfiguration(this, RegistrationMode_Append, Cleanup_Delete);
				mCamera->RegisterImageEventHandler( this, RegistrationMode_Append, Cleanup_Delete);
				mCamera->Open();
				SetIntPara("GevHeartbeatTimeout", 6000);	
				// Now the Instant Camera object can be used as before.
				
				// 提示相机已重新连接
				std::string strMsg = "reconnected camera success, UserID:\""+ mCamera->GetDeviceInfo().GetUserDefinedName() + "\"";
				std::cerr << strMsg << std::endl;

			}
		}	 
	}
	catch (GenICam::GenericException &e)
	{	
		string  str(e.GetDescription());
		std::cerr << str << std::endl;
	}
}

// Adjust value to make it comply with range and increment passed.
//
// The parameter's minimum and maximum are always considered as valid values.
// If the increment is larger than one, the returned value will be: min + (n * inc).
// If the value doesn't meet these criteria, it will be rounded down to ensure compliance.
int64_t Basler::Adjust(int64_t val, int64_t minimum, int64_t maximum, int64_t inc)
{
	// Check the input parameters.
	if (inc <= 0)
	{
		// Negative increments are invalid.
		throw LOGICAL_ERROR_EXCEPTION("Unexpected increment %d", inc);
	}
	if (minimum > maximum)
	{
		// Minimum must not be bigger than or equal to the maximum.
		throw LOGICAL_ERROR_EXCEPTION("minimum bigger than maximum.");
	}

	// Check the lower bound.
	if (val < minimum)
	{
		return minimum;
	}

	// Check the upper bound.
	if (val > maximum)
	{
		return maximum;
	}

	// Check the increment.
	if (inc == 1)
	{
		// Special case: all values are valid.
		return val;
	}
	else
	{
		// The value must be min + (n * inc).
		// Due to the integer division, the value will be rounded down.
		return minimum + ( ((val - minimum) / inc) * inc );
	}
}
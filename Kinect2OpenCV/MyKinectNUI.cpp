#include "stdafx.h"
#include "MyKinectNUI.h"
#include "resource.h"

// http://wiki.livedoor.jp/pafuhana1213/d/Kinect%B8%F8%BC%B0SDK%20%A5%E1%A5%E2#

namespace {
	static const char* VideoWindow = "Video";
	static const char* DepthWindow = "Depth";
	static const char* SkeletonWindow = "Skeleton";
	static const char* PlayerWindow = "Player";

	static TCHAR* g_szAppTitle = _T("Kinect2OpenCV");
};

MyKinectNUI::MyKinectNUI(void)
{
	m_bVideoStreamOpened = false;
	m_bDepthStreamOpened = false;
	m_bSkeletonTracking = false;

	m_hInst = GetModuleHandle(0);
	m_PensTotal = NUI_SKELETON_COUNT;
	m_cameraAngle = NuiCameraElevationGetAngle(&m_cameraAngle);
}


MyKinectNUI::~MyKinectNUI(void)
{
}

int MyKinectNUI::MessageBoxResource(HWND hWnd, UINT nID, UINT nType)
{
	static TCHAR szRes[512];
	int nRet;

	LoadString(m_hInst, nID, szRes, _countof(szRes));
	nRet = MessageBox(m_hWnd, szRes, g_szAppTitle, nType);
	return nRet;
}

void MyKinectNUI::Initialize()	// Kinectセンサ初期化用（RGB/Depth/Skelton）
{
	m_hr = NuiInitialize(
		NUI_INITIALIZE_FLAG_USES_COLOR
		| NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX
		| NUI_INITIALIZE_FLAG_USES_SKELETON
		);

	if (FAILED(m_hr)) {
		MessageBoxResource(m_hWnd, IDS_ERROR_NUIINIT, MB_OK | MB_ICONHAND);
		exit(1);
	}
}

bool MyKinectNUI::InitializeImageStream()
{
	m_hNextVideoFrameEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);	// マニュアルリセット

	m_hr = NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		NUI_IMAGE_RESOLUTION_640x480,
		0,	// 現在は使われていない
		2,	// 最大４だが２で十分だそう
		m_hNextVideoFrameEvent,
		&m_videoStream
		);

	if (FAILED(m_hr)) {
		MessageBoxResource(m_hWnd, IDS_ERROR_VIDEOSTREAM, MB_OK | MB_ICONHAND);
		return false;
	}
	m_bVideoStreamOpened = true;
	return true;
}

bool MyKinectNUI::InitializeDepthStream()
{
	m_hNextDepthFrameEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);	// マニュアルリセット

	m_hr = NuiImageStreamOpen(
		NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
		NUI_IMAGE_RESOLUTION_320x240,
		0,
		2,
		m_hNextDepthFrameEvent,
		&m_depthStream);

	if (FAILED(m_hr)) {
		MessageBoxResource(m_hWnd, IDS_ERROR_DEPTHSTREAM, MB_OK | MB_ICONHAND);
		return false;
	}
	m_bDepthStreamOpened = true;
	return true;
}

bool MyKinectNUI::EnableSkeltonTracking()
{
	m_hNextSkeletonEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);	// マニュアルリセット
	m_hr = NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0);

	if (FAILED(m_hr)) {
		MessageBoxResource(m_hWnd, IDS_ERROR_SKELETONTRACKING, MB_OK | MB_ICONHAND);
		return false;
	}
	m_bSkeletonTracking = true;
	return true;
}

void MyKinectNUI::InitializeCvImage()
{
	if (m_bVideoStreamOpened) {
		cvNamedWindow(VideoWindow);
		m_videoFrame = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 4);
	}
	if (m_bDepthStreamOpened) {
		cvNamedWindow(DepthWindow);
		cvNamedWindow(PlayerWindow);
		m_depthFrame = cvCreateImage(cvSize(320, 240), IPL_DEPTH_16U, 1);
		m_playerFrame = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 1);
	}
	if (m_bSkeletonTracking) {
		cvNamedWindow(SkeletonWindow);
		m_skeletonFrame = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);
	}

	m_hWnd = FindWindowA(NULL, SkeletonWindow);
	m_SkeletonDC = GetDC(m_hWnd);
}

void MyKinectNUI::Shutdown()
{
	NuiShutdown();

	ReleaseDC(m_hWnd, m_SkeletonDC);

    if( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) ){
		WaitForSingleObject(m_hNextSkeletonEvent, INFINITE);
        CloseHandle( m_hNextSkeletonEvent );
        m_hNextSkeletonEvent = NULL;
    }
    if( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) ){
		WaitForSingleObject(m_hNextDepthFrameEvent, INFINITE);
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }
    if( m_hNextVideoFrameEvent && ( m_hNextVideoFrameEvent != INVALID_HANDLE_VALUE ) ){
		WaitForSingleObject(m_hNextVideoFrameEvent, INFINITE);
        CloseHandle( m_hNextVideoFrameEvent );
        m_hNextVideoFrameEvent = NULL;
    }

	cvDestroyAllWindows();
	if (m_videoFrame != NULL) {
		cvReleaseImage(&m_videoFrame);
	}
	if (m_depthFrame != NULL) {
		cvReleaseImage(&m_depthFrame);
	}
	if (m_skeletonFrame != NULL) {
		cvReleaseImage(&m_skeletonFrame);
	}
	if (m_playerFrame != NULL) {
		cvReleaseImage(&m_playerFrame);
	}
}

void MyKinectNUI::GotVideoAlertCVImg()
{
	const NUI_IMAGE_FRAME *pImageFrame = NULL;

	HRESULT hr = NuiImageStreamGetNextFrame(m_videoStream, 0, &pImageFrame);
	if (FAILED(hr)) {
		return;
	}

	NuiImageBuffer* pTexture = pImageFrame->pFrameTexture;
	KINECT_LOCKED_RECT lockedRect;
	pTexture->LockRect(0, &lockedRect, NULL, 0);
	if (lockedRect.Pitch != 0) {
		BYTE* pBuffer = (BYTE*)lockedRect.pBits;
		// Copy
		if (m_videoFrame != NULL) {
			CopyMemory(m_videoFrame->imageData, pBuffer, m_videoFrame->widthStep * m_videoFrame->height);
			cvShowImage(VideoWindow, m_videoFrame);
		}
	} else {
		OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
	}

	NuiImageStreamReleaseFrame(m_videoStream, pImageFrame);
	ResetEvent(m_hNextVideoFrameEvent);
}

void MyKinectNUI::GotDepthAlertCVImg()
{
	const NUI_IMAGE_FRAME *pImageFrame = NULL;

	HRESULT hr = NuiImageStreamGetNextFrame(m_depthStream, 0, &pImageFrame);
	if (FAILED(hr)) {
		return;
	}

	NuiImageBuffer* pTexture = pImageFrame->pFrameTexture;
	KINECT_LOCKED_RECT lockedRect;
	pTexture->LockRect(0, &lockedRect, NULL, 0);
	if (lockedRect.Pitch != 0) {
		BYTE* pBuffer = (BYTE*)lockedRect.pBits;

		// pBufferの上位13ビットが深度データ、下位3ビットがPlayerID

		// PlayerData
		if (m_playerFrame != NULL && m_depthFrame != NULL) {
			CopyMemory(m_depthFrame->imageData, pBuffer, m_depthFrame->widthStep * m_depthFrame->height);
			// 下位3ビットを取得
			cvAndS(m_depthFrame, cvScalarAll(7), m_depthFrame);

			//for (int i = 0; i < 240; i++) {
			//	for (int j = 0; j < 320; j++) {
			//		//m_playerFrame->;
			//	}
			//}

			cvConvertScale(m_depthFrame, m_playerFrame, 32);
			cvShowImage(PlayerWindow, m_playerFrame);
		}

		// DepthData
		if (m_depthFrame != NULL) {
			CopyMemory(m_depthFrame->imageData, pBuffer, m_depthFrame->widthStep * m_depthFrame->height);
			cvAndS(m_depthFrame, cvScalarAll(0xFFF8), m_depthFrame);
			cvShowImage(DepthWindow, m_depthFrame);
		}
	} else{
        OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
    }

	NuiImageStreamReleaseFrame(m_depthStream, pImageFrame);
	ResetEvent(m_hNextDepthFrameEvent);
}

void MyKinectNUI::DrawSkeletonSegmentCVImg( IplImage* Skeleton, int StartPoint, int EndPoint, int ColorIndex)
{
	cvLine(m_skeletonFrame,
		cvPoint(m_Points[StartPoint].x, m_Points[StartPoint].y),
		cvPoint(m_Points[EndPoint].x, m_Points[EndPoint].y),
		//m_CVPen[ColorIndex],
		g_CVJointColorTable[ColorIndex],
		4);
}

void MyKinectNUI::DrawSkeletonCVImg( IplImage* Skeleton, NUI_SKELETON_DATA * pSkel, HWND hWnd, int WhichSkeletonColor )
{
	HGDIOBJ hOldObj = SelectObject(m_SkeletonDC, m_Pen[WhichSkeletonColor % m_PensTotal]);

	//RECT rect;
	int width = 640;
	int height = 480;
	
	if (Skeleton) {
		PatBlt(m_SkeletonDC, 0, 0, width, height, BLACKNESS);
		cvZero(Skeleton);
	}

	int scaleX = width;	// scaling up to image coordinates
	int scaleY = height;
	float fx = 0, fy = 0;
	ushort depthZ = 0;
	int i;

	ushort headZ = 0;
	ushort rightHandZ = 0;

	for (i = 0; i < NUI_SKELETON_POSITION_COUNT; i++) {
		NuiTransformSkeletonToDepthImageF(pSkel->SkeletonPositions[i], &fx, &fy, &depthZ);
		m_Points[i].x = (int)(fx * scaleX + 0.5f);	// image座標系へ変換し、四捨五入
		m_Points[i].y = (int)(fy * scaleY + 0.5f);
		if (i == NUI_SKELETON_POSITION_HEAD) {
			headZ = depthZ;
		} else if (i == NUI_SKELETON_POSITION_HAND_RIGHT) {
			rightHandZ = depthZ;
		}
	}

	char buffer[128];
	sprintf(buffer, "Head:%uh Hand:%uh %s\n", headZ, rightHandZ, rightHandZ + 1800 < headZ ? "MOUSE" : "");
	OutputDebugStringA(buffer);

	// おしり、背中、頭
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_SPINE, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_SHOULDER_CENTER, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_HEAD, WhichSkeletonColor%m_PensTotal);

	// 肩の中心から左肩、左腕、左肘、左手首、左手にかけて
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT, WhichSkeletonColor%m_PensTotal);

	// 肩の中心から右肩、右腕、右肘、右手首、右手にかけて
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT, WhichSkeletonColor%m_PensTotal);

	// おしりから、左脚付け根、左膝、左足首、左足にかけて
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT, WhichSkeletonColor%m_PensTotal);

	// おしりから、右脚付け根、右膝、右足首、右足にかけて
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT, WhichSkeletonColor%m_PensTotal);
	DrawSkeletonSegmentCVImg(Skeleton, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT, WhichSkeletonColor%m_PensTotal);

	// Draw Joints in a different color
	for (i = 0; i < NUI_SKELETON_POSITION_COUNT; i++) {
		cvCircle(Skeleton, cvPoint(m_Points[i].x, m_Points[i].y), 5, g_CVJointColorTable[i], -1);
	}

	DeleteObject(hOldObj);
}

void MyKinectNUI::GotSkeletonAlertCVImg()
{
	if (m_skeletonFrame == NULL) {
		return;
	}

	NUI_SKELETON_FRAME nuiSkeletonFrame;

	HRESULT hr = NuiSkeletonGetNextFrame(0, &nuiSkeletonFrame);
	if (FAILED(hr)) {
		return;
	}

	bool bFoundSkeleton = false;
	for (int i = 0; i < NUI_SKELETON_COUNT; i++) {
		if (nuiSkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED) {
			bFoundSkeleton = true;
		}
	}

	if (!bFoundSkeleton) {
		return;
	}

	NuiTransformSmooth(&nuiSkeletonFrame, NULL);

	// we found a skeleton, restart the timer
	m_bScreenBlanked = false;
	m_LastSkeletonFoundTime = -1;

	// draw each skeleton color according to the slot within they are found.
	bool bBrank = true;
	for (int i = 0; i < NUI_SKELETON_COUNT; i++) {
		if (nuiSkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED) {
			DrawSkeletonCVImg(m_skeletonFrame, &nuiSkeletonFrame.SkeletonData[i], m_hWnd/*GetDlgItem(m_hWnd, IDC_SKELETALVIEW)*/, i);
			bBrank = false;
		}
	}

	cvShowImage(SkeletonWindow, m_skeletonFrame);

	// Nui_DoDoubleBuffer(GetDlgItem(m_hWnd, IDC_SKELETALVIEW), m_SkeletonDC);
	ResetEvent(m_hNextSkeletonEvent);
}

RGBQUAD MyKinectNUI::ShortToQuadDepth(USHORT s)
{
	USHORT RealDepth = (s & 0xFFF8) >> 3;
	USHORT Player = s & 7;

    // transform 13-bit depth information into an 8-bit intensity appropriate
    // for display (we disregard information in most significant bit)
	BYTE l = 255 - (BYTE)(256 * RealDepth / 0xFFF);

	RGBQUAD q = {0};

	switch (Player) {
	case 0:
		q.rgbRed = l / 2;
		q.rgbBlue = l / 2;
		q.rgbGreen = l / 2;
		break;
	case 1:
		q.rgbRed = l;
		break;
	case 2:
		q.rgbGreen = l;
		break;
	case 3:
		q.rgbRed = l / 4;
		q.rgbBlue = l;
		q.rgbGreen = l;
		break;
	case 4:
		q.rgbRed = l;
		q.rgbBlue = l / 4;
		q.rgbGreen = l;
		break;
	case 5:
		q.rgbRed = l;
		q.rgbBlue = l;
		q.rgbGreen = l / 4;
		break;
	case 6:
		q.rgbRed = l / 2;
		q.rgbBlue = l / 2;
		q.rgbGreen = l;
		break;
	case 7:
		q.rgbRed = 255 - l / 2;
		q.rgbBlue = 255 - l / 2;
		q.rgbGreen = 255 - l / 2;
		break;
	}

	return q;
}

void MyKinectNUI::SetCameraAngle(long angle)
{
	if (NUI_CAMERA_ELEVATION_MINIMUM <= angle && angle <= NUI_CAMERA_ELEVATION_MAXIMUM) {
		NuiCameraElevationSetAngle(angle);
		m_cameraAngle = angle;
	}
}

void MyKinectNUI::SetCameraAngleDiff(long diff)
{
	int angle = m_cameraAngle + diff;
	SetCameraAngle(angle);
}


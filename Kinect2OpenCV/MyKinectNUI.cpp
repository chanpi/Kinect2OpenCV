#include "stdafx.h"
#include "MyKinectNUI.h"

// http://wiki.livedoor.jp/pafuhana1213/d/Kinect%B8%F8%BC%B0SDK%20%A5%E1%A5%E2#

namespace {
	static const char* VideoWindow = "Video";
	static const char* DepthWindow = "Depth";
	static const char* SkeletonWindow = "Skeleton";
	static const char* PlayerWindow = "Player";
};

MyKinectNUI::MyKinectNUI(void)
{
}


MyKinectNUI::~MyKinectNUI(void)
{
}

void MyKinectNUI::Initialize()	// Kinectセンサ初期化用（RGB/Depth/Skelton）
{
	m_hr = NuiInitialize(
		NUI_INITIALIZE_FLAG_USES_COLOR
		| NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX
		| NUI_INITIALIZE_FLAG_USES_SKELETON
		);

	if (FAILED(m_hr)) {
		//MessageBox();
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
		//MessageBox();
		return false;
	}
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
		//MessageBox();
		return false;
	}
	return true;
}

bool MyKinectNUI::EnableSkeltonTracking()
{
	m_hNextSkeletonEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);	// マニュアルリセット
	m_hr = NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0);

	if (FAILED(m_hr)) {
		//MessageBox();
		return false;
	}
	return true;
}

void MyKinectNUI::InitializeCvImage()
{
	cvNamedWindow(VideoWindow);
	cvNamedWindow(DepthWindow);
	//cvNamedWindow(SkeletonWindow);
	cvNamedWindow(PlayerWindow);

	m_videoFrame = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 4);
	m_depthFrame = cvCreateImage(cvSize(320, 240), IPL_DEPTH_16U, 1);
	m_playerFrame = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 1);
	//m_skeletonFrame = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);
}

void MyKinectNUI::Shutdown()
{
	NuiShutdown();

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
			cvConvertScale(m_depthFrame, m_playerFrame, 32);
			cvShowImage(PlayerWindow, m_playerFrame);
		}

		// DepthData
		if (m_depthFrame != NULL) {
			CopyMemory(m_depthFrame->imageData, pBuffer, m_depthFrame->widthStep * m_depthFrame->height);
			cvAndS(m_depthFrame, cvScalarAll(0xFFF8), m_depthFrame);
			cvShowImage(DepthWindow, m_depthFrame);
		}
	}
    else{
        OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
    }

	NuiImageStreamReleaseFrame(m_depthStream, pImageFrame);
	ResetEvent(m_hNextDepthFrameEvent);
}

void MyKinectNUI::DrawSkeletonSegmentCVImg( IplImage* Skeleton, int StartPoint, int EndPoint, int ColorIndex)
{
	//cvLine(m_skeletonFrame,
	//	cvPoint(m_poi
}

void MyKinectNUI::DrawSkeletonCVImg( IplImage* Skeleton, NUI_SKELETON_DATA * pSkel, HWND hWnd, int WhichSkeletonColor )
{

}

void MyKinectNUI::GotSkeletonAlertCVImg()
{
	ResetEvent(m_hNextSkeletonEvent);
}

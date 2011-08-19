#pragma once

#include "stdafx.h"
//#include <iostream>
//#include <Windows.h>

///////////////////// Kinect /////////////////////
#include <MSR_NuiApi.h>                 // 全てのNUIAPI、基本的な初期化とアクセス定義
#include <MSR_NuiImageCamera.h>         // NUI画像とカメラサービス関係のAPI定義( NuiCameraXxx, NuiImageXxx )
#include <MSR_NuiProps.h>               // NUIプロパティ列挙のためのAPI定義
#include <MSR_NuiSkeleton.h>            // NUIスケルトンのためのAPI定義( NuiSkeletonXxx, NuiTransformXxx )
#include <MSRKinectAudio.h>             // オーディオAPI定義
#include <NuiImageBuffer.h>             // DirextX9テクスチャに似た作用のためのフレームバッファを定義

#pragma comment(lib, "MSRKinectNUI.lib")

///////////////////// OpenCV /////////////////////
#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>

#ifdef _DEBUG || DEBUG
#pragma comment(lib, "opencv_core220d.lib")
#pragma comment(lib, "opencv_imgproc220d.lib")
#pragma comment(lib, "opencv_highgui220d.lib")
#else
#pragma comment(lib, "opencv_core220.lib")
#pragma comment(lib, "opencv_imgproc220.lib")
#pragma comment(lib, "opencv_highgui220.lib")
#endif

class MyKinectNUI
{
public:
	MyKinectNUI(void);
	~MyKinectNUI(void);

	HRESULT m_hr;
	HANDLE m_videoStream;
	HANDLE m_depthStream;
	HANDLE m_skeletonStream;
	HANDLE m_hNextVideoFrameEvent;
	HANDLE m_hNextDepthFrameEvent;
	HANDLE m_hNextSkeletonEvent;

	IplImage* m_videoFrame;
	IplImage* m_depthFrame;
	IplImage* m_skeletonFrame;
	IplImage* m_playerFrame;

	void Initialize();	// Kinectセンサ初期化用（RGB/Depth/Skelton）
	bool InitializeImageStream();
	bool InitializeDepthStream();
	bool EnableSkeltonTracking();
	void Shutdown();

	void InitializeCvImage();

	void GotVideoAlertCVImg();
    void GotDepthAlertCVImg();
    void DrawSkeletonSegmentCVImg( IplImage* Skeleton, int StartPoint, int EndPoint, int ColorIndex);
    void DrawSkeletonCVImg( IplImage* Skeleton, NUI_SKELETON_DATA * pSkel, HWND hWnd, int WhichSkeletonColor );
    void GotSkeletonAlertCVImg( );
};


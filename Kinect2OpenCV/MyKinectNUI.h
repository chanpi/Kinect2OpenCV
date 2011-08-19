#pragma once

#include "stdafx.h"
//#include <iostream>
//#include <Windows.h>

///////////////////// Kinect /////////////////////
#include <MSR_NuiApi.h>                 // �S�Ă�NUIAPI�A��{�I�ȏ������ƃA�N�Z�X��`
#include <MSR_NuiImageCamera.h>         // NUI�摜�ƃJ�����T�[�r�X�֌W��API��`( NuiCameraXxx, NuiImageXxx )
#include <MSR_NuiProps.h>               // NUI�v���p�e�B�񋓂̂��߂�API��`
#include <MSR_NuiSkeleton.h>            // NUI�X�P���g���̂��߂�API��`( NuiSkeletonXxx, NuiTransformXxx )
#include <MSRKinectAudio.h>             // �I�[�f�B�IAPI��`
#include <NuiImageBuffer.h>             // DirextX9�e�N�X�`���Ɏ�����p�̂��߂̃t���[���o�b�t�@���`

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

	void Initialize();	// Kinect�Z���T�������p�iRGB/Depth/Skelton�j
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


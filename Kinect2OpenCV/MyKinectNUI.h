#pragma once

#include "stdafx.h"

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

static const CvScalar g_CVJointColorTable[NUI_SKELETON_POSITION_COUNT] = 
{
	cvScalar(155, 176, 169),        // NUI_SKELETON_POSITION_HIP_CENTER
	cvScalar(155, 176, 169),        // NUI_SKELETON_POSITION_SPINE
	cvScalar( 29, 230, 168),        // NUI_SKELETON_POSITION_SHOULDER_CENTER
	cvScalar(  0,   0, 200),        // NUI_SKELETON_POSITION_HEAD
	cvScalar( 33,  84,  79),        // NUI_SKELETON_POSITION_SHOULDER_LEFT
	cvScalar( 42,  33,  84),        // NUI_SKELETON_POSITION_ELBOW_LEFT
	cvScalar(  0, 126, 255),        // NUI_SKELETON_POSITION_WRIST_LEFT
	cvScalar(  0,  86, 215),        // NUI_SKELETON_POSITION_HAND_LEFT
	cvScalar( 84,  79,  33),        // NUI_SKELETON_POSITION_SHOULDER_RIGHT
	cvScalar( 84,  33,   3),        // NUI_SKELETON_POSITION_ELBOW_RIGHT
	cvScalar(243, 109,  77),        // NUI_SKELETON_POSITION_WRIST_RIGHT
	cvScalar(243,  69,  37),        // NUI_SKELETON_POSITION_HAND_RIGHT
	cvScalar(243, 109,  77),        // NUI_SKELETON_POSITION_HIP_LEFT
	cvScalar( 84,  33,  69),        // NUI_SKELETON_POSITION_KNEE_LEFT
	cvScalar(122, 170, 229),        // NUI_SKELETON_POSITION_ANKLE_LEFT
	cvScalar(  0, 126, 255),        // NUI_SKELETON_POSITION_FOOT_LEFT
	cvScalar(213, 165, 181),        // NUI_SKELETON_POSITION_HIP_RIGHT
	cvScalar( 76,  222, 71),        // NUI_SKELETON_POSITION_KNEE_RIGHT
	cvScalar(156, 228, 245),        // NUI_SKELETON_POSITION_ANKLE_RIGHT
	cvScalar(243, 109, 77),         // NUI_SKELETON_POSITION_FOOT_RIGHT
};

class MyKinectNUI
{
public:
	MyKinectNUI(void);
	~MyKinectNUI(void);

	void Initialize();	// Kinect�Z���T�������p�iRGB/Depth/Skelton�j
	bool InitializeImageStream();
	bool InitializeDepthStream();
	bool EnableSkeltonTracking();
	void InitializeCvImage();
	void Shutdown();

	void GotVideoAlertCVImg();
    void GotDepthAlertCVImg();
    void GotSkeletonAlertCVImg( );

	// camera�̋p��ύX
	void SetCameraAngle(long angle);
	void SetCameraAngleDiff(long diff);

private:
	HRESULT m_hr;
	HANDLE m_videoStream;
	HANDLE m_depthStream;
	HANDLE m_skeletonStream;
	HANDLE m_hNextVideoFrameEvent;
	HANDLE m_hNextDepthFrameEvent;
	HANDLE m_hNextSkeletonEvent;

	HINSTANCE m_hInst;
	HWND m_hWnd;
	int m_LastSkeletonFoundTime;
	bool m_bScreenBlanked;
	HDC m_SkeletonDC;
	HPEN m_Pen[NUI_SKELETON_COUNT];
	int m_PensTotal;
	POINT m_Points[NUI_SKELETON_POSITION_COUNT];
	CvScalar m_CVPen[NUI_SKELETON_COUNT];

	long m_cameraAngle;

	bool m_bVideoStreamOpened;
	bool m_bDepthStreamOpened;
	bool m_bSkeletonTracking;

	IplImage* m_videoFrame;
	IplImage* m_depthFrame;
	IplImage* m_skeletonFrame;
	IplImage* m_playerFrame;

    void DrawSkeletonSegmentCVImg( IplImage* Skeleton, int StartPoint, int EndPoint, int ColorIndex);
    void DrawSkeletonCVImg( IplImage* Skeleton, NUI_SKELETON_DATA * pSkel, HWND hWnd, int WhichSkeletonColor );
	RGBQUAD ShortToQuadDepth(USHORT s);

	int MessageBoxResource(HWND hWnd, UINT nID, UINT nType);
};


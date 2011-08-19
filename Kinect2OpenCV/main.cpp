#include "stdafx.h"
#include "MyKinectNUI.h"

//int WINAPI _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)

static MyKinectNUI g_kinect;

void InitializeKinectNui() {
	g_kinect.Initialize();
	g_kinect.InitializeImageStream();
	g_kinect.InitializeDepthStream();
	g_kinect.InitializeCvImage();
}

void MainLoop() {
	int key = 0;

	while (key != 'q') {
		g_kinect.GotVideoAlertCVImg();
		g_kinect.GotDepthAlertCVImg();
		key = cvWaitKey(33);
	}
}

int _tmain(void)
{
	InitializeKinectNui();
	MainLoop();

	g_kinect.Shutdown();
	return 0;
}
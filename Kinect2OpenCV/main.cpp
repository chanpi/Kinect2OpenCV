#include "stdafx.h"
#include "MyKinectNUI.h"
#include "resource.h"

#define CAMERA_ANGLE_RANGE	3

INT_PTR CALLBACK CameraWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static MyKinectNUI g_kinect;

bool InitializeKinectNui() {
	g_kinect.Initialize();
	if (!g_kinect.InitializeImageStream()) {
		return false;
	}
	if (!g_kinect.InitializeDepthStream()) {
		return false;
	}
	if (!g_kinect.EnableSkeltonTracking()) {
		return false;
	}
	g_kinect.InitializeCvImage();
	
	return true;
}

void MainLoop() {
	int key = 0;

	while (key != 'q') {
		g_kinect.GotVideoAlertCVImg();
		g_kinect.GotDepthAlertCVImg();
		g_kinect.GotSkeletonAlertCVImg();
		key = cvWaitKey(33);
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	HWND hCameraDialog = NULL;
	if (InitializeKinectNui()) {
		hCameraDialog = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_CAMERA), HWND_DESKTOP, (DLGPROC)CameraWndProc, NULL);
		MainLoop();
	}
	g_kinect.Shutdown();
	return EXIT_SUCCESS;
}

INT_PTR CALLBACK CameraWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_UP:
			g_kinect.SetCameraAngleDiff(CAMERA_ANGLE_RANGE);
			break;

		case IDC_BUTTON_DOWN:
			g_kinect.SetCameraAngleDiff(-CAMERA_ANGLE_RANGE);
			break;

		case IDC_BUTTON_DEFAULT:
			g_kinect.SetCameraAngle(0);
			break;

		case IDC_BUTTON_MAXIMUM:
			g_kinect.SetCameraAngle(NUI_CAMERA_ELEVATION_MAXIMUM);
			break;

		case IDC_BUTTON_MINIMUM:
			g_kinect.SetCameraAngle(NUI_CAMERA_ELEVATION_MINIMUM);
			break;

		case IDOK:
		case IDCANCEL:
			EndDialog(hWnd, LOWORD(wParam));
		}
		return TRUE;

	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hWnd, LOWORD(wParam));
		return TRUE;
	}
	return FALSE;
}
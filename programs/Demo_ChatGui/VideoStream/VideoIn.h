//#include "../wintypes.h"

void* VideoInInit(int width, int height, float FramesPerSec, int video_device, const char* device_name);
bool VideoInGet(void* handle, unsigned char **yuv);
void VideoInExit(void* handle);
void VideoInDemoTime(void* handle, unsigned char *YUY2_Buffer);
//BOOL VideoInDlgSource(void* handle, HWND wnd);
float VideoInGetFPS(void* handle);

#ifndef WINDOWS
#include <QList>
#include <QString>
QList<QString> VideoInGetDevices();
#endif

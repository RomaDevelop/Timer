#include "MyQWidgetLib.h"

#include <windows.h>

void MyQWidgetLib::SetTopMost(QWidget *w, bool topMost)
{
	HWND hwnd = reinterpret_cast<HWND>(w->winId());
	SetWindowPos(hwnd, topMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

// joystick.h

#ifndef _JOYSTICK_H_
#define	_JOYSTICK_H_



extern BOOL InitDirectInput(HINSTANCE hInst, HWND hWnd);
extern BOOL SetAcquire(BOOL bActive);
extern BOOL UpdateInputState(DIJOYSTATE &js);
extern BOOL FreeDirectInput();



#endif //_JOYSTICK_H_


#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <GL\wglext.h>
#define _ISOC99_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
//{ STB image stuff
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_JPEG
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_ASSERT(x)
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_SIMD
#include "stb_image.h"
//}

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define GAMEFUNC     __attribute__ ((section(".game")))
#define OBJCODEATTR  __attribute__ ((section(".game")))
#define _stdcall     __attribute__ ((stdcall))

#define game_width  224
#define game_height 288
#define game_rate   60.0f

typedef unsigned int uint;

int   argc;
char**argv, env;

uint_fast8_t active;

uint_fast16_t	const
	__width   =  game_width ,
	__height  =  game_height,
	__scale   = 2;
const float rate  =     game_rate;
const float delta = 1.f/game_rate;

long long frameBase, frameTime;
long long LAG_0, LAG_1;
long long*CLOCK_FREQ = (long long*)0x7FFE0300;
HANDLE TIMER;

//{
static	PIXELFORMATDESCRIPTOR pfd =
{
	sizeof(PIXELFORMATDESCRIPTOR), 1,
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_OPENGL | // unneeded flag looks like, enables OpenGL detection by other apps
	PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,
	32,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	24,
	0, 0,
	PFD_MAIN_PLANE,
	0, 0, 0, 0 // is this ever used
};
uint_fast32_t pxf;
static HWND hWnd;
static WNDCLASS wc;
static HDC hDC;
static HGLRC hRC;
static MSG msg;
//}

LPCSTR	__title = "gamy",
		__class = "AUXstatic";

char __itoatmp[11];
_stdcall char*itoa_cool(int i) {
	itoa(i,__itoatmp,10);
	return __itoatmp;
}

// maybe try out:
// copy output from getkeyboard state
// and copy all 256 states to fit in
// 8 bytes using bitstream
_stdcall USHORT key(int vKey) {
	if (GetFocus() == hWnd)
		return GetAsyncKeyState(vKey);
}
// what is this called where its like a textbox
// and wasnt there a way to check just the keypress
// once and not have it repeating if held, or am i dreaming
// why even make it a short if theres only bits 1 and 0x8000
#define KST8_DEFAULT	1
#define KST8_HOLD		0x8000

void init();
void loop();
void quit();

#include "tex.c" // JUST USE A PROJECT, STUPID!!
#include "game.c"

#ifdef DBG
FILE*CON;
#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049
GLint GL_mem_total = 0;
GLint GL_mem_free = 0;
#endif

//int _winstart() { WinMain(0,0,0,5); exit(0); }

LRESULT CALLBACK WinProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain (HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR lpCmdLine,
					int iCmdShow)
{
	#ifdef DBG
	AllocConsole();
	CON = freopen("CONOUT$", "w", stdout);
	__getmainargs(&argc,&argv,&env,0);
	#endif
	
	float theta = 0.f;
	
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)0;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = __class;
	RegisterClass(&wc);
	
	hWnd = CreateWindow(
		__class, __title, 
		WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE,
		(GetSystemMetrics(SM_CXSCREEN) >> 1) - ((__width *__scale) >> 1),
		(GetSystemMetrics(SM_CYSCREEN) >> 1) - ((__height*__scale) >> 1),
		(__width *__scale) + (GetSystemMetrics(SM_CXEDGE) << 1) + 2,
		(__height*__scale) + (GetSystemMetrics(SM_CYFIXEDFRAME) << 1)
		+ GetSystemMetrics(SM_CYCAPTION)
		+ GetSystemMetrics(SM_CXFIXEDFRAME) - 3,
		NULL, NULL, hInstance, NULL);
	
	hDC = GetDC(hWnd);
	pxf = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, pxf, &pfd);
	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC,hRC);
	
	glGenTextures(TEX_COUNT, Textures);
	
	for(uint_fast8_t i = 0; i < TEX_COUNT; i++)
	{
		tex[i] = loadTEX(texFn[i]);
		texsize += tex[i].w * tex[i].h << 2;
		glBindTexture(GL_TEXTURE_2D, Textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
			tex[i].w, tex[i].h,
			0, GL_RGBA, GL_UNSIGNED_BYTE, tex[i].data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812D); // GL_CLAMP_TO_BORDER
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &GLtxSz[i].X); // forgot, overwrites Y, oops lol
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &GLtxSz[i].Y);
		glBindTexture(GL_TEXTURE_2D, 0);
		free(tex[i].data);
	}
	active = 3;
	
	init();
	
	//this func for me just assigns a value from a static pointer that never changes place
	//QueryPerformanceFrequency((LARGE_INTEGER*)&CLOCK_FREQ);
	frameBase = -((float)(*CLOCK_FREQ)/rate);
	while (active)
	{
		if(key(VK_RETURN) & 1)
			active ^= 2;
		if(key(VK_ESCAPE))
			active = 0;
		if(active & 2)
			loop();
		SwapBuffers(hDC);
		
		if(!key(VK_TAB))
		{
			TIMER = CreateWaitableTimer(NULL, TRUE, NULL);
			QueryPerformanceCounter((LARGE_INTEGER*)&frameTime);
			frameTime += frameBase-LAG_1;
			SetWaitableTimer(TIMER, (LARGE_INTEGER*)&frameTime, 0, NULL, NULL, 0);
			WaitForSingleObject(TIMER, INFINITE);
			QueryPerformanceCounter((LARGE_INTEGER*)&LAG_1);
			CloseHandle(TIMER);
		}
		
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	quit();
	
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);
	exit(0);
	DestroyWindow(hWnd); // slows down exit >:(

	return msg.wParam;
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT message,
							WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_QUIT:
	case WM_CLOSE:
	case WM_DESTROY:
	{
		active = 0;
		PostQuitMessage(0);
		return 0;
	} break;
	case WM_SYSCOMMAND:
		if(wParam==SC_KEYMENU && (lParam>>16)<=0) return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}



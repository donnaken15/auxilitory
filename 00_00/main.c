
#include <Windows.h>
#include <GL\gl.h>
#include <GL\glu.h>
#include <GL\glut.h>
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

#define assert(x) printf("%u",__LINE__)

#define GAMEFUNC     __attribute__ ((section(".game")))
#define OBJCODEATTR  __attribute__ ((section(".game")))
#define _stdcall     __attribute__ ((stdcall))

unsigned short __tcc_fpu_control = 0x137f;
unsigned short __tcc_int_fpu_control = 0x137f | 0x0c00;

int   argc;
char**argv, env;

// {
inline float itof ( int i) {
	float a = i;
	return a;
}
inline float uitof(UINT i) {
	float a = i;
	return a;
}
inline  int ftoi (float i) {
	int a = i;
	return a;
}
inline UINT ftoui(float i) {
	int a = i;
	return a;
}
// }

//{
static	PIXELFORMATDESCRIPTOR pfd;
uint_fast32_t pxf;
static HWND hWnd;
static WNDCLASSEX wc;
static HDC hDC;
static HGLRC hRC;
//}

LPCSTR	__title = "Xevious",
		__class = "AUX";

typedef struct { // always RGBA
	int_fast32_t w,h;
	unsigned char*data;
} TEX;
_stdcall TEX  loadTEX(LPSTR fname)
{
	int x,y,n;
	unsigned char *data = stbi_load(fname, &x, &y, &n, 4);
	TEX newtex;
	newtex.w = x;
	newtex.h = y;
	newtex.data = data;
	return newtex;
}
uint_fast32_t texsize = 0;

_stdcall void drawImage(
	GLuint texn,
	USHORT l, USHORT t,
	USHORT r, USHORT b,
	short x, short y,
	short w, short h)
{
	int_fast32_t tw, th;

	r += !r;
	b += !b;
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, texn);
		// is this wasteful                                         V
		// as if everything else isnt more resource intensive
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &tw);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);
		glTranslatef(x, y, 0);
		glBegin(GL_QUADS);
			glTexCoord2f(uitof(l) / tw, (uitof(t)+uitof(b)) / th);
			glVertex2i(0, h);
			glTexCoord2f(uitof(l) / tw, uitof(t) / th);
			glVertex2i(0, 0);
			glTexCoord2f((uitof(l)+uitof(r)) / tw, uitof(t) / th);
			glVertex2i(w, 0);
			glTexCoord2f((uitof(l)+uitof(r)) / tw, (uitof(t)+uitof(b)) / th);
			glVertex2i(w, h);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
}

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

char __itoatmp[11];
_stdcall char*itoa_cool(int i) {
	itoa(i,__itoatmp,10);
	return __itoatmp;
}

typedef void (*objfunc)(int);
// ???  ^
typedef struct {
	/* 0x00, 0x04 */ float x,y;
	/* 0x08  ??-> */ uint_fast16_t id	: 10; // is this bit field thing working
	/* 0x0A       */ uint_fast16_t time;
	/* 0x0C       */ uint_fast8_t active : 1;
	/* 0x0D maybe */ unsigned char props[0xB];
	/* 0x18, 0x1C */ objfunc step, die;
} object;
#define MAX_OBJECTS 256
object*objects;
_stdcall UINT objectcount()
{
	UINT a = 0;
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		if (objects[i].active)
			a++;
	}
	return a;
}
_stdcall void kill(int id)
{
	if(objects[id].active)
	{
		(*objects[id].die)(id);
		objects[id].active = 0;
	}
}
_stdcall inline UINT finddeadobj()
{
	for (int i = 0; i < MAX_OBJECTS; i++)
		if (!objects[i].active)
			return i;
}
_stdcall object*newobj(INT x, INT y, objfunc create, objfunc step, objfunc die)
{
	UINT newi = finddeadobj();
	memset(&objects[newi], 0, sizeof(object));
	objects[newi].id = newi;
	objects[newi].active = 1;
	objects[newi].x = x;
	objects[newi].y = y;
	objects[newi].step = step;
	objects[newi].die = die;
	(*create)(newi);
	return&objects[newi];
}
_stdcall object*addobj(UINT x, UINT y, object*obj, objfunc create)
{
	UINT newi = finddeadobj();
	memset(&objects[newi], 0, sizeof(object));
	objects[newi].id = newi;
	objects[newi].active = 1;
	objects[newi].x = x;
	objects[newi].y = y;
	(*create)(newi);
	return&objects[newi];
}

HANDLE TIMER;
unsigned long long frameBase, frameTime;
unsigned long long LAG_0, LAG_1, LAG_2, LAG_3;
unsigned long long*CLOCK_FREQ = (unsigned unsigned long*)0x7FFE0300;

_stdcall void consoleText(USHORT X, USHORT Y, LPCSTR TEXT);
// wth
#include "game.c"

_stdcall void consoleText(USHORT X, USHORT Y, LPCSTR TEXT)
{
	float lastColors[4];
	glGetFloatv(GL_CURRENT_COLOR,lastColors);
	USHORT XX = X, YY = Y, cL, cT;
	BYTE*CURCHAR = (BYTE*)TEXT;
	do
	{
		if (*CURCHAR != '\n')
		{
			cL = (*CURCHAR & 15) << 3;
			cT = ftoui(floor(uitof(*CURCHAR) / 16.0f)) << 3;
			glColor4f(0,0,0,lastColors[3]);
			drawImage(Textures[TEX_TEXT], cL, cT, 8, 8, XX+1, YY+1, 8, 8);
			glColor4f(lastColors[0],lastColors[1],lastColors[2],lastColors[3]);
			drawImage(Textures[TEX_TEXT], cL, cT, 8, 8, XX, YY, 8, 8);
			XX += 8;
		}
		else
		{
			XX = X;
			YY += 8;
		}
		CURCHAR++;
	} while (*CURCHAR);
}

int _winstart() { WinMain(0,0,0,5); exit(0); }

#ifdef DBG
FILE*CON;
#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049
GLint GL_mem_total = 0;
GLint GL_mem_free = 0;
#endif

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

_stdcall int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	#ifdef DBG
	AllocConsole();
	CON = freopen("CONOUT$", "w", stdout);
	__getmainargs(&argc,&argv,&env,0);
	#endif

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)0;
	wc.lpszClassName = __class;

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL,
		__class,
		__title,
		WS_OVERLAPPED | WS_SYSMENU,
		(GetSystemMetrics(SM_CXSCREEN) >> 1) - ((__width *__scale) >> 1),
		(GetSystemMetrics(SM_CYSCREEN) >> 1) - ((__height*__scale) >> 1),
		(__width *__scale) + (GetSystemMetrics(SM_CXEDGE) << 1) + 2,
		(__height*__scale) + (GetSystemMetrics(SM_CYFIXEDFRAME) << 1)
		+ GetSystemMetrics(SM_CYCAPTION)
		+ GetSystemMetrics(SM_CXFIXEDFRAME) - 3,
		NULL,
		NULL,
		hInstance,
		NULL);

	pfd.dwFlags |=
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | // unneeded flag looks like, enables OpenGL detection by other apps
		PFD_DOUBLEBUFFER;
	hDC = GetDC(hWnd);
	pxf = ChoosePixelFormat(hDC, &pfd);
	SetPixelFormat(hDC, pxf, &pfd);
	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);
	
	for(uint_fast8_t i = 0; i < TEX_COUNT; i++)
	{
		tex[i] = loadTEX(texFn[i]);
		texsize += tex[i].w * tex[i].h << 2;
	}
	
	glGenTextures(TEX_COUNT, Textures);
	
	for(uint_fast8_t i = 0; i < TEX_COUNT; i++)
	{
		glBindTexture(GL_TEXTURE_2D, Textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
			tex[i].w, tex[i].h,
			0, GL_RGBA, GL_UNSIGNED_BYTE, tex[i].data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812D); // GL_CLAMP_TO_BORDER
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		free(tex[i].data);
	}

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	objects = calloc(sizeof(object),MAX_OBJECTS);

	ShowWindow(hWnd, nCmdShow);

	static MSG msg;

	init();
	
	//this func for me just assigns a value from a static pointer that never changes place
	//QueryPerformanceFrequency((LARGE_INTEGER*)&CLOCK_FREQ);
	frameBase = -ftoi(uitof(*CLOCK_FREQ)/rate);
	while (active)
	{
		if(key(VK_RETURN) & 1)
			active ^= 2;
		
		// part of this func uses RDTSC LOOOOOOOL
		QueryPerformanceCounter((LARGE_INTEGER*)&LAG_2);
		if(active & 2)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//glColor4ub(255, 255, 255, 255);
			
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, __width, __height, 0, 0, 1e-38);
			//glViewport(0, 0, __width*__scale, __height*__scale);
			
			loop();
			
			QueryPerformanceCounter((LARGE_INTEGER*)&LAG_3);
			#ifdef DBG
			#if (1)
			{ // log to console
				puts(
					"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
				);
				glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, 
					&GL_mem_total);
				glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, 
					&GL_mem_free);
				printf(//"%0u\n"
				"LEVEL: %u\n"
				"TIME: %u\n"
				"OBJ CNT: %4u OF %4u\n"
				"OBJS ARE %u BYTES\n"
				"MEM: %7u / %7u\n"
				"ERROR: %6u\n"
				"TEXSZ: %u\n"
				"GL MEM:\n%8u / %8u\n"
				"GL ERROR: %6u\n",
					((wy-768) >> 12),
					time,
					objectcount(),
					MAX_OBJECTS,
					sizeof(object),
					objectcount()*sizeof(object),
					MAX_OBJECTS*sizeof(object),
					GetLastError(),
					texsize,
					GL_mem_total-GL_mem_free,
					GL_mem_total,
					glGetError());
				printf(
					"LAG us: %7d\n"
					" slept: %7d\n"
					"total = %7d / %7d\n"
					"off by  %7d\n"
					"WHY PRINTF %u\n"
					"WHY PRINTF %u\n"
					"matching rate: %5.4ffps, %5.7fms\n",
					(int)(LAG_3-LAG_2),
					(int)(LAG_1-LAG_0),
					(int)(LAG_1-LAG_0+LAG_3-LAG_2),
					ftoi(uitof(*CLOCK_FREQ)/rate),
					(ftoi(uitof(*CLOCK_FREQ)/rate)-(LAG_1-LAG_0+LAG_3-LAG_2)),
					rate, 1.0f/rate);
				//QueryPerformanceCounter((LARGE_INTEGER*)&LAG_1);
			}
			#else
			{ // draw on game
				consoleText(0,24,argv[0]);
				consoleText(8,32,"LEVEL: ");
				consoleText(64,32,itoa_cool((wy-768)>>12));
				consoleText(8,40,"TIME: ");
				consoleText(56,40,itoa_cool(time));
				consoleText(8,128-64,"OBJ CNT:	     OF");
				consoleText(8+(9<<3),128-64,itoa_cool(objectcount()));
				consoleText(8+(17<<3),128-64,STR(MAX_OBJECTS));
				consoleText(8,144-64-8,"OBJS ARE	   BYTES");
				consoleText(8+(9<<3),144-64-8,itoa_cool(sizeof(object)));
				consoleText(8,152-64-8,"MEM:	     /");
				consoleText(8+(5<<3),152-64-8,itoa_cool(objectcount()*sizeof(object)));
				consoleText(8+(12<<3),152-64-8,itoa_cool(MAX_OBJECTS*sizeof(object)));
				consoleText(8,160-64-8,"TEXSZ:");
				consoleText(8+(7<<3),160-64-8,itoa_cool(texsize));
				consoleText(8,168-64-8,"GL MEM:");
				consoleText(8,176-64-8,"            /");
				glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, 
					&GL_mem_total);
				glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, 
					&GL_mem_free);
				consoleText(8,176-64-8,itoa_cool(GL_mem_total-GL_mem_free));
				consoleText(8+(14<<3),176-64-8,itoa_cool(GL_mem_total));
				consoleText(8,184-64-8,"ERROR:");
				consoleText(8+(7<<3),184-64-8,itoa_cool(glGetError()));
				consoleText(8,192-64-8,"LAG us:");
				consoleText(8,192-64," slept:");
				consoleText(8+(8<<3),192-64-8,itoa_cool((int)(LAG_3-LAG_2)));
				consoleText(8+(8<<3),192-64,itoa_cool((int)(LAG_1-LAG_0)));
				consoleText(8,192-64+8,"total =         / 166666");
				consoleText(8+(8<<3),192-64+8,itoa_cool((int)(LAG_1-LAG_0+LAG_3-LAG_2)));
				consoleText(8,192-64+16,"off by");
				consoleText(8+(8<<3),192-64+16,itoa_cool((int)(166666-(LAG_1-LAG_0+LAG_3-LAG_2))));
			}
			#endif
			#endif
			
			//glFlush(); do i need this?
			SwapBuffers(hDC);
		}
		
		QueryPerformanceCounter((LARGE_INTEGER*)&LAG_0);
		if(!key(VK_TAB)) // <-- FIXES HIGHER STUPID CPU USAGE
						// (FROM 1% INCREMENTING TO 3% WITHOUT UNLIMIT AND THEN TO 0.8%)
						// SO STUPID
			// HOW ABOUT USE ONE IMAGE, DUMMY
			// APPARENTLY THAT'S NOT WHY IT'S HAPPENINGuy
			// I DON'T KNOW!!
		{
			TIMER = CreateWaitableTimer(NULL, TRUE, NULL);
			QueryPerformanceCounter((LARGE_INTEGER*)&frameTime);
			frameTime += frameBase-LAG_1;
			// lags hard on ReactOS
			// even without the timer
			// TODO: implement using either QPC, time periods, or TickCount
			SetWaitableTimer(TIMER, (LARGE_INTEGER*)&frameTime, 0, NULL, NULL, 0);
			WaitForSingleObject(TIMER, INFINITE);
			CloseHandle(TIMER);
		}
		QueryPerformanceCounter((LARGE_INTEGER*)&LAG_1);
		if(key(VK_ESCAPE))
			active = 0;
		
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;
	}
	
	free(objects);

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);

	//return msg.wParam;
	return 0;
}

_stdcall LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
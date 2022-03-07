
#include <stdio.h>

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




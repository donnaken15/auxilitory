
#include "map.h"

uint_fast16_t	const
	__width   =  224,
	__height  =  288,
	__scale   = 2,
	levelcount = 4;
uint_fast32_t	wy = 0;
	time = 0, score = 0, hiscore = 0,
	tmpax, tmpbx, tmpcx, tmpdx, tmpex;
float tmpfx, ws = 2;
uint_fast8_t active = 3,
	levels[] = { // x >> 4
		32,
		50,
		8,
		43, // boss
		18,
		37,
		0,
		48,
		29,
		3,
		43,
		22,
		50,
		0,
		48,
		29,
		1,
		43,
		50,
		4,
		57,
		48,
		36,
		8,
		37,
		50,
		0,
		48,
		29,
		4,
		43,
		48,
		22,
		6,
		37,
		50
	};

#define TEX_COUNT		4
#define TEX_TEXT		0
#define TEX_OBJECT		1
#define TEX_PARTICLES	2
#define TEX_TERRAIN		3
GLuint Textures[TEX_COUNT];
TEX tex[TEX_COUNT];
char*texFn[] = {
	"gfx\\text.png",
	"gfx\\object.png",
	"gfx\\particles.png",
	"gfx\\terrain.png"
};

int _player; // unassigned var, lol no Fs given
//{
#pragma region OBJECT CODE
#define player objects[_player]
#define this objects[id]
// weird
void nop()
{
}
void player_create(int id)
{
	
}
void player_step(id)
{
	this.x -= key(VK_LEFT)  &&this.x>0;
	this.x += key(VK_RIGHT) &&this.x<224-16;
	this.y -= key(VK_UP)	&&this.y>16+96;
	this.y += key(VK_DOWN)  &&this.y<288-16;
	glPushMatrix();
		glTranslatef(this.x, this.y, 0);
		drawImage(Textures[TEX_OBJECT],
			0, 0,
			16, 16,
			0, 0,
			16, 16);
		glTranslatef(8, -88, 0);
		for (int i = 0; i < 4; i++)
		{
			drawImage(Textures[TEX_OBJECT],
				120, 0,
				8, 8,
				0, 0,
				8, 8);
			glRotatef(90,0,0,1);
		}
	glPopMatrix();
}
void toroid_create(int id)
{
	if(this.x > 112)
		this.props[2] = 1;
	else
		this.props[2] = 0;
}
void toroid_step(int id)
{
	this.y += 2;
	if (this.props[0])
	{
		if(this.props[2] == 0)
			this.x -= (this.time/6);
		else
			this.x += (this.time/6);
		if(this.props[1]<32)
			this.props[1]++;
	}
	else // why is bitwise not not working
	{
		if(this.props[2] == 0)
		{
			if(this.x>player.x-8) {
				this.props[0] = 1;
				this.time = 0;
			}
			this.x += 0.3f;
		}
		else
		{
			if(this.x<player.x+8) {
				this.props[0] = 1;
				this.time = 0;
			}
			this.x -= 0.3f;
		}
	}
	drawImage(Textures[TEX_OBJECT],
		((this.props[1]>>2)&7)*12, 16,
		12, 12,
		floor(this.x), floor(this.y),
		12, 12);
	if(this.y>288||this.x<-32||this.x>224+32)
		kill(id);
}
#pragma endregion
//}

typedef struct {
	uint_fast32_t time;
	uint_fast16_t cmd, a, b, c, d;
} worldcontrol;
worldcontrol WC[] = { // lol name
	//	TIME	CMD		A		B		C		D
	//					(usually x,y)
	{	460,	1,		0,		0,		0,		0},
	{	680,	1,		0,		0,		0,		0},
	{	1000,	1,		0,		0,		0,		0},
	{	99999,	0xFF,	0,		0,		0,		0},
};
int WCi = 0;
char WCstop = 0;

void world_spawntoroids()
{
	int offset = 0;
	for (int i = 0; i < 2 + rand() / (RAND_MAX/4); i++)
	{
		if (rand() > RAND_MAX/2)
			offset = 120;
		newobj(offset+16+(rand()/(RAND_MAX/64)),-16,toroid_create,toroid_step,nop);
		offset = 0;
	}
}
void world_create(int id)
{
	
}
void world_step(int id)
{
	if (wy == WC[WCi].time && !WCstop)
	{
		switch (WC[WCi].cmd)
		{
			case 1:
				world_spawntoroids();
				break;
			case 0xFF:
				WCstop = 1;
				break;
		}
		WCi++;
	}
	//if (wy % 3 == 0)
		//world_spawntoroids();
}

//#define DBG
float rate = 60.0f;

GAMEFUNC void init()
{
	srand(GetTickCount());
	newobj((224>>1) - 8, 288-40, player_create, player_step, nop);
	newobj(0, 0, world_create, world_step, nop);
}
GAMEFUNC void loop()
{
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glPushMatrix();
			glTranslatef(0, 268, 0);
			for (int i = 0; i < 2; i++)
			{
				glTranslatef(0, -224, 0);
				if (wy < 940)
				drawImage(Textures[TEX_TERRAIN],
					256, 0,
					224, 224,
					0, (wy/ws),
					224, 244);
			}
			for (int i = 0; i < levelcount; i++)
			{
				register unsigned int tile = 0;
				int offx, offy;
				offx = levels[i] << 1;
				offy = 0;
				glTranslatef(0, -2048, 0);
				// draw one image map
				/*drawImage(Textures[TEX_TERRAIN],
					levels[i] << 4, 0,
					tex[TEX_TERRAIN].w, tex[TEX_TERRAIN].h,
					0, ftoui(wy/ws),
					tex[TEX_TERRAIN].w, tex[TEX_TERRAIN].h);*/
				glPushMatrix();
					// tile drawing
					for (int k = 0; k < 2048; k++)
					{
						for (int j = 0; j < 28; j++)
						{
							if (wy + (k << 4) < (4096*(i+1))+512-128-45 ||
								wy + (k << 4) > (4096*(i+1))+512+7+(52<<3))
								break;
							tile = (offx+j)+((k+offy)*mapw);
							if (tile >= mapw * maph ||
								j >= mapw || k >= maph)
								goto outofboundsTiles;
							drawImage(Textures[TEX_TERRAIN],
								(map[tile]&31)<<3, (map[tile]>>5)<<3, 8, 8,
								j<<3, (wy/ws)+(k<<3),
								8, 8);
						}
					}
					outofboundsTiles:
				glPopMatrix();
			}
		glPopMatrix();
	}
	
	{
		for (int i = 0, j = MAX_OBJECTS; i < j; i++)
		{
			if(objects[i].active)
			{
				(*objects[i].step)(i);
				objects[i].time++;
			}
		}
		consoleText(32,0,"1UP");
		consoleText(32,8,itoa_cool(score));
		consoleText(96,0,"HIGH SCORE");
		consoleText(96,8,itoa_cool(hiscore));
		
		wy++;
		time++; // ditching Clock, lol
	}

}

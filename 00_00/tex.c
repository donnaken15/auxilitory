
#include <stdio.h>

char*texFn[] = {
	"gfx\\text.png",
	"gfx\\object.png",
	"gfx\\particles.png",
	"gfx\\terrain.png"
};
enum TEXIDS {
	TEX_TEXT,
	TEX_OBJECT,
	TEX_PARTICLES,
	TEX_TERRAIN,
	
	TEX_COUNT
};

typedef struct { // always RGBA
	uint_fast32_t w,h;
	unsigned char*data;
} TEX;
TEX  loadTEX(LPSTR fname)
{
	int x,y,n;
	unsigned char *data = stbi_load(fname, &x, &y, &n, 4);
	TEX newtex;
	newtex.w = x;
	newtex.h = y;
	newtex.data = data;
	return newtex;
}

GLuint Textures[TEX_COUNT];
TEX tex[TEX_COUNT];
COORD GLtxSz[TEX_COUNT+1];
uint_fast32_t texsize = 0;

void drawImage(
	GLuint texn,
	USHORT l, USHORT t,
	USHORT r, USHORT b,
	short x, short y,
	short w, short h)
{
	//uint_fast16_t tw, th;
	COORD*txs = &GLtxSz[texn]; // is this faster to use
	
	r += !r;
	b += !b;
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, Textures[texn]);
		
		//tw = GLtxSz[texn].X;
		//th = GLtxSz[texn].Y;
		glTranslatef(x, y, 0);
		
		float texcoords[] = {
			(float)(l) / txs->X,
			(float)(t) / txs->Y,
			((float)(l)+(float)(r)) / txs->X,
			((float)(t)+(float)(b)) / txs->Y,
		};
		#define drawImageUseQuads 0
		#if (drawImageUseQuads == 1)
		glBegin(GL_QUADS);
			glTexCoord2f(texcoords[0], texcoords[2]);
			glVertex2i(0, h);
			glTexCoord2f(texcoords[0], texcoords[1]);
			glVertex2i(0, 0);
			glTexCoord2f(texcoords[2], texcoords[1]);
			glVertex2i(w, 0);
			glTexCoord2f(texcoords[2], texcoords[3]);
			glVertex2i(w, h);
		glEnd();
		#else
		glBegin(GL_TRIANGLES);
			glTexCoord2f(texcoords[0], texcoords[1]);
			glVertex2i(0, 0);
			glTexCoord2f(texcoords[2], texcoords[1]);
			glVertex2i(w, 0);
			glTexCoord2f(texcoords[0], texcoords[3]);
			glVertex2i(0, h);
			
			glTexCoord2f(texcoords[2], texcoords[1]);
			glVertex2i(w, 0);
			glTexCoord2f(texcoords[0], texcoords[3]);
			glVertex2i(0, h);
			glTexCoord2f(texcoords[2], texcoords[3]);
			glVertex2i(w, h);
		glEnd();
		#endif
		
		glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
}

//#if (TEX_TEXT)
_stdcall void consoleText(short X, short Y, LPCSTR TEXT)
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
			cT = (uint)(floor((floor)(*CURCHAR) / 16.0f)) << 3;
			glColor4f(0,0,0,lastColors[3]);
			drawImage(TEX_TEXT, cL, cT, 8, 8, XX+1, YY+1, 8, 8);
			glColor4f(lastColors[0],lastColors[1],lastColors[2],lastColors[3]);
			drawImage(TEX_TEXT, cL, cT, 8, 8, XX, YY, 8, 8);
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
//#endif


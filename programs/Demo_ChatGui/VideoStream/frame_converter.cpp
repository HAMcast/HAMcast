#include <QImage>
#include <string.h>
#include "frame_converter.h"
//#include <windows.h>
//#include "../wintypes.h"
#ifdef WINDOWS
#include <windows.h>
#endif

#include "qimagescale.h"
#define APPMAX 2560*1920
//#include "../globals.h"

#include "ColorFormat.h"

frame_converter::frame_converter(int x, int y)
{
	aimX = x;
	aimY = y;
	puffer = new unsigned char[x*y*4];
	memset(puffer,128,x*y*3/2); //eigentlich egal
}

frame_converter::~frame_converter()
{
}

//*************************************************************************
//verarbeitet alle Groessen von 128x96 bis 768x576
//grob Rechenzeitoptimiert
//man koennte dies noch in 3 Methoden gliedern zur Uebersichtlichkeit
//*************************************************************************

bool frame_converter::convertFrame(int w, int h, unsigned char *input, unsigned char *output)
{
	//Variablendeklarationen vielleicht in die Headerdatei;
	int a,d,e;				//schleifen Variablen
	int memA,memB,memC,memD;//Variablen fuer Zwischenergebnisse zur Rechenoptimierung
	float b,c;				//Zoomfaktor
	unsigned char *OUTy;	//Zeiger auf y, u, v Zielspeicher
	unsigned char *INy;	//Zeiger auf y, u, v Quellspeicher
	unsigned char *OUTu;	//Zeiger auf y, u, v Zielspeicher
	unsigned char *INu;	//Zeiger auf y, u, v Quellspeicher
	unsigned char *OUTv;	//Zeiger auf y, u, v Zielspeicher
	unsigned char *INv;	//Zeiger auf y, u, v Quellspeicher

	if ((w<128)||(h<96)||(w*h>APPMAX)) //grauen Frame zurueckliefern bei falschen Werten
	{
		memset(output,128,aimX*aimY*3/2);
		return false;
	}
	b = (float)((float)aimY/(float)h);
	c = (float)((float)aimX/(float)w);
	if ((b<1.0f)&&(c>1.0f)) b=c;
	else if ((c<1.0f)&&(b>1.0f)) c=b;
	else if (c<b) b=c;

	if ((aimX!=w)&&(aimY!=h)&&(aimX==((aimY*5/3)&0xfff0))&&(b<1))
	{
		if (((float)w/(float)h)<=((float)aimX/(float)aimY))
		{
			int hoehe = (aimX*h/w)&0xfffC;

			qSmoothScaleImage(input, w, h, puffer, aimX, hoehe);
			qSmoothScaleImage(input+w*h, w/2, h/2, puffer+aimX*hoehe, aimX/2, hoehe/2);
			qSmoothScaleImage(input+w*h*5/4, w/2, h/2, puffer+aimX*hoehe*5/4, aimX/2, hoehe/2);

			w = aimX;
			h = hoehe;

			//setze obere linke Ecke in den Y Frame
			memcpy(output, puffer + ((h-aimY)/2*w),aimX*aimY);
			//setze obere linke Ecke in den U Frame
			memcpy(output + (aimX*aimY), puffer + w*h + ((h/2-aimY/2)/2*(w/2)), aimX*aimY/4);
			//setze obere linke Ecke in den V Frame
			memcpy(output + (aimX*aimY*5/4), puffer + w*h*5/4 + ((h/2-aimY/2)/2*(w/2)), aimX*aimY/4);
		}
		else
		{
			int breite = (aimY*w/h)&0xfffe;

			qSmoothScaleImage(input, w, h, puffer, breite, aimY);
			qSmoothScaleImage(input+w*h, w/2, h/2, puffer+breite*aimY, breite/2, aimY/2);
			qSmoothScaleImage(input+w*h*5/4, w/2, h/2, puffer+breite*aimY*5/4, breite/2, aimY/2);

			w = breite;
			h = aimY;
			
			for (int i=0;i<h/2;i++)
			{
				//setze obere linke Ecke in den Y Frame
				memcpy(output+ i*2   *aimX, puffer +  i*2   *w + (w-aimX)/2,aimX);
				memcpy(output+(i*2+1)*aimX, puffer + (i*2+1)*w + (w-aimX)/2,aimX);
				//setze obere linke Ecke in den U Frame
				memcpy(output+(aimX*aimY) + i*aimX/2, puffer+w*h + i*w/2 + (w/2-aimX/2)/2,aimX/2);
				//setze obere linke Ecke in den V Frame
				memcpy(output+(aimX*aimY*5/4) + i*aimX/2, puffer+w*h*5/4 + i*w/2 + (w/2-aimX/2)/2,aimX/2);
			}
		}
		return true;
	}
/*	if ((aimX==((gHeight*5/3)&0xfff0))&&(w!=gWidth)&&(aimY==gHeight)&&(h!=gHeight)&&(b<1))
	{
		int breite = (aimY*w/h)&0xfffe;

		qSmoothScaleImage(input, w, h, puffer, breite, aimY);
		qSmoothScaleImage(input+w*h, w/2, h/2, puffer+breite*aimY, breite/2, aimY/2);
		qSmoothScaleImage(input+w*h*5/4, w/2, h/2, puffer+breite*aimY*5/4, breite/2, aimY/2);

		w = breite;
		h = aimY;
		
		for (int i=0;i<h/2;i++)
		{
			//setze obere linke Ecke in den Y Frame
			memcpy(output+ i*2   *aimX, puffer +  i*2   *w + (w-aimX)/2,aimX);
			memcpy(output+(i*2+1)*aimX, puffer + (i*2+1)*w + (w-aimX)/2,aimX);
			//setze obere linke Ecke in den U Frame
			memcpy(output+(aimX*aimY) + i*aimX/2, puffer+w*h + i*w/2 + (w/2-aimX/2)/2,aimX/2);
			//setze obere linke Ecke in den V Frame
			memcpy(output+(aimX*aimY*5/4) + i*aimX/2, puffer+w*h*5/4 + i*w/2 + (w/2-aimX/2)/2,aimX/2);
		}
		return true;
	}
*/	if ((aimX==w)&&(aimY==((aimX*3/5)&0xfff0))&&(b<1))
	{
		//setze obere linke Ecke in den Y Frame
		memcpy(output, input + ((h-aimY)/2*w),aimX*aimY);
		//setze obere linke Ecke in den U Frame
		memcpy(output + (aimX*aimY), input + w*h + ((h/2-aimY/2)/2*(w/2)), aimX*aimY/4);
		//setze obere linke Ecke in den V Frame
		memcpy(output + (aimX*aimY*5/4), input + w*h*5/4 + ((h/2-aimY/2)/2*(w/2)), aimX*aimY/4);
		return true;
	}
	if ((aimX==((aimY*5/3)&0xfff0))&&(aimY==h)&&(b<1))
	{
		for (int i=0;i<h/2;i++)
		{
			//setze obere linke Ecke in den Y Frame
			memcpy(output+ i*2   *aimX, input +  i*2   *w + (w-aimX)/2,aimX);
			memcpy(output+(i*2+1)*aimX, input + (i*2+1)*w + (w-aimX)/2,aimX);
			//setze obere linke Ecke in den U Frame
			memcpy(output+(aimX*aimY) + i*aimX/2, input+w*h + i*w/2 + (w/2-aimX/2)/2,aimX/2);
			//setze obere linke Ecke in den V Frame
			memcpy(output+(aimX*aimY*5/4) + i*aimX/2, input+w*h*5/4 + i*w/2 + (w/2-aimX/2)/2,aimX/2);
		}
		return true;
	}
	if (b<1)	//****** verkleinern **************************************
	{	
		qSmoothScaleImage(input, w, h, output, aimX, aimY);
		qSmoothScaleImage(input+w*h, w/2, h/2, output+aimX*aimY, aimX/2, aimY/2);
		qSmoothScaleImage(input+w*h*5/4, w/2, h/2, output+aimX*aimY*5/4, aimX/2, aimY/2);

		return true;
	}

	if (b<.25)	b=.125f;		//waehle skalierung  -  von 128x76 bis 1600x1200 lauffaehig!
	else
		if(b<.5) b=.25;
		else
			if(b<1) b=.5;
			else
				if(b<2)	b=1;
				else
					if(b<3) b=2;
					else
						if(b<4) b=3;
						else
							if(b<5) b=4;
							else
								if(b<6) b=5;
								else b=6;

	//*************************************************************************************
	if (b!=1)	//***** skaliere kleiner oder groesser ************************************
	{
		//puffer	=	savePointer;
		//memset(puffer,128,aimX*aimY*3/2);
		
		OUTy	=	puffer;
		INy	=	input;
		if (b<1)	//****** verkleinern **************************************
		{	
			//**** Y ****
			memA=(int)(w*b);
			memB=(int)(1/b);
			memC=(int)(w/b-w);
			memD=(int)(h*b);
			for(d=0;d<memD;d++)	//zeilenweise
			{	
				for (a=0;a<memA;a++)	//spaltenweise
				{
					*OUTy = *INy;
					INy+=memB;
					OUTy++;
				}
				INy+=memC;	//ueberspringe Zeilen
			}

			//**** U,V ****
			OUTu =	puffer + (long)(w*h*b*b);
			INu = input  + (long)(w*h);
			OUTv = puffer + (long)(w*h*b*b*5/4);
			INv = input  + (long)(w*h*5/4);
			memA=(int)(w*b/2);
			memC=(int)(w/b/2-w/2);
			memD=(int)(h*b/2);
			for(d=0;d<memD;d++)	//zeilenweise
			{	
				for (a=0;a<memA;a++)	//spaltenweise
				{
					*OUTu = *INu;
					INu+=memB;
					OUTu++;
					*OUTv = *INv;
					INv+=memB;
					OUTv++;
				}
				INu+=memC;	//ueberspringe Zeilen
				INv+=memC;	//ueberspringe Zeilen
			}
		}
		else		//**** vergroessern *****************************************
		{
			//**** Y ****
			memA=(int)(w*b);
			memB=(int)(b-1);
			for(d=0;d<h;d++)	//zeilenweise
			{	
				for (a=0;a<w;a++)	//spaltenweise
				{
					for(e=0;e<b;e++)
					{
						*OUTy = *INy;
						OUTy++;
					}
					INy++;
				}
				for(e=0;e<memB;e++)
				{
					memcpy(OUTy,OUTy-memA,memA);
					OUTy+=memA;
				}
			}
			//**** U,V ****
			OUTu=	puffer + (long)(w*h*b*b);
			INu = input  + (long)(w*h);
			OUTv= puffer + (long)(w*h*b*b*5/4);
			INv = input  + (long)(w*h*5/4);
			memA=w/2;
			memB=(int)(b-1);
			memC=(int)(w*b/2);
			memD=h/2;
			for(d=0;d<memD;d++)	//zeilenweise
			{	
				for (a=0;a<memA;a++)	//spaltenweise
				{
					for(e=0;e<b;e++)
					{
						*OUTu = *INu;
						OUTu++;
						*OUTv = *INv;
						OUTv++;
					}
					INu++;
					INv++;
				}
				for(e=0;e<memB;e++)
				{
					memcpy(OUTu,OUTu-memC,memC);
					OUTu+=memC;
					memcpy(OUTv,OUTv-memC,memC);
					OUTv+=memC;
				}
			}
		}
		//memcpy(input,puffer,w*h*3/2);
		//setze die Ergebnisse der Skalierung
		input = puffer;
		w		=	(int)(w*b);
		h		=	(int)(h*b);
	}

	if ((w!=aimX)||(h!=aimY))
	{
		//***********************************************************************************
		//setze skaliertes Video in die Mitte des Frames
		memset(output,222,aimX*aimY);
		memset(output+aimX*aimY,126,aimX*aimY*1/4);
		memset(output+aimX*aimY*5/4,129,aimX*aimY*1/4);
		//setze obere linke Ecke in den Y Frame
		OUTy = output + ((aimY-h)/2*aimX+((aimX-w)/2));
		if (OUTy<output) OUTy=output;
		INy  = input;
		//setze obere linke Ecke in den U Frame
		OUTu = output + (aimX*aimY)+((aimY-h)/8*aimX+((aimX-w)/4));
		if (OUTu<output + (aimX*aimY)) OUTu=output + (aimX*aimY);
		INu  = input + w*h;
		//setze obere linke Ecke in den V Frame
		OUTv = output + (aimX*aimY*5/4)+((aimY-h)/8*aimX+((aimX-w)/4));
		if (OUTv<output + (aimX*aimY*5/4)) OUTv=output + (aimX*aimY*5/4);
		INv  = input + w*h*5/4;

		memA=w/2;
		memB=aimX/2;
		memC=h/2;
		memD=aimY/2;
		if (memC>memD) memC=memD;
		for(a=0;a<memC;a++)
		{
			memcpy(OUTy,INy,w);
			OUTy += aimX;
			INy  += w;
			memcpy(OUTy,INy,w);
			OUTy += aimX;
			INy  += w;
			memcpy(OUTu,INu,memA);
			OUTu += memB;
			INu  += memA;
			memcpy(OUTv,INv,memA);
			OUTv += memB;
			INv  += memA;
		}
	}
	else
	{
		memcpy(output,input,w*h*3/2);
	}
	return true;
}

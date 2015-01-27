
#include <memory.h>
#include "ColorFormat.h"

extern "C"
{
//#include "mjpeg/utils.h"
}

/******************************************
 *
 * Tabellen f�r YUVtoRBG, RGB565 und RGB555
 *
 ******************************************/
static int yb[256],yg[256],yr[256],ug[256],ur[256],vb[256],vg[256];
static int rv[256],gu[256],gv[256],bu[256];
static unsigned char clip[1024]; 
static unsigned short rclip565[1024]; 
static unsigned short gclip565[1024]; 
static unsigned short bclip565[1024]; 
static unsigned short rclip555[1024]; 
static unsigned short gclip555[1024]; 
static unsigned short bclip555[1024]; 


/**************************************************************
 *
 * Initialisierung der Tabellen f�r YUVtoRBG, RGB565 und RGB555
 *
 **************************************************************/
void initColorLUT()
{
int i,n;
unsigned char *iclip;
unsigned short *irclip,*igclip,*ibclip;

	for (n= 0; n < 256; n++) 
	{
		yb[n]=(int)(n*0.114);
		yg[n]=(int)(n*0.587);
		yr[n]=(int)(n*0.299);
		ug[n]=(int)(n*0.3316);
		ur[n]=(int)(n*0.169);
		vb[n]=(int)(n*0.0813);
		vg[n]=(int)(n*0.4186);
	}

	for (n= 0; n < 256; n++) 
	{
		rv[n]=(int)((n-128)*1.40208770291);
		gu[n]=(int)((n-128)*0.344102331921);
		gv[n]=(int)((n-128)*0.714225834977);
		bu[n]=(int)((n-128)*1.77179519564);

	}

	iclip = clip+512;

	for (i=-512; i<512; i++)
      iclip[i] = (i < 0 ) ? 0 : ((i>255) ? 255 : i);

	irclip = rclip555+512;
	igclip = gclip555+512;
	ibclip = bclip555+512;

	for (i=-512; i<512; i++)
	{
 	  irclip[i] = (i < 0 ) ? 0 : ((i>255) ? (255>>3)<<10: (i>>3)<<10);
	  igclip[i] = (i < 0 ) ? 0 : ((i>255) ? (255>>3)<<5 : (i>>3)<<5);
	  ibclip[i] = (i < 0 ) ? 0 : ((i>255) ? 255>>3 : i>>3);
	}

	irclip = rclip565+512;
	igclip = gclip565+512;
	ibclip = bclip565+512;

	for (i=-512; i<512; i++)
	{
	  irclip[i] = (i < 0 ) ? 0 : ((i>255) ? (255>>3)<<11: (i>>3)<<11);
	  igclip[i] = (i < 0 ) ? 0 : ((i>255) ? (255>>2)<<5 : (i>>2)<<5);
	  ibclip[i] = (i < 0 ) ? 0 : ((i>255) ? 255>>3 : i>>3);
	}
}

void colorConvYUV420toYUV420(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  memcpy(out,in,inWidth*inHeight*3/2);
}

void colorConvYUYVtoYUV420(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char *yuyvp;

	yuyvp= in;
	
	for(i=0; i<inWidth*inHeight; i++)
	{
		*out++ = *yuyvp;
		yuyvp+=2;
	}

	yuyvp= in+1;

	for (i=0; i<inHeight/2; i++) 
	{
	  for (j=0; j<inWidth/2; j++)  
	  {
		 *out++ = *yuyvp;
		 yuyvp+=4;
	  }
	  yuyvp+=inWidth*2;
	}

	yuyvp= in+3;

	for (i=0; i<inHeight/2; i++) 
	{
	  for (j=0; j<inWidth/2; j++)  
	  {
		 *out++ = *yuyvp;
		 yuyvp+=4;
	  }
	  yuyvp+=inWidth*2;
	}
}

void colorConvRGB32toYUV420(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*yp2,*up,*vp,*rgbp,*rgbp2;

  rgbp =in+inWidth*4*(inHeight-1);
  rgbp2=in+inWidth*4*(inHeight-2);

  yp =out;
  yp2=out+inWidth;
  
  up =yp+inWidth*inHeight;
  vp =up+inWidth*inHeight/4;

  iclip = clip+512;

  for (i=0; i<inHeight/2; i++) 
  {
    for (j=0; j<inWidth/2; j++)  
	{
		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;
		rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];//>>2];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];//>>2];
		yp++;

		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;
		rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp++;

		b  = *rgbp2++;
		g  = *rgbp2++;
		r  = *rgbp2++;
		rgbp2++;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

		b  = *rgbp2++;
		g  = *rgbp2++;
		r  = *rgbp2++;
		rgbp2++;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

	    up++;
	    vp++;
	}
    yp+=inWidth;
    yp2+=inWidth;
    rgbp-=inWidth*4*3;
    rgbp2-=inWidth*4*3;
  }
}

void colorConvRGB24toYUV420(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*yp2,*up,*vp,*rgbp,*rgbp2;

  rgbp =in+inWidth*3*(inHeight-1);
  rgbp2=in+inWidth*3*(inHeight-2);

  yp =out;
  yp2=out+inWidth;
  
  up =yp+inWidth*inHeight;
  vp =up+inWidth*inHeight/4;

  iclip = clip+512;

  for (i=0; i<inHeight/2; i++) 
  {
    for (j=0; j<inWidth/2; j++)  
	{
		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];//>>2];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];//>>2];
		yp++;

		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp++;

		b  = *rgbp2++;
		g  = *rgbp2++;
		r  = *rgbp2++;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

		b  = *rgbp2++;
		g  = *rgbp2++;
		r  = *rgbp2++;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

	    up++;
	    vp++;
	}
    yp+=inWidth;
    yp2+=inWidth;
    rgbp-=inWidth*3*3;
    rgbp2-=inWidth*3*3;
  }
}

void colorConvRGB565toYUV420(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*yp2,*up,*vp,*rgbp,*rgbp2;

  rgbp =in+inWidth*2*(inHeight-1);
  rgbp2=in+inWidth*2*(inHeight-2);

  yp =out;
  yp2=out+inWidth;
  
  up =yp+inWidth*inHeight;
  vp =up+inWidth*inHeight/4;

  iclip = clip+512;

  for (i=0; i<inHeight/2; i++) 
  {
    for (j=0; j<inWidth/2; j++)  
	{
		b  = *rgbp<<3;
		g  = (*(rgbp+1)<<5)+((*rgbp&0xe0)>>3);
		r  = (*(rgbp+1)&0xf8);
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];//>>2];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];//>>2];
		yp++;

		b  = *rgbp<<3;
		g  = (*(rgbp+1)<<5)+((*rgbp&0xe0)>>3);
		r  = (*(rgbp+1)&0xf8);
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp++;

		b  = *rgbp2<<3;
		g  = (*(rgbp2+1)<<5)+((*rgbp2&0xe0)>>3);
		r  = (*(rgbp2+1)&0xf8);
		rgbp2+=2;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

		b  = *rgbp2<<3;
		g  = (*(rgbp2+1)<<5)+((*rgbp2&0xe0)>>3);
		r  = (*(rgbp2+1)&0xf8);
		rgbp2+=2;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

	    up++;
	    vp++;
	}
    yp+=inWidth;
    yp2+=inWidth;
    rgbp-=inWidth*2*3;
    rgbp2-=inWidth*2*3;
  }
}

void colorConvRGB555toYUV420(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*yp2,*up,*vp,*rgbp,*rgbp2;

  rgbp =in+inWidth*2*(inHeight-1);
  rgbp2=in+inWidth*2*(inHeight-2);

  yp =out;
  yp2=out+inWidth;
  
  up =yp+inWidth*inHeight;
  vp =up+inWidth*inHeight/4;

  iclip = clip+512;

  for (i=0; i<inHeight/2; i++) 
  {
    for (j=0; j<inWidth/2; j++)  
	{
		b  = *rgbp<<3;
		g  = (*(rgbp+1)<<6)+((*rgbp&0xe0)>>2);
		r  = (*(rgbp+1)&0x7c)<<1;
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];//>>2];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];//>>2];
		yp++;

		b  = *rgbp<<3;
		g  = (*(rgbp+1)<<6)+((*rgbp&0xe0)>>2);
		r  = (*(rgbp+1)&0x7c)<<1;
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp++;

		b  = *rgbp2<<3;
		g  = (*(rgbp2+1)<<6)+((*rgbp2&0xe0)>>2);
		r  = (*(rgbp2+1)&0x7c)<<1;
		rgbp2+=2;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

		b  = *rgbp2<<3;
		g  = (*(rgbp2+1)<<6)+((*rgbp2&0xe0)>>2);
		r  = (*(rgbp2+1)&0x7c)<<1;
		rgbp2+=2;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

	    up++;
	    vp++;
	}
    yp+=inWidth;
    yp2+=inWidth;
    rgbp-=inWidth*2*3;
    rgbp2-=inWidth*2*3;
  }
}


void colorConvUYVYtoYUV420(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char *yuyvp;

	yuyvp= in+1;
	
	for(i=0; i<inWidth*inHeight; i++)
	{
		*out++ = *yuyvp;
		yuyvp+=2;
	}

	yuyvp= in;

	for (i=0; i<inHeight/2; i++) 
	{
	  for (j=0; j<inWidth/2; j++)  
	  {
		 *out++ = *yuyvp;
		 yuyvp+=4;
	  }
	  yuyvp+=inWidth*2;
	}

	yuyvp= in+2;

	for (i=0; i<inHeight/2; i++) 
	{
	  for (j=0; j<inWidth/2; j++)  
	  {
		 *out++ = *yuyvp;
		 yuyvp+=4;
	  }
	  yuyvp+=inWidth*2;
	}
}


void colorConvYUV420toYUYV(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char *yp,*yp2,*up,*vp,*yuyvp,*yuyvp2;

  yuyvp =out;
  yuyvp2=out+outWidth;

  yp =in;
  yp2=in+inWidth;
  
  up =yp+inWidth*inHeight;
  vp =up+inWidth*inHeight/4;

	for (i=0; i<inHeight/2; i++) 
	{
	  for (j=0; j<inWidth/2; j++)  
	  {
		yuyvp[0] = yp[0];				
		yuyvp[1] = up[0];	
		yuyvp[2] = yp[1];			
		yuyvp[3] = vp[0];	

		yuyvp2[0] = yp2[0];		
		yuyvp2[1] =	up[0];	
		yuyvp2[2] = yp2[1];			
		yuyvp2[3] =	vp[0];	

		yp+=2;
		yp2+=2;
		up++;
		vp++;
		yuyvp+=4;
		yuyvp2+=4;
	  }

	  yp+=inWidth;
	  yuyvp+=outWidth*2-inWidth*2;
	  yp2+=inWidth;
	  yuyvp2+=outWidth*2-inWidth*2;
	}
}

void colorConvYUYVtoYUYV(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i;

  inWidth*=2;

  for (i=0; i<inHeight; i++)
  {
    memcpy(out,in,inWidth);
    out+=outWidth;
    in+=inWidth;
  }
}

void colorConvRGB32toYUYV(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp,*rgbp;

  rgbp =in+inWidth*4*(inHeight-1);

  yp = out;
  up = out+1;
  vp = out+3;

  iclip = clip+512;

  for (i=0; i<inHeight; i++) 
  {
    for (j=0; j<inWidth/2; j++)  
	{
		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;
		rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];//>>1];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];//>>1];
		yp+=2;

		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;
		rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>1) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>1) + *vp];

		yp+=2;
	    up+=4;
	    vp+=4;
	}
	yp+=outWidth-inWidth*2;
	up+=outWidth-inWidth*2;
	vp+=outWidth-inWidth*2;
    rgbp-=inWidth*4*2;
  }
}

void colorConvRGB24toYUYV(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp,*rgbp;

  rgbp =in+inWidth*3*(inHeight-1);

  yp = out;
  up = out+1;
  vp = out+3;

  iclip = clip+512;

  for (i=0; i<inHeight; i++) 
  {
    for (j=0; j<inWidth/2; j++)  
	{
		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];//>>1];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];//>>1];
		yp+=2;

		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>1) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>1) + *vp];

		yp+=2;
	    up+=4;
	    vp+=4;
	}
	yp+=outWidth-inWidth*2;
	up+=outWidth-inWidth*2;
	vp+=outWidth-inWidth*2;
    rgbp-=inWidth*3*2;
  }
}

void colorConvRGB565toYUYV(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp,*rgbp;

  rgbp =in+inWidth*2*(inHeight-1);

  yp = out;
  up = out+1;
  vp = out+3;

  iclip = clip+512;

  for (i=0; i<inHeight; i++) 
  {
    for (j=0; j<inWidth/2; j++)  
	{
 	    b  = *rgbp<<3;
	    g  = (*(rgbp+1)<<5)+((*rgbp&0xe0)>>3);
	    r  =(*(rgbp+1)&0xf8);
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];//>>1];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];//>>1];
		yp+=2;

 	    b  = *rgbp<<3;
	    g  = (*(rgbp+1)<<5)+((*rgbp&0xe0)>>3);
	    r  =(*(rgbp+1)&0xf8);
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>1) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>1) + *vp];

		yp+=2;
	    up+=4;
	    vp+=4;
	}
	yp+=outWidth-inWidth*2;
	up+=outWidth-inWidth*2;
	vp+=outWidth-inWidth*2;
    rgbp-=inWidth*2*2;
  }
}

void colorConvRGB555toYUYV(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp,*rgbp;

  rgbp =in+inWidth*2*(inHeight-1);

  yp = out;
  up = out+1;
  vp = out+3;

  iclip = clip+512;

  for (i=0; i<inHeight; i++) 
  {
    for (j=0; j<inWidth/2; j++)  
	{
 	    b  = *rgbp<<3;
	    g  = (*(rgbp+1)<<6)+((*rgbp&0xe0)>>2);
	    r  = (*(rgbp+1)&0x7c)<<1;
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];//>>1];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];//>>1];
		yp+=2;

 	    b  = *rgbp<<3;
	    g  = (*(rgbp+1)<<6)+((*rgbp&0xe0)>>2);
	    r  = (*(rgbp+1)&0x7c)<<1;
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    //*up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>1) + *up];
	    //*vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>1) + *vp];

		yp+=2;
	    up+=4;
	    vp+=4;
	}
	yp+=outWidth-inWidth*2;
	up+=outWidth-inWidth*2;
	vp+=outWidth-inWidth*2;
    rgbp-=inWidth*2*2;
  }
}



void colorConvYUV420toRGB32(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  int u,v;
  unsigned char *iclip;
  unsigned char *yp,*yp2,*up,*vp,*rgbp,*rgbp2;

  rgbp =out;
  rgbp2=out+outWidth;

  yp =in;
  yp2=in+inWidth;
  
  up =yp+inWidth*inHeight;
  vp =up+inWidth*inHeight/4;

  iclip = clip+512;

  for (i=0; i<inHeight/2; i++) 
  {
    for (j=0; j<inWidth/2; j++)  
	{
		u  = *up++;
		v  = *vp++;

		*rgbp++ = 	iclip[*yp + bu[u]];				//Blue
		*rgbp++ =	iclip[*yp - gu[u] - gv[v]];		//Green
		*rgbp++ = 	iclip[*yp + rv[v]];				//Red
  		rgbp++;										//Alpha
		yp++;

		*rgbp++ = 	iclip[*yp + bu[u]];				//Blue
		*rgbp++ =	iclip[*yp - gu[u] - gv[v]];		//Green
		*rgbp++ = 	iclip[*yp + rv[v]];				//Red
  		rgbp++;										//Alpha
		yp++;

		*rgbp2++ = 	iclip[*yp2 + bu[u]];			//Blue
		*rgbp2++ =	iclip[*yp2 - gu[u] - gv[v]];	//Green
		*rgbp2++ = 	iclip[*yp2 + rv[v]];			//Red
  		rgbp2++;									//Alpha
		yp2++;

		*rgbp2++ = 	iclip[*yp2 + bu[u]];			//Blue
		*rgbp2++ =	iclip[*yp2 - gu[u] - gv[v]];	//Green
		*rgbp2++ = 	iclip[*yp2 + rv[v]];			//Red
  		rgbp2++;									//Alpha
		yp2++;

	}
	yp+=inWidth;
	rgbp+=outWidth*2-inWidth*4;
	yp2+=inWidth;
	rgbp2+=outWidth*2-inWidth*4;
  } 
}

void colorConvYUYVtoRGB32(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp;

  yp = in;
  up = in+1;
  vp = in+3;

  iclip = clip+512;

  for (i=0; i<inHeight; i++)
  {
    for (j=0; j<inWidth/2; j++)  
	{

	  *out++ = 	iclip[*yp + bu[*up]];				//Blue
	  *out++ =	iclip[*yp - gu[*up] - gv[*vp]];		//Green
	  *out++ = 	iclip[*yp + rv[*vp]];				//Red
	  out++;										//Alpha

	  yp+=2;

	  *out++ = 	iclip[*yp + bu[*up]];				//Blue
	  *out++ =	iclip[*yp - gu[*up] - gv[*vp]];		//Green
	  *out++ = 	iclip[*yp + rv[*vp]];				//Red
	  out++;										//Alpha

	  yp+=2;
	  up+=4;
	  vp+=4;
	}
	out+=outWidth-inWidth*4;
  }
}

void colorConvRGB32toRGB32(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i;
  unsigned char *rgbp;

  inWidth*=4;

  rgbp =in+inWidth*(inHeight-1);

  for (i=0; i<inHeight; i++)
  {
    memcpy(out,rgbp,inWidth);
    out+=outWidth;
	rgbp-=inWidth;
  }
}

void colorConvRGB24toRGB32(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char *rgbp;

  rgbp =in+inWidth*3*(inHeight-1);

  for (i = 0; i < inHeight; i++) 
  {
    for (j = 0; j < inWidth; j++) 
    {
	  *out++=*rgbp++;
	  *out++=*rgbp++;
	  *out++=*rgbp++;
	  out++;
    }
	out+=outWidth-inWidth*4;
	rgbp-=inWidth*3*2;
  }
}

void colorConvRGB565toRGB32(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char *rgbp;

  rgbp =in+inWidth*2*(inHeight-1);

  for (i = 0; i < inHeight; i++) 
  {
    for (j = 0; j < inWidth; j++) 
    {
 	  *out++=*rgbp<<3;
	  *out++=(*(rgbp+1)<<5)+((*rgbp&0xe0)>>3);
	  *out++=(*(rgbp+1)&0xf8);
	  out++;
	  rgbp+=2;
    }
	out+=outWidth-inWidth*4;
	rgbp-=inWidth*2*2;
  }
}

void colorConvRGB555toRGB32(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char *rgbp;

  rgbp =in+inWidth*2*(inHeight-1);

  for (i = 0; i < inHeight; i++) 
  {
    for (j = 0; j < inWidth; j++) 
    {
 	  *out++=*rgbp<<3;
	  *out++=(*(rgbp+1)<<6)+((*rgbp&0xe0)>>2);
	  *out++=(*(rgbp+1)&0x7c)<<1;
	  out++;
	  rgbp+=2;
    }
	out+=outWidth-inWidth*4;
	rgbp-=inWidth*2*2;
  }
}



void colorConvYUV420toRGB24(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  int u,v;
  unsigned char *iclip;
  unsigned char *yp,*yp2,*up,*vp,*rgbp,*rgbp2;

  rgbp =out;
  rgbp2=out+outWidth;

  yp =in;
  yp2=in+inWidth;
  
  up =yp+inWidth*inHeight;
  vp =up+inWidth*inHeight/4;

  iclip = clip+512;

  for (i=0; i<inHeight/2; i++) 
  {
    for (j=0; j<inWidth/2; j++)  
	{
		u  = *up++;
		v  = *vp++;

		*rgbp++ = 	iclip[*yp + bu[u]];				//Blue
		*rgbp++ =	iclip[*yp - gu[u] - gv[v]];		//Green
		*rgbp++ = 	iclip[*yp + rv[v]];				//Red
		yp++;

		*rgbp++ = 	iclip[*yp + bu[u]];				//Blue
		*rgbp++ =	iclip[*yp - gu[u] - gv[v]];		//Green
		*rgbp++ = 	iclip[*yp + rv[v]];				//Red
		yp++;

		*rgbp2++ = 	iclip[*yp2 + bu[u]];			//Blue
		*rgbp2++ =	iclip[*yp2 - gu[u] - gv[v]];	//Green
		*rgbp2++ = 	iclip[*yp2 + rv[v]];			//Red
		yp2++;

		*rgbp2++ = 	iclip[*yp2 + bu[u]];			//Blue
		*rgbp2++ =	iclip[*yp2 - gu[u] - gv[v]];	//Green
		*rgbp2++ = 	iclip[*yp2 + rv[v]];			//Red
		yp2++;

	}
	yp+=inWidth;
	rgbp+=outWidth*2-inWidth*3;
	yp2+=inWidth;
	rgbp2+=outWidth*2-inWidth*3;
  } 
}

void colorConvYUYVtoRGB24(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp;

  yp = in;
  up = in+1;
  vp = in+3;

  iclip = clip+512;

  for (i=0; i<inHeight; i++)
  {
    for (j=0; j<inWidth/2; j++)  
	{

	  *out++ = 	iclip[*yp + bu[*up]];				//Blue
	  *out++ =	iclip[*yp - gu[*up] - gv[*vp]];		//Green
	  *out++ = 	iclip[*yp + rv[*vp]];				//Red

	  yp+=2;

	  *out++ = 	iclip[*yp + bu[*up]];				//Blue
	  *out++ =	iclip[*yp - gu[*up] - gv[*vp]];		//Green
	  *out++ = 	iclip[*yp + rv[*vp]];				//Red

	  yp+=2;
	  up+=4;
	  vp+=4;
	}
	out+=outWidth-inWidth*3;
  }
}

void colorConvRGB32toRGB24(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char *rgbp;

  rgbp =in+inWidth*4*(inHeight-1);

  for (i = 0; i < inHeight; i++) 
  {
    for (j = 0; j < inWidth; j++) 
    {
	  *out++=*rgbp++;
	  *out++=*rgbp++;
	  *out++=*rgbp++;
	  rgbp++;
    }
	out+=outWidth-inWidth*3;
	rgbp-=inWidth*4*2;
  }
}

void colorConvRGB24toRGB24(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i;
  unsigned char *rgbp;

  inWidth*=3;

  rgbp =in+inWidth*(inHeight-1);

  for (i=0; i<inHeight; i++)
  {
    memcpy(out,rgbp,inWidth);
    out+=outWidth;
	rgbp-=inWidth;
  }
}

void colorConvRGB565toRGB24(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char *rgbp;

  rgbp =in+inWidth*2*(inHeight-1);

  for (i = 0; i < inHeight; i++) 
  {
    for (j = 0; j < inWidth; j++) 
    {
 	  *out++=*rgbp<<3;
	  *out++=(*(rgbp+1)<<5)+((*rgbp&0xe0)>>3);
	  *out++=(*(rgbp+1)&0xf8);
	  rgbp+=2;
    }
	out+=outWidth-inWidth*3;
	rgbp-=inWidth*2*2;
  }
}

void colorConvRGB555toRGB24(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char *rgbp;

  rgbp =in+inWidth*2*(inHeight-1);

  for (i = 0; i < inHeight; i++) 
  {
    for (j = 0; j < inWidth; j++) 
    {
 	  *out++=*rgbp<<3;
	  *out++=(*(rgbp+1)<<6)+((*rgbp&0xe0)>>2);
	  *out++=(*(rgbp+1)&0x7c)<<1;
	  rgbp+=2;
    }
	out+=outWidth-inWidth*3;
	rgbp-=inWidth*2*2;
  }
}



void colorConvYUV420toRGB565(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  int u,v;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned char *yp,*yp2,*up,*vp;
  unsigned short *rgbp,*rgbp2;	
  
	rgbp =(unsigned short*)out;
	rgbp2=(unsigned short*)(out+outWidth);

	yp =in;
	yp2=in+inWidth;
	up =yp+inWidth*inHeight;
	vp =up+inWidth*inHeight/4;

	irclip = rclip565+512;
	igclip = gclip565+512;
	ibclip = bclip565+512;

	for (i=0; i<inHeight/2; i++) 
	{
	  for (j=0; j<inWidth/2; j++)  
	  {

	    u  = *up++;
		v  = *vp++;

		*rgbp++ = ibclip[*yp + bu[u]] | igclip[*yp - gu[u] - gv[v]] | irclip[*yp + rv[v]];
		yp++;

		*rgbp++ = ibclip[*yp + bu[u]] | igclip[*yp - gu[u] - gv[v]] | irclip[*yp + rv[v]];
		yp++;

		*rgbp2++ = ibclip[*yp2 + bu[u]] | igclip[*yp2 - gu[u] - gv[v]] | irclip[*yp2 + rv[v]];
		yp2++;

		*rgbp2++ = ibclip[*yp2 + bu[u]] | igclip[*yp2 - gu[u] - gv[v]] | irclip[*yp2 + rv[v]];
		yp2++;
	  }

	yp+=inWidth;
	rgbp+=outWidth-inWidth;
	yp2+=inWidth;
	rgbp2+=outWidth-inWidth;
	} 
}

void colorConvYUYVtoRGB565(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  int u,v;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned char *yp,*up,*vp;
  unsigned short *rgb;	

  yp = in;
  up = in+1;
  vp = in+3;
  
	irclip = rclip565+512;
	igclip = gclip565+512;
	ibclip = bclip565+512;

  for (i=0; i<inHeight; i++)
  {
	rgb =(unsigned short*)out;
    for (j=0; j<inWidth/2; j++)  
	{
	  u  = *up;
	  v  = *vp;

	  *rgb++ = ibclip[*yp + bu[u]] | igclip[*yp - gu[u] - gv[v]] | irclip[*yp + rv[v]];
	  yp+=2;

	  *rgb++ = ibclip[*yp + bu[u]] | igclip[*yp - gu[u] - gv[v]] | irclip[*yp + rv[v]];

	  yp+=2;
 	  up+=4;
	  vp+=4;
	}
	out+=outWidth;
	}
}

void colorConvRGB32toRGB565(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j,r,g,b;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned short *rgb;
  unsigned char *rgbp;

  rgbp =in+inWidth*4*(inHeight-1);
  
	irclip = rclip565+512;
	igclip = gclip565+512;
	ibclip = bclip565+512;

	for (i=0; i<inHeight; i++)
	{
	rgb=(unsigned short*)out;
		for (j=0; j<inWidth; j++) 
		{
			b=*rgbp++;
			g=*rgbp++;
			r=*rgbp++;
	    *rgb++ = ibclip[b] | igclip[g] | irclip[r];
      *rgbp++;
		}
	out+=outWidth;
	rgbp-=inWidth*4*2;
	}
}

void colorConvRGB24toRGB565(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j,r,g,b;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned short *rgb;
  unsigned char *rgbp;

  rgbp =in+inWidth*3*(inHeight-1);
  
	irclip = rclip565+512;
	igclip = gclip565+512;
	ibclip = bclip565+512;

	for (i=0; i<inHeight; i++)
	{
	rgb=(unsigned short*)out;
		for (j=0; j<inWidth; j++) 
		{
			b=*rgbp++;
			g=*rgbp++;
			r=*rgbp++;
	    *rgb++ = ibclip[b] | igclip[g] | irclip[r];
		}
	out+=outWidth;
	rgbp-=inWidth*3*2;
	}
}

void colorConvRGB565toRGB565(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i;
  unsigned char *rgbp;

  inWidth*=2;

  rgbp =in+inWidth*(inHeight-1);

  for (i=0; i<inHeight; i++)
  {
    memcpy(out,rgbp,inWidth);
    out+=outWidth;
	rgbp-=inWidth;
  }
}

void colorConvRGB555toRGB565(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
	unsigned char r,g,b;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned short *rgb;
  unsigned char *rgbp;

  rgbp =in+inWidth*2*(inHeight-1);
  
	irclip = rclip565+512;
	igclip = gclip565+512;
	ibclip = bclip565+512;

	for (i=0; i<inHeight; i++)
	{
	rgb=(unsigned short*)out;
		for (j=0; j<inWidth; j++) 
		{
 	    b=*rgbp<<3;
	    g=(*(rgbp+1)<<6)+((*rgbp&0xe0)>>2);
	    r=(*(rgbp+1)&0x7c)<<1;
	    *rgb++ = ibclip[b] | igclip[g] | irclip[r];
			rgbp+=2;
		}
	out+=outWidth;
	rgbp-=inWidth*2*2;
	}
}



void colorConvYUV420toRGB555(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  int u,v;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned char *yp,*yp2,*up,*vp;
  unsigned short *rgbp,*rgbp2;	
  
	rgbp =(unsigned short*)out;
	rgbp2=(unsigned short*)(out+outWidth);

	yp =in;
	yp2=in+inWidth;
	up =yp+inWidth*inHeight;
	vp =up+inWidth*inHeight/4;

	irclip = rclip555+512;
	igclip = gclip555+512;
	ibclip = bclip555+512;

	for (i=0; i<inHeight/2; i++) 
	{
	  for (j=0; j<inWidth/2; j++)  
	  {

	    u  = *up++;
		v  = *vp++;

		*rgbp++ = ibclip[*yp + bu[u]] | igclip[*yp - gu[u] - gv[v]] | irclip[*yp + rv[v]];
		yp++;

		*rgbp++ = ibclip[*yp + bu[u]] | igclip[*yp - gu[u] - gv[v]] | irclip[*yp + rv[v]];
		yp++;

		*rgbp2++ = ibclip[*yp2 + bu[u]] | igclip[*yp2 - gu[u] - gv[v]] | irclip[*yp2 + rv[v]];
		yp2++;

		*rgbp2++ = ibclip[*yp2 + bu[u]] | igclip[*yp2 - gu[u] - gv[v]] | irclip[*yp2 + rv[v]];
		yp2++;
	  }

	yp+=inWidth;
	rgbp+=outWidth-inWidth;
	yp2+=inWidth;
	rgbp2+=outWidth-inWidth;
	} 
}

void colorConvYUYVtoRGB555(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  int u,v;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned char *yp,*up,*vp;
  unsigned short *rgb;	

  yp = in;
  up = in+1;
  vp = in+3;
  
	irclip = rclip555+512;
	igclip = gclip555+512;
	ibclip = bclip555+512;

  for (i=0; i<inHeight; i++)
  {
	rgb =(unsigned short*)out;
    for (j=0; j<inWidth/2; j++)  
	{
	  u  = *up;
	  v  = *vp;

	  *rgb++ = ibclip[*yp + bu[u]] | igclip[*yp - gu[u] - gv[v]] | irclip[*yp + rv[v]];
	  yp+=2;

	  *rgb++ = ibclip[*yp + bu[u]] | igclip[*yp - gu[u] - gv[v]] | irclip[*yp + rv[v]];

	  yp+=2;
 	  up+=4;
	  vp+=4;
	}
	out+=outWidth;
	}
}

void colorConvRGB32toRGB555(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j,r,g,b;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned short *rgb;
  unsigned char *rgbp;

  rgbp =in+inWidth*4*(inHeight-1);
  
	irclip = rclip555+512;
	igclip = gclip555+512;
	ibclip = bclip555+512;

	for (i=0; i<inHeight; i++)
	{
	rgb=(unsigned short*)out;
		for (j=0; j<inWidth; j++) 
		{
			b=*rgbp++;
			g=*rgbp++;
			r=*rgbp++;
	    *rgb++ = ibclip[b] | igclip[g] | irclip[r];
      *rgbp++;
		}
	out+=outWidth;
	rgbp-=inWidth*4*2;
	}
}

void colorConvRGB24toRGB555(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j,r,g,b;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned short *rgb;
  unsigned char *rgbp;

  rgbp =in+inWidth*3*(inHeight-1);
  
	irclip = rclip555+512;
	igclip = gclip555+512;
	ibclip = bclip555+512;

	for (i=0; i<inHeight; i++)
	{
	rgb=(unsigned short*)out;
		for (j=0; j<inWidth; j++) 
		{
			b=*rgbp++;
			g=*rgbp++;
			r=*rgbp++;
	    *rgb++ = ibclip[b] | igclip[g] | irclip[r];
		}
	out+=outWidth;
	rgbp-=inWidth*3*2;
	}
}

void colorConvRGB565toRGB555(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned short *rgb;
  unsigned char *rgbp;

  rgbp =in+inWidth*2*(inHeight-1);
  
	irclip = rclip555+512;
	igclip = gclip555+512;
	ibclip = bclip555+512;

	for (i=0; i<inHeight; i++)
	{
	rgb=(unsigned short*)out;
		for (j=0; j<inWidth; j++) 
		{
 	    b=*rgbp<<3;
	    g=(*(rgbp+1)<<5)+((*rgbp&0xe0)>>3);
	    r=(*(rgbp+1)&0xf8);
	    *rgb++ = ibclip[b] | igclip[g] | irclip[r];
			rgbp+=2;
		}
	out+=outWidth;
	rgbp-=inWidth*2*2;
	}
}

void colorConvRGB555toRGB555(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i;
  unsigned char *rgbp;

  inWidth*=2;

  rgbp =in+inWidth*(inHeight-1);

  for (i=0; i<inHeight; i++)
  {
    memcpy(out,rgbp,inWidth);
    out+=outWidth;
	rgbp-=inWidth;
  }
}










void colorConvRGB32toYUV444(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp,*rgbp;

  rgbp =in;

  iclip = clip+512;

  for (i=0; i<inHeight; i++) 
  {
	yp =out+i*outWidth;
	up =yp+outWidth*outHeight;
	vp =up+outWidth*outHeight;
    
	for (j=0; j<inWidth; j++)  
	{
		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;
		rgbp++;

/*	    *yp = iclip[ (b + (g<<1) + r)>>2 ];
	    *up = iclip[ ((r - g)>>1) + 128];
	    *vp = iclip[ ((b - g)>>1) + 128];
*/
		*yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];
		
		yp++;
	    up++;
	    vp++;
	}
  }
}

void colorConvRGB24toYUV444(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp,*rgbp;

  rgbp =in;

  iclip = clip+512;

  for (i=0; i<inHeight; i++) 
  {
	yp =out+i*outWidth;
	up =yp+outWidth*outHeight;
	vp =up+outWidth*outHeight;

    for (j=0; j<inWidth; j++)  
	{
		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];
		yp++;
	    up++;
	    vp++;
	}
    yp+=inWidth;
  }
}

void colorConvRGB565toYUV444(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp,*rgbp;

  rgbp =in;

  iclip = clip+512;

  for (i=0; i<inHeight; i++) 
  {
	yp =out+i*outWidth;
	up =yp+outWidth*outHeight;
	vp =up+outWidth*outHeight;
    for (j=0; j<inWidth; j++)  
	{
		b  = *rgbp<<3;
		g  = (*(rgbp+1)<<5)+((*rgbp&0xe0)>>3);
		r  = (*(rgbp+1)&0xf8);
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];
		yp++;
	    up++;
	    vp++;
	}
  }
}

void colorConvRGB555toYUV444(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp,*rgbp;

  rgbp =in;

  iclip = clip+512;

  for (i=0; i<inHeight; i++) 
  {
	yp =out+i*outWidth;
	up =yp+outWidth*outHeight;
	vp =up+outWidth*outHeight;

    for (j=0; j<inWidth; j++)  
	{
		b  = *rgbp<<3;
		g  = (*(rgbp+1)<<6)+((*rgbp&0xe0)>>2);
		r  = (*(rgbp+1)&0x7c)<<1;
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)];
		yp++;
	    up++;
	    vp++;
	}
  }
}


void colorConvYUV444toRGB32(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  int u,v,g;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp,*rgbp;

  rgbp =out;

  yp =in;
  
  up =yp+inWidth*inHeight;
  vp =up+inWidth*inHeight;

  iclip = clip+512;

  for (i=0; i<inHeight; i++) 
  {
    for (j=0; j<inWidth; j++)  
	{
/*		u  = *up++-128;
		v  = *vp++-128;
		g  = *yp - ((u+v)>>1);	
		*rgbp++ = 	iclip[(v<<1)+g];
		*rgbp++ =	iclip[g];	
		*rgbp++ = 	iclip[(u<<1)+g];			
*/
		u  = *up++;
		v  = *vp++;
		*rgbp++ = 	iclip[*yp + bu[u]];				//Blue
		*rgbp++ =	iclip[*yp - gu[u] - gv[v]];		//Green
		*rgbp++ = 	iclip[*yp + rv[v]];				//Red
		
		rgbp++;								
		yp++;
	}
  } 
}



void colorConvYUV444toRGB24(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  int u,v;
  unsigned char *iclip;
  unsigned char *yp,*up,*vp,*rgbp;

  rgbp =out;

  yp =in;
  
  up =yp+inWidth*inHeight;
  vp =up+inWidth*inHeight;

  iclip = clip+512;

  for (i=0; i<inHeight; i++) 
  {
    for (j=0; j<inWidth; j++)  
	{
		u  = *up++;
		v  = *vp++;

		*rgbp++ = 	iclip[*yp + bu[u]];				//Blue
		*rgbp++ =	iclip[*yp - gu[u] - gv[v]];		//Green
		*rgbp++ = 	iclip[*yp + rv[v]];				//Red
		yp++;

	}
  } 
}




void colorConvYUV444toRGB565(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  int u,v;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned char *yp,*up,*vp;
  unsigned short *rgbp;	
  
	rgbp =(unsigned short*)out;

	yp =in;
	up =yp+inWidth*inHeight;
	vp =up+inWidth*inHeight;

	irclip = rclip565+512;
	igclip = gclip565+512;
	ibclip = bclip565+512;

	for (i=0; i<inHeight; i++) 
	{
	  for (j=0; j<inWidth; j++)  
	  {

	    u  = *up++;
		v  = *vp++;

		*rgbp++ = ibclip[*yp + bu[u]] | igclip[*yp - gu[u] - gv[v]] | irclip[*yp + rv[v]];
		yp++;
	  }
	} 
}

void colorConvYUV444toRGB555(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  int u,v;
  unsigned short *irclip,*igclip,*ibclip;
  unsigned char *yp,*up,*vp;
  unsigned short *rgbp;	
  
	rgbp =(unsigned short*)out;

	yp =in;
	up =yp+inWidth*inHeight;
	vp =up+inWidth*inHeight;

	irclip = rclip555+512;
	igclip = gclip555+512;
	ibclip = bclip555+512;

	for (i=0; i<inHeight; i++) 
	{
	  for (j=0; j<inWidth; j++)  
	  {

	    u  = *up++;
		v  = *vp++;

		*rgbp++ = ibclip[*yp + bu[u]] | igclip[*yp - gu[u] - gv[v]] | irclip[*yp + rv[v]];
		yp++;
	  }
	} 
}


void colorConvMJPEGtoYUV420(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
        /*!
    unsigned char *temp = new unsigned char[inWidth*inHeight*4];
	//convert from the color mode of the grabber to YUV420
	if (jpeg_decode(&temp, in, (int*)&inWidth, (int*)&inHeight) < 0) {
	    //printf("jpeg decode errors\n");
	}
	//fprintf (stderr, "jpeg decode %i %i\n",w,h);
	colorConvert[YUV420][YUYV](out, outWidth, temp, inWidth, inHeight);
	delete temp;
       */
}


void colorConvRGB32toYUV420(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned int offX, unsigned int offY, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*yp2,*up,*vp,*rgbp,*rgbp2;

  rgbp =in;
  rgbp2=in+inWidth*4;


  iclip = clip+512;

  for (i=0; i<inHeight/2; i++) 
  {
    yp =out+i*outWidth*2+offX+offY*outWidth;
    yp2=yp+outWidth;
    up =out+i*outWidth/2+outWidth*outHeight+offX/2+offY/2*outWidth/2;
    vp =up+outWidth*outHeight/4;

	for (j=0; j<inWidth/2; j++)  
	{
		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;
		rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)>>2];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)>>2];
		yp++;

		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;
		rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp++;

		b  = *rgbp2++;
		g  = *rgbp2++;
		r  = *rgbp2++;
		rgbp2++;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

		b  = *rgbp2++;
		g  = *rgbp2++;
		r  = *rgbp2++;
		rgbp2++;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

	    up++;
	    vp++;
	}
    rgbp+=inWidth*4;
    rgbp2+=inWidth*4;
  }
}

void colorConvRGB24toYUV420(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned int offX, unsigned int offY, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*yp2,*up,*vp,*rgbp,*rgbp2;

  rgbp =in;
  rgbp2=in+inWidth*3;

  iclip = clip+512;

  for (i=0; i<inHeight/2; i++) 
  {
    yp =out+i*outWidth*2+offX+offY*outWidth;
    yp2=yp+outWidth;
    up =out+i*outWidth/2+outWidth*outHeight+offX/2+offY/2*outWidth/2;
    vp =up+outWidth*outHeight/4;

    for (j=0; j<inWidth/2; j++)  
	{
		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)>>2];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)>>2];
		yp++;

		b  = *rgbp++;
		g  = *rgbp++;
		r  = *rgbp++;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp++;

		b  = *rgbp2++;
		g  = *rgbp2++;
		r  = *rgbp2++;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

		b  = *rgbp2++;
		g  = *rgbp2++;
		r  = *rgbp2++;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

	    up++;
	    vp++;
	}
    rgbp+=inWidth*3;
    rgbp2+=inWidth*3;
  }
}

void colorConvRGB565toYUV420(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned int offX, unsigned int offY, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*yp2,*up,*vp,*rgbp,*rgbp2;

  rgbp =in;
  rgbp2=in+inWidth*2;

  yp =out;
  yp2=out+outWidth;
  
  up =yp+outWidth*outHeight;
  vp =up+outWidth*outHeight/4;

  iclip = clip+512;

  for (i=0; i<inHeight/2; i++) 
  {
    yp =out+i*outWidth*2+offX+offY*outWidth;
    yp2=yp+outWidth;
    up =out+i*outWidth/2+outWidth*outHeight+offX/2+offY/2*outWidth/2;
    vp =up+outWidth*outHeight/4;

    for (j=0; j<inWidth/2; j++)  
	{
		b  = *rgbp<<3;
		g  = (*(rgbp+1)<<5)+((*rgbp&0xe0)>>3);
		r  = (*(rgbp+1)&0xf8);
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)>>2];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)>>2];
		yp++;

		b  = *rgbp<<3;
		g  = (*(rgbp+1)<<5)+((*rgbp&0xe0)>>3);
		r  = (*(rgbp+1)&0xf8);
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp++;

		b  = *rgbp2<<3;
		g  = (*(rgbp2+1)<<5)+((*rgbp2&0xe0)>>3);
		r  = (*(rgbp2+1)&0xf8);
		rgbp2+=2;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

		b  = *rgbp2<<3;
		g  = (*(rgbp2+1)<<5)+((*rgbp2&0xe0)>>3);
		r  = (*(rgbp2+1)&0xf8);
		rgbp2+=2;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

	    up++;
	    vp++;
	}
    rgbp+=inWidth*2;
    rgbp2+=inWidth*2;
  }
}

void colorConvRGB555toYUV420(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned int offX, unsigned int offY, unsigned char *in, unsigned int inWidth, unsigned int inHeight)
{
  unsigned int i,j;
  unsigned char r,g,b;
  unsigned char *iclip;
  unsigned char *yp,*yp2,*up,*vp,*rgbp,*rgbp2;

  rgbp =in;
  rgbp2=in+inWidth*2;

  yp =out;
  yp2=out+outWidth;
  
  up =yp+outWidth*outHeight;
  vp =up+outWidth*outHeight/4;

  iclip = clip+512;

  for (i=0; i<inHeight/2; i++) 
  {
    yp =out+i*outWidth*2+offX+offY*outWidth;
    yp2=yp+outWidth;
    up =out+i*outWidth/2+outWidth*outHeight+offX/2+offY/2*outWidth/2;
    vp =up+outWidth*outHeight/4;

    for (j=0; j<inWidth/2; j++)  
	{
		b  = *rgbp<<3;
		g  = (*(rgbp+1)<<6)+((*rgbp&0xe0)>>2);
		r  = (*(rgbp+1)&0x7c)<<1;
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[ ((b>>1) - ug[ g ] - ur[ r ] + 128)>>2];
	    *vp = iclip[ ((r>>1) - vg[ g ] - vb[ b ] + 128)>>2];
		yp++;

		b  = *rgbp<<3;
		g  = (*(rgbp+1)<<6)+((*rgbp&0xe0)>>2);
		r  = (*(rgbp+1)&0x7c)<<1;
		rgbp+=2;

	    *yp = iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp++;

		b  = *rgbp2<<3;
		g  = (*(rgbp2+1)<<6)+((*rgbp2&0xe0)>>2);
		r  = (*(rgbp2+1)&0x7c)<<1;
		rgbp2+=2;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

		b  = *rgbp2<<3;
		g  = (*(rgbp2+1)<<6)+((*rgbp2&0xe0)>>2);
		r  = (*(rgbp2+1)&0x7c)<<1;
		rgbp2+=2;

	    *yp2= iclip[ yb[ b ] + yg[ g ] + yr[ r ] ];
	    *up = iclip[(((b>>1) - ug[ g ] - ur[ r ] + 128)>>2) + *up];
	    *vp = iclip[(((r>>1) - vg[ g ] - vb[ b ] + 128)>>2) + *vp];
		yp2++;

	    up++;
	    vp++;
	}
    rgbp+=inWidth*2;
    rgbp2+=inWidth*2;
  }
}



colorConverter* colorConvert[8][9]=
        { colorConvYUV420toYUV420,colorConvYUYVtoYUV420,colorConvRGB32toYUV420,colorConvRGB24toYUV420,colorConvRGB565toYUV420,colorConvRGB555toYUV420,0                      ,colorConvUYVYtoYUV420, colorConvMJPEGtoYUV420,
      colorConvYUV420toYUYV,  colorConvYUYVtoYUYV,  colorConvRGB32toYUYV,  colorConvRGB24toYUYV,  colorConvRGB565toYUYV,  colorConvRGB555toYUYV,  0                      ,0                    ,0,
      colorConvYUV420toRGB32, colorConvYUYVtoRGB32, colorConvRGB32toRGB32, colorConvRGB24toRGB32, colorConvRGB565toRGB32, colorConvRGB555toRGB32, colorConvYUV444toRGB32 ,0                    ,0,
      colorConvYUV420toRGB24, colorConvYUYVtoRGB24, colorConvRGB32toRGB24, colorConvRGB24toRGB24, colorConvRGB565toRGB24, colorConvRGB555toRGB24, colorConvYUV444toRGB24 ,0                    ,0,
      colorConvYUV420toRGB565,colorConvYUYVtoRGB565,colorConvRGB32toRGB565,colorConvRGB24toRGB565,colorConvRGB565toRGB565,colorConvRGB555toRGB565,colorConvYUV444toRGB565,0                    ,0,
      colorConvYUV420toRGB555,colorConvYUYVtoRGB555,colorConvRGB32toRGB555,colorConvRGB24toRGB555,colorConvRGB565toRGB555,colorConvRGB555toRGB555,colorConvYUV444toRGB555,0                    ,0,
      0                      ,0                    ,0                     ,0                     ,0                      ,0                      ,0                      ,0                    ,0,
      0                      ,0                    ,0                     ,0                     ,0                      ,0                      ,0                      ,0                    ,0,
	};

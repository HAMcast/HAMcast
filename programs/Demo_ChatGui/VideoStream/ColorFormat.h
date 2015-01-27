
#ifndef __COLORFORMAT_H__
#define __COLORFORMAT_H__

enum ColorFormat {
   YUV420,
   YUYV,
   RGB32,
   RGB24,
   RGB565,
   RGB555,
   YUV444,
   UYVY,
   MJPEG
};

extern void initColorLUT();

typedef void colorConverter(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight);

extern colorConverter* colorConvert[8][9];

void colorConvRGB32toYUV420(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned int offX, unsigned int offY, unsigned char *in, unsigned int inWidth, unsigned int inHeight);
void colorConvRGB24toYUV420(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned int offX, unsigned int offY, unsigned char *in, unsigned int inWidth, unsigned int inHeight);
void colorConvRGB565toYUV420(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned int offX, unsigned int offY, unsigned char *in, unsigned int inWidth, unsigned int inHeight);
void colorConvRGB555toYUV420(unsigned char *out, unsigned int outWidth, unsigned int outHeight, unsigned int offX, unsigned int offY, unsigned char *in, unsigned int inWidth, unsigned int inHeight);

void colorConvYUV420toRGB32(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight);
void colorConvYUV420toRGB24(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight);
void colorConvYUV420toRGB565(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight);
void colorConvYUV420toRGB555(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight);
void colorConvYUV420toYUYV(unsigned char *out, unsigned int outWidth, unsigned char *in, unsigned int inWidth, unsigned int inHeight);

#endif

#include <memory.h>
#include "qimagescale.h"

/*
 * Copyright (C) 2004, 2005 Daniel M. Duley
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* OTHER CREDITS:
 *
 * This is the normal smoothscale method, based on Imlib2's smoothscale.
 *
 * Originally I took the algorithm used in NetPBM and Qt and added MMX/3dnow
 * optimizations. It ran in about 1/2 the time as Qt. Then I ported Imlib's
 * C algorithm and it ran at about the same speed as my MMX optimized one...
 * Finally I ported Imlib's MMX version and it ran in less than half the
 * time as my MMX algorithm, (taking only a quarter of the time Qt does).
 * After further optimization it seems to run at around 1/6th.
 *
 * Changes include formatting, namespaces and other C++'ings, removal of old
 * #ifdef'ed code, and removal of unneeded border calculation code.
 *
 * Imlib2 is (C) Carsten Haitzler and various contributors. The MMX code
 * is by Willem Monsuwe <willem@stack.nl>. All other modifications are
 * (C) Daniel M. Duley.
 */


namespace MyImageScale {
    struct MyImageScaleInfo {
        int *xpoints;
        unsigned char **ypoints;
        int *xapoints, *yapoints;
        int xup_yup;
    };

    unsigned char** qimageCalcYPoints(unsigned char *src, int sw, int sh,
                                     int dh);
    int* qimageCalcXPoints(int sw, int dw);
    int* qimageCalcApoints(int s, int d, int up);
    MyImageScaleInfo* qimageFreeScaleInfo(MyImageScaleInfo *isi);
    MyImageScaleInfo *qimageCalcScaleInfo(unsigned char *pic, int sw, int sh,
                                         int dw, int dh, char aa);
}

using namespace MyImageScale;

//
// Code ported from Imlib...
//


#define B_VAL(p) (*p)

#define INV_XAP                   (256 - xapoints[x])
#define XAP                       (xapoints[x])
#define INV_YAP                   (256 - yapoints[dyy + y])
#define YAP                       (yapoints[dyy + y])

unsigned char** MyImageScale::qimageCalcYPoints(unsigned char *src,
                                              int sw, int sh, int dh)
{
    unsigned char **p;
    int i, j = 0;
    int val, inc, rv = 0;

    if(dh < 0){
        dh = -dh;
        rv = 1;
    }
    p = new unsigned char* [dh+1];

    val = 0;
    inc = (sh << 16) / dh;
    for(i = 0; i < dh; i++){
        p[j++] = src + ((val >> 16) * sw);
        val += inc;
    }
    if(rv){
        for(i = dh / 2; --i >= 0; ){
            unsigned char *tmp = p[i];
            p[i] = p[dh - i - 1];
            p[dh - i - 1] = tmp;
        }
    }
    return(p);
}

int* MyImageScale::qimageCalcXPoints(int sw, int dw)
{
    int *p, i, j = 0;
    int val, inc, rv = 0;

    if(dw < 0){
        dw = -dw;
        rv = 1;
    }
    p = new int[dw+1];

    val = 0;
    inc = (sw << 16) / dw;
    for(i = 0; i < dw; i++){
        p[j++] = (val >> 16);
        val += inc;
    }

    if(rv){
        for(i = dw / 2; --i >= 0; ){
            int tmp = p[i];
            p[i] = p[dw - i - 1];
            p[dw - i - 1] = tmp;
        }
    }
   return(p);
}

int* MyImageScale::qimageCalcApoints(int s, int d, int up)
{
    int *p, i, j = 0, rv = 0;

    if(d < 0){
        rv = 1;
        d = -d;
    }
    p = new int[d];

    /* scaling up */
    if(up){
        int val, inc;

        val = 0;
        inc = (s << 16) / d;
        for(i = 0; i < d; i++){
            p[j++] = (val >> 8) - ((val >> 8) & 0xffffff00);
            if((val >> 16) >= (s - 1))
                p[j - 1] = 0;
            val += inc;
        }
    }
    /* scaling down */
    else{
        int val, inc, ap, Cp;
        val = 0;
        inc = (s << 16) / d;
        Cp = ((d << 14) / s) + 1;
        for(i = 0; i < d; i++){
            ap = ((0x100 - ((val >> 8) & 0xff)) * Cp) >> 8;
            p[j] = ap | (Cp << 16);
            j++;
            val += inc;
        }
    }
    if(rv){
        int tmp;
        for(i = d / 2; --i >= 0; ){
            tmp = p[i];
            p[i] = p[d - i - 1];
            p[d - i - 1] = tmp;
        }
    }
    return(p);
}

MyImageScaleInfo* MyImageScale::qimageFreeScaleInfo(MyImageScaleInfo *isi)
{
    if(isi){
        delete[] isi->xpoints;
        delete[] isi->ypoints;
        delete[] isi->xapoints;
        delete[] isi->yapoints;
        delete isi;
    }
    return 0;
}

MyImageScaleInfo* MyImageScale::qimageCalcScaleInfo(unsigned char *pic,
                                                  int sw, int sh,
                                                  int dw, int dh, char aa)
{
    MyImageScaleInfo *isi;
    int scw, sch;

    scw = dw;
    sch = dh;

    isi = new MyImageScaleInfo;
    if(!isi)
        return 0;
    memset(isi, 0, sizeof(MyImageScaleInfo));

    isi->xup_yup = (dw >= sw) + ((dh >= sh) << 1);

    isi->xpoints = qimageCalcXPoints(sw, scw);
    if(!isi->xpoints)
        return(qimageFreeScaleInfo(isi));
    isi->ypoints = qimageCalcYPoints(pic,
                                     sw, sh, sch);
    if (!isi->ypoints)
        return(qimageFreeScaleInfo(isi));
    if(aa){
        isi->xapoints = qimageCalcApoints(sw, scw, isi->xup_yup & 1);
        if(!isi->xapoints)
            return(qimageFreeScaleInfo(isi));
        isi->yapoints = qimageCalcApoints(sh, sch, isi->xup_yup & 2);
        if(!isi->yapoints)
            return(qimageFreeScaleInfo(isi));
    }
    return(isi);
}

/* scale by area sampling - IGNORE the ALPHA byte*/
static void qt_qimageScaleAARGB(MyImageScaleInfo *isi, unsigned char *dest,
                                int dxx, int dyy, int dx, int dy, int dw,
                                int dh, int dow, int sow)
{
    unsigned char *sptr, *dptr;
    int x, y, end;
    unsigned char **ypoints = isi->ypoints;
    int *xpoints = isi->xpoints;
    int *xapoints = isi->xapoints;
    int *yapoints = isi->yapoints;

    end = dxx + dw;
	{
        /*\ 'Correct' version, with math units prepared for MMXification \*/
        int Cx, Cy, i, j;
        unsigned char *pix;
        int b, bx;
        int xap, yap;

        for(y = 0; y < dh; y++){
            Cy = YAP >> 16;
            yap = YAP & 0xffff;

            dptr = dest + dx + ((y + dy) * dow);
            for(x = dxx; x < end; x++){
                Cx = XAP >> 16;
                xap = XAP & 0xffff;

                sptr = ypoints[dyy + y] + xpoints[x];
                pix = sptr;
                sptr += sow;
                bx = (B_VAL(pix) * xap) >> 9;
                pix++;
                for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                    bx += (B_VAL(pix) * Cx) >> 9;
                    pix++;
                }
                if(i > 0){
                    bx += (B_VAL(pix) * i) >> 9;
                }

                b = (bx * yap) >> 14;

                for(j = (1 << 14) - yap; j > Cy; j -= Cy){
                    pix = sptr;
                    sptr += sow;
                    bx = (B_VAL(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        bx += (B_VAL(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        bx += (B_VAL(pix) * i) >> 9;
                    }

                    b += (bx * Cy) >> 14;
                }
                if(j > 0){
                    pix = sptr;
                    sptr += sow;
                    bx = (B_VAL(pix) * xap) >> 9;
                    pix++;
                    for(i = (1 << 14) - xap; i > Cx; i -= Cx){
                        bx += (B_VAL(pix) * Cx) >> 9;
                        pix++;
                    }
                    if(i > 0){
                        bx += (B_VAL(pix) * i) >> 9;
                    }

                    b += (bx * j) >> 14;
                }

                *dptr = b >> 5;
                dptr++;
            }
        }
    }
}


void qSmoothScaleImage(unsigned char *in, int w, int h, unsigned char *out, int dw, int dh)
{
    MyImageScaleInfo *scaleinfo =
        qimageCalcScaleInfo(in, w, h, dw, dh, true);
    if (!scaleinfo)
        return;

    qt_qimageScaleAARGB(scaleinfo, out,
                          0, 0, 0, 0, dw, dh, dw, w);

    qimageFreeScaleInfo(scaleinfo);
}

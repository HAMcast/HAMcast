//  This software has been developed by 
//  daViKo Gesellschaft fr digitale audiovisuelle Kommunikation mbH 
//  Copyright daViKo GmbH 2005
//
//  CONFIDENTIALITY:
//
//      This file is the property of daViKo GmbH.
//      It contains information that is regarded as privilege
//      and confidential by daViKo GmbH.
//      It may not be publicly disclosed or communicated in any way without 
//      prior written authorization by daViKo GmbH.
//      It cannot be copied, used, or modified without obtaining
//      an authorization from daViKo GmbH.
//      If such an authorization is provided, any modified version or
//      copy of the software has to contain this header.
//
//  WARRANTIES: 
//      This software is provided as << is >>, daViKo GmbH 
//      makes no warranty express or implied with respect to this software, 
//      its fitness for a particular purpose or its merchantability. 
//      In no case, shall daViKo GmbH be liable for any 
//      incidental or consequential damages, including but not limited 
//      to lost profits.
//
//      daViKo GmbH shall be under no obligation or liability in respect of 
//      any infringement of statutory monopoly or intellectual property 
//      rights of third parties by the use of elements of such software 
//      and the user shall in any case be entirely responsible for the use 
//      to which he puts such elements.
//
//  AUTHORS:
//      Mark Palkow <palkow@daviko.com>
//

//#include "wintypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <linux/videodev.h>
#include <QString>
#include <QList>
#include <iostream>
#include "frame_converter.h"
#include "ColorFormat.h"


typedef struct
{
  int video_fd;
  int type;
  struct video_mmap mm;
  struct video_mbuf mb;
  unsigned char * data[4];
  int width,height;			//Breite, Hhe des Frames
  unsigned char *yuvBuffer;   //Buffer fr Farbconvertierung
  unsigned char *rgb;
  unsigned char *mem;
  frame_converter *fc;		//Objekt zur Frameskalierung
  ColorFormat color;
  bool useRead;
}
VideoInDeviceHandler;


/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function VideoInExit
*
* \brief    close the capturing of video frames
*
* \param    handle:   (in) a handle returned by a previous call to VideoInInit()
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
void VideoInExit(void* handle)
{
  VideoInDeviceHandler *m_handle = (VideoInDeviceHandler*) handle;

  if (m_handle->type==2)
  {
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(m_handle->video_fd, VIDIOC_STREAMOFF, &type);
  }

  if (!m_handle->useRead)
  {
    for (int i = 0; i < 4; i++)
      if (m_handle->data[i]) munmap (m_handle->data[i], m_handle->mb.size);
  }
  fprintf (stderr, "v4l close\n");

  close(m_handle->video_fd);

  delete m_handle->yuvBuffer;
  m_handle->yuvBuffer = NULL;

  delete m_handle->rgb;
  delete m_handle->mem;

	delete m_handle->fc;
	m_handle->fc = NULL;
}

/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function VideoInInit
*
* \brief    starts capturing of video frame
*
* \param    width:        (in) width of video frame (must be suported by the device)
*           height:       (in) height of video frame (must be suported by the device)
*           FramesPerSec: (in) capture rate (30fps for NTSC 25fps for PAL devices)
*           video_device: (in) number of the device that should be initialized
*
* \return   0 on error, handle to video capturing instance if success
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
void* VideoInInit(int width, int height, float FramesPerSec, int video_device,char const * device_name)
{
  struct v4l2_capability	v4l2capability;
  struct v4l2_streamparm	v4l2streamparm;
  struct v4l2_requestbuffers v4l2requestbuffers;
  struct v4l2_buffer 		video_buffer[4];
  struct v4l2_format		v4l2format;
  int err;


  VideoInDeviceHandler *handle = new VideoInDeviceHandler;
  if (!handle) return 0;

  handle->yuvBuffer = new unsigned char[width*height*3/2*10]; //yuv buffer used by color convert
  if (handle->yuvBuffer==NULL)
  {
    return 0;
  }
  handle->rgb = new unsigned char[640*480*4]; //yuv buffer used by color convert
  if (handle->yuvBuffer==NULL)
  {
    return 0;
  }
  handle->mem = new unsigned char[640*480*4]; //yuv buffer used by color convert
  if (handle->yuvBuffer==NULL)
  {
    return 0;
  }

  if (width<=320)
  {
  handle->width = 320;
  handle->height = 240;
  }
  else
  {
  handle->width = 640;
  handle->height = 480;
  }

  handle->fc = new frame_converter(width, height); //mit Zielgroesse initialisieren

  handle->type = 0;
  handle->useRead = false;
  handle->data[0] = 0;
  handle->data[1] = 0;
  handle->data[2] = 0;
  handle->data[3] = 0;


  char device[80];
  sprintf(device,"/dev/video%i",video_device);

  handle->video_fd = ::open(device, O_RDWR);
  if (handle->video_fd<=0)
  {
    fprintf( (FILE *)stderr, "open error\n");
    return 0;
  }

  err = ioctl( handle->video_fd, VIDIOC_QUERYCAP ,  &v4l2capability  ) ;
  if (err == 0)
  {
    if ((v4l2capability.capabilities & V4L2_CAP_VIDEO_CAPTURE)==0)
    {
      return 0;
    }
/*
    if (!(v4l2capability.capabilities & V4L2_CAP_STREAMING))
    {
      fprintf( (FILE *)stderr, "\nERROR:Device %s can't accept frames or data via asynchronously via pre-allocated buffers.\n",(char *) video_device);
      return 0;
    }
    else
      fprintf((FILE *)stderr, "\nCHECK:stream.\n");
*/

    v4l2format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    err = ioctl( handle->video_fd, VIDIOC_G_FMT, &v4l2format);
    if (err < 0)
    {
      fprintf( (FILE *)stderr, "VIDEO_GETFORMAT:ioctl error\n");
      return 0;

    }
fprintf( (FILE *)stderr, "VIDEO_GETFORMAT %i %i %i\n",v4l2format.fmt.pix.width,v4l2format.fmt.pix.height,v4l2format.fmt.pix.pixelformat);

    v4l2format.fmt.pix.width		= handle->width;
    v4l2format.fmt.pix.height		= handle->height;
    v4l2format.fmt.pix.pixelformat	= V4L2_PIX_FMT_YUV420;
    handle->color = YUV420;
    
    err = ioctl( handle->video_fd, VIDIOC_S_FMT, &v4l2format);
    if (err < 0)
    {
	v4l2format.fmt.pix.width	= handle->width;
	v4l2format.fmt.pix.height	= handle->height;
	v4l2format.fmt.pix.pixelformat	= V4L2_PIX_FMT_YUYV;
        handle->color = YUYV;

	err = ioctl( handle->video_fd, VIDIOC_S_FMT, &v4l2format);
	if (err < 0)
	{
		goto MJPEG;
	}
	err = ioctl( handle->video_fd, VIDIOC_G_FMT, &v4l2format);
	if (err < 0)
	{
		fprintf( (FILE *)stderr, "VIDEO_GETFORMAT:ioctl error\n");
		return 0;
	}
	if ((v4l2format.fmt.pix.width!=handle->width)||(v4l2format.fmt.pix.height!=handle->height))
	{
MJPEG:		fprintf( (FILE *)stderr, "try MJPEG\n");
		v4l2format.fmt.pix.width	= handle->width;
		v4l2format.fmt.pix.height	= handle->height;
		v4l2format.fmt.pix.pixelformat	= V4L2_PIX_FMT_MJPEG;
		handle->color = MJPEG;
	
		err = ioctl( handle->video_fd, VIDIOC_S_FMT, &v4l2format);
		if (err < 0)
		{
			fprintf( (FILE *)stderr, "VIDEO_SETFORMAT:ioctl error\n");
			return 0;
		}
	}
    }
fprintf( (FILE *)stderr, "VIDEO_SETFORMAT %i %i %i\n",v4l2format.fmt.pix.width,v4l2format.fmt.pix.height,v4l2format.fmt.pix.pixelformat);

    int numStreamBuffs = 4;
    v4l2requestbuffers.count = numStreamBuffs;
    v4l2requestbuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2requestbuffers.memory = V4L2_MEMORY_MMAP;

    err = ioctl ( handle->video_fd, VIDIOC_REQBUFS, &v4l2requestbuffers );
    if (err < 0 || v4l2requestbuffers.count < 1)
    {
      fprintf((FILE *)stderr, "ERROR:REQBUFS\n");
      return 0;
    }


    // Query each buffer and map it to the video device
    for ( int i = 0; i < v4l2requestbuffers.count; ++i)
    {
      struct v4l2_buffer *vbuffer = &video_buffer[i];

      vbuffer->index = i;

      vbuffer->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

      err = ioctl ( handle->video_fd , VIDIOC_QUERYBUF, vbuffer );
      if (err < 0)
      {
        fprintf((FILE *)stderr,"QUERYBUF:workaround\n");
        // check support function read()
        if (!(v4l2capability.capabilities & V4L2_CAP_READWRITE))
        {
          fprintf( (FILE *)stderr, "\nERROR:Device doesn't support read().\n");
          goto V4L;
        }
        else
        {
          handle->useRead = true;
 fprintf (stderr, "v4l use read\n");
        }
      }
      else
      {
      handle->data[i] =(unsigned char *) mmap (0, vbuffer->length, PROT_READ|PROT_WRITE, MAP_SHARED, handle->video_fd , vbuffer->m.offset);
      if (handle->data[i] ==(void*) -1)
      {
        fprintf((FILE *)stderr,"ERROR:mmap()\n");
        return 0;
      }
      handle->mb.size = vbuffer->length;
      err = ioctl (  handle->video_fd, VIDIOC_QBUF, vbuffer );
      if (err)
      {
        fprintf((FILE *)stderr,"ERROR:QBUF\n");
        return 0;
      }
      }
    }

    if (!handle->useRead)
    {
      // Set video stream capture on
      err = ioctl ( handle->video_fd, VIDIOC_STREAMON, &video_buffer[0].type);
      if (err)
      {
        fprintf((FILE *)stderr,"ERROR:STREAMON\n");
        VideoInExit(handle);
        return 0;
      }
    }
    handle->type=2;
  }
  else
  {
V4L:    struct video_capability vcap;
    if (ioctl(handle->video_fd, VIDIOCGCAP, &vcap) == 0)
    {
      struct video_window    vw;

      if(ioctl(handle->video_fd, VIDIOCGWIN, &vw) < 0)
      {
        fprintf (stderr, "ERROR:V4L GWIN\n");
        return 0;
      }

      vw.width = handle->width;
      vw.height = handle->height;

      if(ioctl(handle->video_fd, VIDIOCSWIN, &vw) < 0)
      {
        fprintf (stderr, "ERROR:V4L SWIN\n");
        return 0;
      }

      struct video_picture    vp;

      if(ioctl(handle->video_fd, VIDIOCGPICT, &vp) < 0)
      {
        fprintf (stderr, "ERROR:V4L GPICTn");
        return 0;
      }

      //vp.palette = VIDEO_PALETTE_YUYV;
      //vp.depth = 16;
      //handle->color = YUYV;
      vp.palette = VIDEO_PALETTE_YUV420P;
      vp.depth = 12;
      handle->color = YUV420;

      if(ioctl(handle->video_fd, VIDIOCSPICT, &vp) < 0)
      {
        fprintf (stderr, "ERROR:V4L SPICT\n");
        return 0;
      }



      if(ioctl(handle->video_fd, VIDIOCGMBUF, &handle->mb) < 0)
      {
        fprintf (stderr, "ERROR:V4L GMBUF\n");
        return 0;
      }

      handle->data[0] = (unsigned char*)mmap(0, handle->mb.size,
                                             PROT_READ|PROT_WRITE,MAP_SHARED, handle->video_fd, 0);

      if(handle->data[0] == 0)
      {
        fprintf (stderr, "ERROR:V4L data\n");
        return 0;
      }

      handle->mm.frame  = 0;
      handle->mm.height = handle->height;
      handle->mm.width  = handle->width;
      handle->mm.format = vp.palette;

      if(ioctl(handle->video_fd, VIDIOCMCAPTURE, &handle->mm) < 0)
      {
        fprintf (stderr, "ERROR:V4L CAPT\n");
        return 0;
      }

      handle->type=1;
    }
  }

//  handle->width = width;
//  handle->height = height;


  return handle;
}



/*!
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*
* \function VideoInGet
*
* \brief    get one caputure video frame
*
* \param    handle:   (in) a handle returned by a previous call to VideoInInit()
*           yuv:      (out) pointer where the frame will be given to (VideoIn will handle frame memory)
*
* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
int VideoInGet(void* handle, unsigned char **yuv)
{
  int err, n;
  fd_set rdset;
  struct timeval timeout;

  VideoInDeviceHandler *m_handle = (VideoInDeviceHandler*) handle;

  if (m_handle->type==2)
  {
    if (m_handle->useRead)
    {
        read ( m_handle->video_fd , m_handle->rgb, m_handle->width*m_handle->height*3/2 );
	//convert from the color mode of the grabber to YUV420
	colorConvert[YUV420][m_handle->color](m_handle->mem,m_handle->width,m_handle->rgb,m_handle->width,m_handle->height);
	if (m_handle->fc) m_handle->fc->convertFrame(m_handle->width, m_handle->height, m_handle->mem, m_handle->yuvBuffer);
    }
    else
    {
    FD_ZERO (&rdset);
    FD_SET ( m_handle->video_fd , &rdset);

    timeout.tv_sec = 1;  // max time
    timeout.tv_usec = 0;

    n = select (  m_handle->video_fd  + 1, &rdset, NULL, NULL, &timeout);
    if (n == -1)
    {
      return n;
    }
    else if (n == 0)
    {
      fprintf (stderr, "select timeout\n");
    }
    else if (FD_ISSET ( m_handle->video_fd , &rdset))
    {

      struct v4l2_buffer   tempbuf;

      tempbuf.memory = V4L2_MEMORY_MMAP;
      tempbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

      err = ioctl ( m_handle->video_fd  , VIDIOC_DQBUF ,  &tempbuf );
      if (err)
      {
        fprintf (stderr, "ERROR:DQBUF\n");
	usleep(100000);
        return err;
      }
	
	colorConvert[YUV420][m_handle->color](m_handle->mem, m_handle->width, m_handle->data[tempbuf.index], m_handle->width, m_handle->height);
        if (m_handle->fc) m_handle->fc->convertFrame(m_handle->width, m_handle->height, m_handle->mem, m_handle->yuvBuffer);

      err = ioctl ( m_handle->video_fd , VIDIOC_QBUF,  &tempbuf);
      if (err)
      {
        fprintf (stderr, "ERROR:QBUF\n");
        return err;
      }
    }
    }
  }
  else if (m_handle->type==1)
  {

    unsigned int p=m_handle->mm.frame;

    m_handle->mm.frame  = (p+1) % m_handle->mb.frames;

    if(ioctl(m_handle->video_fd, VIDIOCMCAPTURE, &m_handle->mm) < 0)
    {
      return -1;
    }

    if(ioctl(m_handle->video_fd, VIDIOCSYNC, &p) < 0)
    {
      return -1;
    }
	//convert from the color mode of the grabber to YUV420
	colorConvert[YUV420][m_handle->color](m_handle->mem,m_handle->width, m_handle->data[0]+m_handle->mb.offsets[p],m_handle->width,m_handle->height);
	if (m_handle->fc) m_handle->fc->convertFrame(m_handle->width, m_handle->height, m_handle->mem, m_handle->yuvBuffer);
  }
  *yuv=m_handle->yuvBuffer;

  return true;
}







//BOOL VideoInDlgSource(int handle, HWND wnd){std::cout <<"NOT IMPLEMENTED"<<std::endl;exit(1);}


void VideoInDemoTime(void* handle, unsigned char *YUY2_Buffer){std::cout <<"NOT IMPLEMENTED 1"<<std::endl;}
float VideoInGetFPS(void* handle){std::cout <<"NOT IMPLEMENTED 2"<<std::endl;return 30.0f;}

QList<QString> VideoInGetDevices(){
    std::cout <<"NOT IMPLEMENTED"<<std::endl;
    QList<QString> device_list;
    device_list.append(QString("blub"));
    return device_list;
}



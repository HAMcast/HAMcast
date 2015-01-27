/* libVLC and Qt sample code
 * Copyright © 2009 Alexander Maringer <maringer@maringer-it.de>
 */
#include "vlc_on_qt.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QFrame>
#include <vlcStreamer/udpreciever.h>

Player::Player(QSlider* slider,QWidget * parent)
: QWidget(parent)
{
    //preparation of the vlc command
    const char * const vlc_args[] = {
              "-I", "dummy", /* Don't use any interface */
              "--ignore-config", /* Don't use VLC's config */
              "--extraintf=logger", //log anything
              "--verbose=2", //be much more verbose then normal for debugging purpose
              "--plugin-path=C:\\vlc-0.9.9-win32\\plugins\\" };


    _videoWidget=new QFrame(this);

    // [20101215 JG] If KDE is used like unique desktop environment, only use _videoWidget=new QFrame(this);

    _volumeSlider=slider;
    _volumeSlider->setMaximum(100); //the volume is between 0 and 100
    _volumeSlider->setToolTip("Audio slider");

    // Note: if you use streaming, there is no ability to use the position slider
//    _positionSlider=new QSlider(Qt::Horizontal,this);
//    _positionSlider->setMaximum(POSITION_RESOLUTION);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(_videoWidget);
//    layout->addWidget(_positionSlider);
//    layout->addWidget(_volumeSlider);
    setLayout(layout);

    _isPlaying=false;
    poller=new QTimer(this);

    //Initialize an instance of vlc
    //a structure for the exception is neede for this initalization
    //libvlc_exception_init(&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.

    //create a new libvlc instance
    _vlcinstance=libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);  //tricky calculation of the char space used
    //_vlcinstance=libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args,&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    //raise (&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.

    // Create a media player playing environement
    _mp = libvlc_media_player_new (_vlcinstance);
    //_mp = libvlc_media_player_new (_vlcinstance, &_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    //raise (&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.

    //connect the two sliders to the corresponding slots (uses Qt's signal / slots technology)
    connect(poller, SIGNAL(timeout()), this, SLOT(updateInterface()));
//    connect(_positionSlider, SIGNAL(sliderMoved(int)), this, SLOT(changePosition(int)));
    connect(_volumeSlider, SIGNAL(sliderMoved(int)), this, SLOT(changeVolume(int)));

    poller->start(100); //start timer to trigger every 100 ms the updateInterface slot
//    udprec = new UdpReciever("bla");
//    udprec->start();

}

//desctructor
Player::~Player()
{
    /* Stop playing */
    libvlc_media_player_stop (_mp);
    //libvlc_media_player_stop (_mp, &_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.

    /* Free the media_player */
    libvlc_media_player_release (_mp);

    libvlc_release (_vlcinstance);
    //raise (&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
}

void Player::playFile(QString file)
{
    //the file has to be in one of the following formats /perhaps a little bit outdated)
    /*
    [file://]filename              Plain media file
    http://ip:port/file            HTTP URL
    ftp://ip:port/file             FTP URL
    mms://ip:port/file             MMS URL
    screen://                      Screen capture
    [dvd://][device][@raw_device]  DVD device
    [vcd://][device]               VCD device
    [cdda://][device]              Audio CD device
    udp:[[<source address>]@[<bind address>][:<bind port>]]
    */

    /* Create a new LibVLC media descriptor */
//    _m = libvlc_media_new_path(_vlcinstance, "v4l2:///dev/video0");
    _m = libvlc_media_new_path(_vlcinstance, file.toAscii());
    //_m = libvlc_media_new (_vlcinstance, file.toAscii(), &_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    //raise(&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.

    libvlc_media_player_set_media (_mp, _m);
    //libvlc_media_player_set_media (_mp, _m, &_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    //raise(&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.

    // /!\ Please note /!\
    //
    // passing the widget to the lib shows vlc at which position it should show up
    // vlc automatically resizes the video to the ´given size of the widget
    // and it even resizes it, if the size changes at the playing

    /* Get our media instance to use our window */
//    #if defined(Q_OS_WIN)
//        libvlc_media_player_set_drawable(_mp, reinterpret_cast<unsigned int>(_videoWidget->winId()));
//        //libvlc_media_player_set_drawable(_mp, reinterpret_cast<unsigned int>(_videoWidget->winId()), &_vlcexcep ); // [20101215 JG] Used for versions prior to VLC 1.2.0.
//        //libvlc_media_player_set_hwnd(_mp, _videoWidget->winId(), &_vlcexcep ); // for vlc 1.0
//    #elif defined(Q_OS_MAC)
//        libvlc_media_player_set_drawable(_mp, _videoWidget->winId());
//        //libvlc_media_player_set_drawable(_mp, _videoWidget->winId(), &_vlcexcep ); // [20101215 JG] Used for versions prior to VLC 1.2.0.
//        //libvlc_media_player_set_agl (_mp, _videoWidget->winId(), &_vlcexcep); // for vlc 1.0
//    #else //Linux
//        //[20101201 Ondrej Spilka] obsolete call on libVLC >=1.1.5
//        //libvlc_media_player_set_drawable(_mp, _videoWidget->winId(), &_vlcexcep );
//        //libvlc_media_player_set_xwindow(_mp, _videoWidget->winId(), &_vlcexcep ); // for vlc 1.0

//     /* again note X11 handle on Linux is needed
//        winID() returns X11 handle when QX11EmbedContainer us used */

        int windid = _videoWidget->winId();
        libvlc_media_player_set_xwindow (_mp, windid );

//    #endif
    //raise(&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.

    /* Play */
    libvlc_media_player_play (_mp);


//    libvlc_vlm_add_broadcast(inst, "mybroadcast", "/home/zagaria/Downloads/test4.mp4", "#transcode{vcodec=h264,vb=0,scale=0,acodec=mp4a,ab=128,channels=2,samplerate=44100}:rtp{sdp=rtsp://:5544/}", 0, NULL, TRUE, 0);

    //libvlc_media_player_play (_mp, &_vlcexcep ); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    //raise(&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.

    _isPlaying=true;
}


void Player::broadcastFile(QString file){
    libvlc_vlm_add_broadcast(_vlcinstance, "mybroadcast", file.toAscii(), "#rtp{dst=127.0.0.1,port=5004,mux=ts}", 0, NULL, TRUE, 10000);
//libvlc_vlm_add_broadcast(_vlcinstance, "mybroadcast", "v4l2:///dev/video0", "#transcode{vcodec=h264,vb=0,scale=0,acodec=mp4a,ab=128,channels=2,samplerate=44100}:rtp{dst=127.0.0.1,port=5004,mux=ts}", 0, NULL, TRUE, 0);
    libvlc_vlm_play_media(_vlcinstance, "mybroadcast");

}

void Player::stopPlaying(){
    _isPlaying = false;
    libvlc_media_player_stop(_mp);
    libvlc_vlm_stop_media(_vlcinstance,"mybroadcast");
}

void Player::changeVolume(int newVolume)
{
    //libvlc_exception_clear(&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    libvlc_audio_set_volume (_mp,newVolume);
    //libvlc_audio_set_volume (_vlcinstance,newVolume , &_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    //raise(&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
}

void Player::changePosition(int newPosition)
{
    //libvlc_exception_clear(&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    // It's possible that the vlc doesn't play anything
    // so check before
    libvlc_media_t *curMedia = libvlc_media_player_get_media (_mp);
    //libvlc_media_t *curMedia = libvlc_media_player_get_media (_mp, &_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    //libvlc_exception_clear(&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    if (curMedia == NULL)
        return;

    float pos=(float)(newPosition)/(float)POSITION_RESOLUTION;
    libvlc_media_player_set_position (_mp, pos);
    //libvlc_media_player_set_position (_mp, pos, &_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    //raise(&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
}

void Player::updateInterface()
{
    if(!_isPlaying)
        return;

    // It's possible that the vlc doesn't play anything
    // so check before
    libvlc_media_t *curMedia = libvlc_media_player_get_media (_mp);
    //libvlc_media_t *curMedia = libvlc_media_player_get_media (_mp, &_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    //libvlc_exception_clear(&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    if (curMedia == NULL)
        return;

    float pos=libvlc_media_player_get_position (_mp);
    //float pos=libvlc_media_player_get_position (_mp, &_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
//    int siderPos=(int)(pos*(float)(POSITION_RESOLUTION));
//    _positionSlider->setValue(siderPos);
    int volume=libvlc_audio_get_volume (_mp);
    //int volume=libvlc_audio_get_volume (_vlcinstance,&_vlcexcep); // [20101215 JG] Used for versions prior to VLC 1.2.0.
    _volumeSlider->setValue(volume);
}
/*void Player::raise(libvlc_exception_t * ex)
{
    if (libvlc_exception_raised (ex))
    {
         fprintf (stderr, "error: %s\n", libvlc_exception_get_message(ex));
         exit (-1);
    }
}*/  // [20101215 JG] Used for versions prior to VLC 1.2.0.



#include "core.h"
#include "VideoIn.h"
#include "codec/VideoCoDecLib.h"
#include "avmux_def.h"
//#include "control.h"
#include <iostream>



extern "C" {
    #include "rtp/davc_rtp.h"
}

static bool file=false;


void* ClientThread(void* args){

    Core* core=(Core*)args;
    while(core->running){
        core->Receive();
        bool decode=core->RTP_Unpack();
        if(decode){
            core->DecodeFrame();
        }
    }

    return NULL;
}

void* ServerThread(void* args){
    Core* core=(Core*)args;
    while(core->running){
        if(file){

            pthread_mutex_lock(&sendq_mutex);
            std::cout << sendq.size() << "Senq Size server Thread" << std::endl;
            if(sendq.size()>0){
                int w_=640;
                int h_=480;
                core->out[RAW].SetData(sendq.front(),w_*h_*2);
            }else{
                pthread_mutex_unlock(&sendq_mutex);
                //! SLEEP 100ms

                continue;

            }
            pthread_mutex_unlock(&sendq_mutex);
        }else{

            if (core->GetVideoDevice()){
                if(!VideoInGet(core->GetVideoDevice(), core->out[RAW].GetAddrOfData() ))
                {
                    core->Error("VideoInGet failed"); //!ob das wirklich ein error werfen muss? eig. reicht hier ein cout und dann ein continue
                }
            }
        }

        core->EncodeFrame();
        core->RTP_Pack();
        core->Send();

    }
    return NULL;

}

Core::Core(GUI* gui,bool server):control(true)
{
    mem = new unsigned char[5000];

    this->gui=gui;
    network=new Network(false);
    this->server=server;



    this->eps_value = 25;
    w=WIDTH;
    h=HEIGHT;
    vd = NULL;

    //Control Thread starten:
//    int rc = pthread_create(&thread_id[2], NULL, ControlThread, (void*)this);
//    if (rc){
//        //printf("ERROR; return code from pthread_create() is %d\n", rc);
//        Error("Creating Thread failed");
//        exit(-1);
//    }
}
Core::~Core(){
    running=false;
    control=false;

    VideoInExit(vd);

    sleep(1); //sek warten wegen threads
    delete mem;
}


void Core::setEPSValue(int eps){
    this->eps_value= eps;
}

void Core::InitCodec(){
    //codec = InitEncode(breite,hoehe,1,1,4,16,5,9,DAVC_YUV_420,0,0,1,1,15*VideoInGetFPS(videoin)/temporal_levels,temporal_levels,num_frame_insert,-1,12,60/temporal_levels);

    int breite=w;
    int hoehe=h;
    temporal_levels=3;
    int num_frame_insert[2] = {1,2};
    int qp_offsets[3] = {0,0,0};

    int fps=VideoInGetFPS(vd);
    //! im moment ist das noch der alte codec imo!!
    SetRate(30);

    enc = InitEncode(breite,hoehe,1,1,4,16,5,9,DAVC_YUV_420,0,0,1,1,15*fps/temporal_levels,temporal_levels,num_frame_insert,-1,12,60/temporal_levels);
    if (enc==0){
        Error("Encode init failed");
        exit(1);
    }
    SetRate(30);
    SetTemporalLevelQpOffsets( enc, qp_offsets );

    //Decoder:
    dec= InitDecode(0,0);
    if (dec==0){
        Error("Decode init failed");
        exit(1);
    }


}

void Core::SetRate(int kbps){
    target_kbps=kbps;
    avr_interlen = ((target_kbps*1000.0f)/8.0f) / VideoInGetFPS(vd);
    if (avr_interlen > 960) slicelen = 960;
    else slicelen = avr_interlen;

    EncoInitQparam(avr_interlen,w,h);

}


unsigned char* Core::GetFrame(){
    unsigned char* mem=NULL;
    if(!server){
        return in[RAW].GetData();
    }else{
        return out[RAW].GetData();
    }
    return mem;
}


void Core::Error(string error){
    if(gui){
        gui->ShowError(error);
    }else{
        cout<<"Error: "<<error<<endl;
    }
}
void Core::AddClient(string ip){
    clients.push_back(new Client(ip,network));
}

void Core::RemoveClient(string ip){
    //! untested

    std::vector<Client*>::iterator it;
    std::vector<Client*>::iterator it_to_remove;
    for(it=clients.begin();it!=clients.end();it++){
        if ((*it)->GetIP()==ip){
            it_to_remove=it;
        }
    }
    delete (*it_to_remove);
    clients.erase(it_to_remove);

}

void Core::ClearClients(){
    std::vector<Client*>::iterator it;
    for(it=clients.begin();it!=clients.end();it++){
        delete (*it);
    }
    clients.clear();
}

void Core::Send(){
    std::vector<Client*>::iterator it;
    while(network_out.size()>0){
        for(it=clients.begin();it!=clients.end();it++){
            (*it)->Send(*network_out.front());
        }
        network_out.pop();
    }
}

void Core::Receive(){

    shared_ptr<Data> data_ptr(new Data());
    int len;
    network->Receive(*data_ptr);
    network_in.push(data_ptr);
}

void Core::RTP_Pack(){
    //! HACK statt RTP:
    //network_out.push(shared_ptr<Data>(new Data(out[CODEC])));
    //return;





    unsigned long time = timeGetTime();

    uint8_t *p_packets = NULL;
    uint32_t *p_packets_len_array = NULL;
    unsigned char p=0;

    p_rtp_pack = NULL;
    if ( p_rtp_pack == NULL ) p_rtp_pack = H264RTP_Open( 1350, 1, 0 );
    H264RTP_PacketizeAU( p_rtp_pack, out[CODEC].GetData(), out[CODEC].GetSize(), &p_packets, &p_packets_len_array );

    if( p_packets && p_packets_len_array )
    {
            int extracted_temporal_level = H264RTP_GetTemporalLevel( p_packets, temporal_levels );

            int written = 0, to_write = 0, write = 0, slice=0;

            for ( written=0, slice=0; p_packets_len_array[slice]!=0; slice++ )
            {
                    to_write = p_packets_len_array[slice];
                    ExtraDataType *extra;    // extra data
                    extra = (ExtraDataType*) mem;

                    int seq = 0;

                    if( extracted_temporal_level == 1 )
                    {
                            extra->id = NEW_LAYER1_VIDEO;
                    }
                    else if( extracted_temporal_level == 2 )
                    {
                            extra->id = NEW_LAYER2_VIDEO;
                    }
                    else
                    {
                            extra->id = NEW_SIP_VIDEO;
                            seq = p++;
                    }

                    //! if (my_num!=0) extra->id = NEW_VIDEO_FRAME;
                    //! :
                    //extra->id=NEW_VIDEO_FRAME;

                    //! extra->data[0] = crc_of_num(0);
                    //! :
                    extra->data[0]=0;

                    extra->data[1] = to_write + ((seq&0xf)<<12) + ((p_packets_len_array[slice+1] == 0)<<16) + ((time&0x3fff)<<17) +((intra&0x1)<<31);

                    memcpy(mem + sizeof(ExtraDataType),p_packets+written,to_write);

                    //!send_data(mem, sizeof(ExtraDataType) + to_write, 2, TRUE);
                    try{
                        //slicecounter++;
                        //network_out.push(shared_ptr<Data> (new Data(mem,sizeof(ExtraDataType)+to_write)));
                        shared_ptr<Data> tmp_p(new Data(mem,sizeof(ExtraDataType)+to_write));
                        network_out.push(tmp_p);
                    }catch(bad_alloc& ba){
                        std::cout<<"bad_alloc: "<< ba.what()<<" to_write: "<<to_write <<std::endl;
                        Error("Bad Alloc");
                    }
                    written+=to_write;
                    write+=to_write;
                    to_write = 0;

                    if ((write>avr_interlen )&&(p_packets_len_array[slice+1]>slicelen/4)&&(target_kbps<=1280.0f) )
                    {
                            //write = 0;
                            //! if (videoin) VideoInGet(videoin,&yuv);//false);
                    }
            }
            //! SendRatesVideo(len+(slice)*24);   // Statistik
    }

}

bool Core::RTP_Unpack(){
    //! HACK:
    //in[CODEC]=*network_in.front();
    //network_in.pop();
    //return true;



    shared_ptr <Data>  network_data=(network_in.front());
    network_in.pop();

        ExtraDataType *extra;    // extra data
        extra = (ExtraDataType*) network_data->GetData();
    {
            //! DAS MUSS UNBEDINGT IN EINE ZWISCHENSTUFE

            //! Je nachdem von wem es kommt muss es getrennt dekodiert werden.... irgendwann mal :P

            //num = num_of_crc(extra->data[0]);

        switch( extra->id )
        {
            case NEW_SIP_VIDEO:
            case NEW_LAYER1_VIDEO:
            case NEW_LAYER2_VIDEO:
                break;
            case NEW_VIDEO_FRAME:
                break;
            default:
                Error("Unknown packet");
                return false;

        }
    }
//    decoder[0][num]->PlaySipFrame(buffer, len - sizeof(ExtraDataType), (extra->data[1]>>16)&0x1, extra->data[1]>>17, (extra->data[1]>>12)&0xf);
    unsigned char* data=network_data->GetData()+ sizeof(ExtraDataType);
    int len= network_data->GetSize()-sizeof(ExtraDataType);
    bool marker= (extra->data[1]>>16)&0x1;
    unsigned short timestamp=extra->data[1]>>17;
    unsigned char aktnum=(extra->data[1]>>12)&0xf;


//bool DecoderSession::PlayFrame(unsigned char *data, int len, bool marker, unsigned short timestamp, unsigned char aktnum)

    /*
    if (H264RTP_GetTemporalLevel(data, 3) == 0)
    {
            paketnum++;
            //packet loss?
            if ((aktnum&0xf)!=(paketnum&0xf))
            {
                Error("Packet lost");
                //! wenn packetnummer und aktnum nicht gleich sind ist ein packet verloren gegangen!

                    if ((data[0]&0x1f) != 7 ) //nicht wenn Frame der Start eines Intras ist
                    {
                            ExtraDataType extra;
                            extra.id = NEW_SIP_INTRA;
                            extra.data[0] = crc_of_num(0);
                            extra.data[1] = crc_of_num(my_num);
                            send_data_num((unsigned char*)&extra, sizeof(ExtraDataType),my_num,4,false);
                    }
                    paketnum = aktnum;

            }
    }

*/
    if (rtp_unpack_time==0)
    {
            rtp_unpack_time=timeGetTime();
            oldtimestamp = timestamp;
    }
    if (timestamp+8192 > oldtimestamp) rtp_unpack_time += timestamp - oldtimestamp;
    else rtp_unpack_time += (timestamp+16384) - oldtimestamp;
    //fprintf(stderr,"num: %i time:%i %i %i  timeGetTime:%i\n",my_num,time,timestamp-oldtimestamp,timeGetTime()-time,timeGetTime());
    oldtimestamp = timestamp;

    if ( p_rtp_depack == NULL ) p_rtp_depack = H264RTP_Open( 0, 0, 0 );

    uint8_t *p_depack_au = NULL;
    uint32_t i_depack_au_len = 0;




    if( (H264RTP_Depacketize( p_rtp_depack, data, len, 0, marker, &p_depack_au, &i_depack_au_len)==0) && (i_depack_au_len>0))
    {

        in[CODEC].SetData(p_depack_au,i_depack_au_len);

        int count = 0;
        //!	if (SoundOut[my_num]) count = SoundOut[my_num]->GetCount();
        //! decode[rtp_unpack_in]=memory;
        //! length[rtp_unpack_in]=i_depack_au_len;

        //! delay ist mir erst mal egal...
        //! delay[rtp_unpack_in]=timeGetTime()+count;


        //! if (delay[rtp_unpack_in]<0) delay[rtp_unpack_in]=0;
        rtp_unpack_in++;

        if (rtp_unpack_in==32) rtp_unpack_in=0;

        return true;

    }else{
        //Ist kein echter Fehler, da er einfach wartet bis er alle Packets zusammen hat
        //std::cout<<"Depack failed... maybe ok?"<<std::endl;
        return false;
    }
}



void Core::Server(bool server){
    this->server=server;



    //Videokamera initialisieren
    if(server){
        if(network) delete network;
        network=new Network(true);
        if(vd ==NULL){
            vd=VideoInInit(w,h, 30, 0,"");
        }
        if(vd==NULL){
            Error("VideoIn failed");
            exit(1);
        }
        videosource=RAW;
//        videosource=CODEC;
    }else{
        if(network) delete network;
        network=new Network(false);
        if(vd !=NULL){
            VideoInExit(vd);
            vd = NULL;
        }
        videosource=NETWORK;
    }

    InitCodec();


    //Threads starten:
    running=true;
    if(!server){
        int rc = pthread_create(&thread_id[0], NULL, ClientThread, (void*)this);
        if (rc){
            //printf("ERROR; return code from pthread_create() is %d\n", rc);
            Error("Creating Thread failed");
            exit(-1);
        }
    }else{
        int rc = pthread_create(&thread_id[1], NULL, ServerThread, (void*)this);
        if (rc){
            //printf("ERROR; return code from pthread_create() is %d\n", rc);
            Error("Creating Thread failed");
            exit(-1);
        }
    }


}

void Core::EncodeFrame(){
    static int counter=-1;
    counter++;


    //int imode=1;  //imode:    (in) if 1 a full frame will be encoded (INTRA I);
    int imode=0;
    std::cout <<"counter: "<<counter<<std::endl;
    if(counter==INTER){
        imode=1;
        counter=0;
    }



    //return;
    int *slices;
    int len,intra;
    unsigned char *yuv_reco;

    int err=Encode( enc, out[RAW].GetData(), this->eps_value, imode, &intra, 2, slicelen, 0, 0, out[CODEC].GetData(), &slices, &len, &yuv_reco);
    if(err<0){
        //QMessageBox::about(window, "Error","Encode failed");
        std::cout<<"Encode failed with error code: "<<err<<std::endl;
    }

//    eps_value = EncoGetNewQparam( eps_value,len,intra );
//    eps_value =25;

    out[CODEC].SetSize(len);



}

unsigned char* Core::DecodeFrame(){
    //std::cout<<"slices: "<<slicecounter<<std::endl;
    int breite = 0;
    int hoehe = 0;
    int i,f;
    //-----------
    unsigned char* pInData;
    int len;
    if(server){
        pInData=out[CODEC].GetData(&len);
    }else{
        pInData=in[CODEC].GetData(&len);

        if(len==0 || pInData==NULL){
            std::cout<<"No Data"<<std::endl;
            return NULL;
        }
    }
    unsigned char* yuv;
    //-----------
    int ok = Decode(dec, pInData, len, &yuv, &breite, &hoehe, &f, &i);
    if ((ok!=ERR_NALU)&&(ok&EOP)&&(breite!=0)&&(hoehe!=0)){
        in[RAW].SetData(yuv,breite*hoehe*3/2);
        //std::cout<<"Decode success"<<std::endl;
    }else{
        yuv=NULL;
        //QMessageBox::about(window, "Error","Decode failed");
        std::cout<<"Decode failed"<<std::endl;
    }

    return yuv;
}











//}

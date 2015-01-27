#include "data.h"
#include <cstring>

Data::Data()
{
    data=NULL;
    Init(DATASIZE);
}

Data::Data(const Data& d){
    Init(DATASIZE);
    unsigned int tmp_len;
    unsigned char* tmp_data=d.GetData(&tmp_len);
    SetData(tmp_data,tmp_len);
}

Data::Data(unsigned char* data, unsigned int len){
    Init(len);
    SetData(data,len);
}

Data::~Data(){
    delete data;
    data=NULL;
}

Data::Data(unsigned int size){
    Init(size);

}
void Data::Init(unsigned int size){
    data = new unsigned char[size];
    this->size=size;
}

void Data::SetData(unsigned char* data,unsigned int len){
    if(len<DATASIZE){
        memcpy(this->data,data,len);
        this->size=len;
    }else{
        std::cout<<"Error in SetData: not enough mem" <<std::endl;
    }
}

unsigned char* Data::GetData(unsigned int* len)const{
    if(len!=NULL){
        *len=GetSize();
    }
    return data;
}

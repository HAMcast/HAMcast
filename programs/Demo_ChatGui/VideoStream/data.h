#ifndef DATA_H
#define DATA_H

//#define DATASIZE 15000000000
#define DATASIZE 150000000  //150k
//#define DATASIZE 15  //15
#include <iostream>

class Data
{
public:
    Data();
    ~Data();
    Data(unsigned int size);
    Data(unsigned char* data, unsigned int len);
    Data(const Data& d);
    Data & operator=(const Data& d){
        if(this!=&d){
            unsigned int tmp_len;
            unsigned char* tmp_data=d.GetData(&tmp_len);
            SetData(tmp_data,tmp_len);
        }
        return *this;
    }
    void Init(unsigned int size);
    void SetData(unsigned char* data,unsigned int len);

    unsigned char* GetData(int* len)const{
        return GetData((unsigned int*)len);
    }
    unsigned char* GetData(unsigned int* len)const;

    unsigned int GetSize()const {return size;}
    void SetSize(int size){this->size=size;}

    unsigned char* GetData()const{return data;}

    unsigned char** GetAddrOfData(){
        std::cout <<"Eeeeeeeak! vermutlich wird hier der pointer neu gesetzt! MEMORYLEAK!!!"<<std::endl;
        return &data;
    }
private:
    unsigned int size;
    unsigned char* data;
};

#endif // DATA_H

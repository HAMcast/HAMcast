//#include "control.h"
#include <iostream>

using namespace std;

//void* ControlThread(void* args){
//    Core* core=(Core*) args;
//    Network network(false,CONTROLPORT); //! hier muss ein anderer port genommen werden
//    Data data;
//    while(core->control){
//        try{
//            //cout<<"control running"<<endl;
//            network.Receive(data);

//            vector<string> command;
//            SplitString((const char*)data.GetData(),command);
//            vector<string>& c=command;
//            vector<string>::iterator it;

//            if(Cmp(c,"server")){

//            }else if(Cmp(c,"quit")){

//            }
//        }catch(CoreException& e){
//            e.PrintError();
//        }
//    }
//    return NULL;
//}



//void SplitString(string src,vector<string>& dst){
//    string tmp="";

//    for(int i=0; i<src.size();i++){
//        if(src[i]==' '){
//            dst.push_back(tmp);
//            tmp="";
//        }else{
//            tmp+=src[i];
//        }
//    }
//    dst.push_back(tmp);
//}

//bool Cmp(vector<string> cmd, string str){
//    if(cmd.size()<=0)
//        return false;
//    return(cmd.front()==str);
//}

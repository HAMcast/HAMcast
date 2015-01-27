#include "Instance.h"

Instance::Instance(AribaModule* ariba, string nodename,string spovnet, ServiceID sID, size_t blen,  size_t tout, size_t ival):
    ariba(ariba),
    node(NULL),
    name(nodename),
    spovnet(spovnet),
    tout(tout),
    ival(ival),
    buf(reinterpret_cast<char*>(malloc(blen))),
    runtime_total_loss(0),
    p_all(0),
    p_ival(0),
    pktno_recv(0),
    cur_time(),
    to_end(),
    temp(),
    started(false),
    logging(false),
    sid(sID),
    last_lost_packages(0),
    join_completed(false),
    first_package(0)
{
    std::cout << "constructor instance.cpp \n";
}

Instance::~Instance()
{
}

void Instance::onJoinCompleted(const SpoVNetID &vid){


    mcpo = new MCPO( this, ariba, node );
    ServiceID group(1234);
    mcpo->joinGroup(group);
    join_completed=true;

}

void Instance::onJoinFailed(const SpoVNetID &vid)
{
}

void Instance::onLeaveCompleted(const SpoVNetID &vid)
{
}

void Instance::onLeaveFailed(const SpoVNetID &vid)
{
}

void Instance::startup()
{



    node=new ariba::Node(*ariba,name);
    std::cout << "created node" << std::endl;
    nodeID=new NodeID(node->getNodeId());

    bool binded1 = node->bind(this);
    std::cout << "binded mcpotester1" << std::endl;
    node->start();
    std::cout << "started node" << std::endl;

    ariba::Name spovnetName(spovnet);
    ariba::SpoVNetProperties params;
    node->initiate(spovnetName,params);
    std::cout << "initiated spovnet" << std::endl;
    node->join(spovnetName);

    std::cout << "joined spovnet"<<std::endl;

}


void Instance::shutdown()
{
}

void Instance::eventFunction()
{
}

void Instance::receiveData(const DataMessage &msg)
{
    std::cout << "falsches receive " << std::endl;

}

void Instance::receiveData(const DataMessage &msg, ServiceID sid)
{
uint32_t tmp;
uint32_t  pktno;
    if(!started){
        temp=microsec_clock::local_time();
        to_end=microsec_clock::local_time();
        temp=temp+boost::posix_time::seconds(ival);
        to_end=to_end+boost::posix_time::seconds(tout);


        this->started=true;
        void* str= msg.getData();
        uint32_t msgsize=(msg.getSize())/8;
        memcpy(buf, str , msgsize);
        memcpy(&tmp, buf, sizeof(tmp));
       pktno = ntohl(tmp);
        first_package=tmp;
    }  else{

    void* str= msg.getData();
    uint32_t msgsize=(msg.getSize())/8;
    memcpy(buf, str , msgsize);
    memcpy(&tmp, buf, sizeof(tmp));
    pktno = ntohl(tmp);
    }


    if(!started){
        p_all=pktno;
        p_ival=pktno;
        last_lost_packages=pktno;
    }
    //pktno_recv=letzte empfangene paketnummer
    //pktno=diese empfangene Paketnummer

    if (pktno == (pktno_recv+1)) {
             pktno_recv = pktno;
             p_all++;
             p_ival++;
             //    std::cout << "paketno: " << pktno << std::endl;
         }

         else if (pktno > (pktno_recv+1)) {

                    runtime_total_loss+=(pktno-pktno_recv-1);


                    pktno_recv = pktno;

                    p_all++;
                    p_ival++;

         }
         else if (pktno <= pktno_recv) {
    
         }

         this->cur_time = microsec_clock::local_time();


         if(this->cur_time >= this->temp){
          //   uint32_t time=temp.seconds();
             time_duration dur=cur_time-temp;
             double packed_in_ival=(p_ival*1000)/((boost::posix_time::seconds(ival)+dur).total_milliseconds());
             packages_per_ival.push_back(packed_in_ival);

             uint32_t lost_in_ival=runtime_total_loss-last_lost_packages;
              lost_per_ival.push_back(lost_in_ival);
              last_lost_packages=runtime_total_loss;

              long timestamp=std::time(0);
             times.push_back(timestamp);

             p_ival=0;
             temp=microsec_clock::local_time();
             temp+=boost::posix_time::seconds(ival);

         }

         if(cur_time >= to_end ){




             std::cout << "#total package count: " << p_all << std::endl;
             std::cout << "#ival\t\ttime\t\tpackages\t\tpacketloss"<< std::endl;
             double sum_up_lost_packages=0;
              for(int i=0;i<packages_per_ival.size()-1;i++){

                  std::cout << i <<"\t\t"<<times.at(i) ;
                  std::cout <<"\t"<< packages_per_ival.at(i);
                  std::cout << "\t\t"<<lost_per_ival[i] << std::endl;

                  sum_up_lost_packages+=lost_per_ival[i];
              }
                std::cout << "#first received package: " << first_package << std::endl;
              std::cout << "#packet loss counted at time: " << runtime_total_loss << "packet loss summed up: "<< sum_up_lost_packages <<   " last sendet package: " << pktno_recv<<  std::endl;
             exit(EXIT_SUCCESS);

         }
    }


void Instance::serviceIsReady()
{

}

void Instance::setCluster(unsigned int layer, NodeID leader, NodeID rp, std::vector<NodeID> members)
{
}

void Instance::setRemoteCluster(unsigned int layer, NodeID leader, std::vector<NodeID> members)
{
}

void Instance::setK(unsigned int k)
{
}

void Instance::sendToGroup(const void *message, uint32_t length, uint32_t group)
{
    DataMessage msg(message, length);
    ServiceID gr(group);

   /* void* buffer=msg.getData();
    for(int i=0;i<length; i++){
      std::cout << (int) (((char*)buffer)[i]) << " ";
    }
    std::cout << std::endl;*/

    mcpo->sendToGroup(*msg,gr);
}

uint32_t Instance::getSendetPackagesFromAriba()
{
    return node->getSendetPackageCount();
}



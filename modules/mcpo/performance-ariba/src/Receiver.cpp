#include "Receiver.h"


using ariba::utility::StartupInterface;
using namespace ariba;



// logging
//use_logging_cpp( Receiver );

// construction
Receiver::Receiver(uint32_t ival, uint32_t tout, uint32_t blen, std::string role, std::string endpoints, std::string bootstap, std::string nodename, std::string spovnet, uint32_t payloadsize)
    :node(NULL),
      nodeID(NULL),
      ival(ival),
      tout(tout),
      buf(reinterpret_cast<char*>(malloc(blen))),
      p_all(0),
      p_ival(0),
      pktno_recv(0),
      runtime_total_loss(0),
      cur_time(),
      to_end(),
      temp(),
      started(false),
      role(role),
      endpoints(endpoints),
      bootstrap(bootstap),
      nodename(nodename),
      spovnet(spovnet),
      payloadsize(payloadsize),
      logging(false),
      sid(222),
      last_lost_packages(0),
      join_completed(false),
      packet_count(0)
{
    Timer::setInterval( 1 );
}

// destruction
Receiver::~Receiver() {
}




// implementation of the startup interface
void Receiver::startup() {
        aribamodule= new ariba::AribaModule();

        aribamodule->setProperty("endpoints", endpoints);
        aribamodule->setProperty("bootstrap.hints", bootstrap);
        aribamodule->start();
        node=new ariba::Node(*aribamodule,nodename);
        nodeID=new NodeID(node->getNodeId());
        node->start();
        bool binded1 = node->bind(this);
        std::cout << "Node listener was binded: "<< binded1 << std::endl;
        bool binded2 = node->bind(this,sid);
        std::cout << "Communication interface was binded: "<< binded2<< std::endl;
        ariba::SpoVNetProperties params;
        node->initiate(spovnet,params);
        node->join(spovnet);
        if(role.compare("sender") == 0 ){
            log("i am sender!");

        }



}

// implementation of the startup interface
void Receiver::shutdown() {
    aribamodule->stop();


    // leave spovnet
    node->leave();

    // unbind node listener
    node->unbind( this );


    // stop the ariba module


    // delete node and ariba module
    delete node;
    delete aribamodule;

    // now we are completely shut down

}



/*
 * NodeListener interface
 */
void Receiver::onJoinCompleted( const SpoVNetID& vid ) {


join_completed=true;
//if (role =="sender") Timer::start();


}

void Receiver::onJoinFailed( const SpoVNetID& vid ) {

}

void Receiver::onLeaveCompleted( const SpoVNetID& vid ){

}

void Receiver::onLeaveFailed( const SpoVNetID& vid ){

}

void Receiver::onMessage(const DataMessage &msg, const NodeID &remote,const LinkID& lnk )
{

     int recv_size = 0;


            if(!started){
                this->temp=microsec_clock::local_time();
                this->to_end=microsec_clock::local_time();
                this->temp=this->temp+boost::posix_time::seconds(ival);
                this->to_end=this->to_end+boost::posix_time::seconds(tout);



                this->started=true;

             }


            uint32_t msgsize=msg.getMessage()->getPayload().getLength()/8;
         //   std::cout << "size: "<<msgsize << std::endl;

           // std::cout << "data: "<< msg.getMessage()->getPayload().getBuffer() << std::endl;
           // memcpy(buf, msg.getMessage()->getPayload().getBuffer(), msgsize);
            PingPongMessage* pingmsg = msg.getMessage()->convert<PingPongMessage> ();
            recv_size = msgsize;
             uint32_t tmp;
                  //memcpy(&tmp, buf, sizeof(tmp));
                   tmp=pingmsg->getNum();
                //   uint32_t pktno = ntohl(tmp);
                    uint32_t pktno = tmp;
            //        std::cout << "received nr: " << pingmsg->getNum() << std::endl;

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

              std::cout << "#packet loss counted at time: " << runtime_total_loss << "packet loss summed up: "<< sum_up_lost_packages <<" last sendet package: " << pktno_recv<<  std::endl;
              stop_all();

         }
    }




void Receiver::setCluster(unsigned int layer, NodeID leader, NodeID rp, std::vector<NodeID> members) {

}

void Receiver::setRemoteCluster(unsigned int layer, NodeID leader, std::vector<NodeID> members) {

}



void inline Receiver::log(const char *str){
    if(logging) std::cout << str << std::endl;
}

void Receiver::eventFunction()
{
  /*  packet_count++;
    //std::cout<< "sending: " << packet_count << std::endl;
    PingPongMessage  m=PingPongMessage(0,packet_count);
    std::vector<NodeID> nodes = node->getNeighborNodes();

    BOOST_FOREACH( NodeID nid, nodes ){
    node->sendMessage(m,nid,sid);
    }
 //   std::cout << "EVENT!" << std::endl;

    // if(role.compare("sender")==0)send();
*/

}

void Receiver::send(char* buf, uint32_t payloadsize){

    DataMessage m=DataMessage(buf,payloadsize);

    string str=buf;

    std::vector<NodeID> nodes = node->getNeighborNodes();
   // std::cout << "into send" << std::endl;
    BOOST_FOREACH( NodeID nid, nodes ){
    node->sendMessage(m,nid,sid);
    }
   // delete m;
}

void Receiver::send_with_ping(uint32_t nr, char* buf, uint32_t payloadsize){

   // DataMessage* m=new DataMessage(buf,payloadsize);

    string str=buf;

    //PingPongMessage*  m=new PingPongMessage(0,nr,str);
    PingPongMessage  m=PingPongMessage(0,nr,str);
    std::vector<NodeID> nodes = node->getNeighborNodes();

    BOOST_FOREACH( NodeID nid, nodes ){
    node->sendMessage(m,nid,sid);
    }
    //delete m;
}


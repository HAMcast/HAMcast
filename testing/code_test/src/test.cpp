#define BOOST_TEST_MODULE hamcast_test
#define BOOST_TEST_DYN_LINK
#include<boost/test/unit_test.hpp>
#include "hamcast/hamcast.hpp"
#include <boost/shared_ptr.hpp>
#include <iostream>
#include<stdlib.h>
#include<string>
#include <exception>

using namespace std;
using namespace hamcast;
using namespace boost;



const string protocol = "ip";

int is_ip_mcast_group_subscribed(string group)
{
    string cmd = "/sbin/ip maddress show|grep -q ";
    cmd.append(group);
    return (system(cmd.c_str()));
}



BOOST_AUTO_TEST_CASE(simple_join_leave)
{
        uri u(protocol+"://239.0.0.1:1234");
        multicast_socket m;
        m.join(u);
        if(protocol == "ip")
            BOOST_CHECK(is_ip_mcast_group_subscribed("239.0.0.1") == 0);
        m.leave(u);
        if(protocol == "ip")
            BOOST_CHECK(is_ip_mcast_group_subscribed("239.0.0.1") != 0);
}

BOOST_AUTO_TEST_CASE(ssm_join_leave)
{
    multicast_socket m;
    m.join(protocol+"://192.168.0.1@239.5.0.1:1234");
    BOOST_CHECK(is_ip_mcast_group_subscribed("239.5.0.1") == 0);
    m.leave(protocol+"://192.168.0.1@239.5.0.1:1234");
    BOOST_CHECK(is_ip_mcast_group_subscribed("239.5.0.1") != 0);
}

BOOST_AUTO_TEST_CASE(socket_create_close)
{
    vector<shared_ptr<multicast_socket> > list;
    list.reserve(50000);
    BOOST_TEST_MESSAGE("creating 50000 multicast_sockets");
    for(int i=0;i<50000;i++)
    {
        shared_ptr<multicast_socket> m(new multicast_socket());
        list.push_back(m);

    }

    BOOST_TEST_MESSAGE("closing 50000 multicast_sockets + call destructor");
    for (vector<shared_ptr<multicast_socket> >::iterator it = list.begin(); it!=list.end(); ++it)
    {
        BOOST_CHECK_NO_THROW((*it)->close());
        list.erase(it);
    }
}

BOOST_AUTO_TEST_CASE(socket_join_close)
{
    BOOST_TEST_MESSAGE("joining 3 groups, closing socket, check if groups are unsubscribed");
        multicast_socket m;
        m.join(protocol+"://239.0.0.1:1234");
        m.join(protocol+"://239.0.0.2:1134");
        m.join(protocol+"://239.0.0.3:1239");
        if(protocol == "ip")
        {
            BOOST_CHECK(is_ip_mcast_group_subscribed("239.0.0.1") == 0);
            BOOST_CHECK(is_ip_mcast_group_subscribed("239.0.0.2") == 0);
            BOOST_CHECK(is_ip_mcast_group_subscribed("239.0.0.3") == 0);
        }
        BOOST_CHECK_NO_THROW(m.close());
        if(protocol == "ip")
        {
            BOOST_CHECK(is_ip_mcast_group_subscribed("239.0.0.1") != 0);
            BOOST_CHECK(is_ip_mcast_group_subscribed("239.0.0.2") != 0);
            BOOST_CHECK(is_ip_mcast_group_subscribed("239.0.0.3") != 0);
        }

}


BOOST_AUTO_TEST_CASE(mass_join_one_socket_one_group)
{
        const int cnt=5000;
        multicast_socket m;
        uri u(protocol+"://239.0.0.1:1234");

        m.join(u);
        BOOST_TEST_MESSAGE("joining " + lexical_cast<string>(cnt)+" times "+ u.str()+ " on same socket");
        for(int i=0;i<cnt;i++)
        {
            BOOST_CHECK_NO_THROW(m.join(u));
        }
}


BOOST_AUTO_TEST_CASE(mass_join_one_socket_different_groups)
{
        multicast_socket m;
        const string base("239.0.");

        //every join opens an new native socket, os depended max socket limit exists
        for(int i=0;i<13;i++)
        {
            for(int j=1;j<10;j++)
            {
                for(int port=1500;port<1505;port++)
                {
                    string group(base+lexical_cast<string>(i)+"."+lexical_cast<string>(j));
                    uri u(protocol+"://"+group + ":"+lexical_cast<string>(port));
                    BOOST_TEST_MESSAGE("joining " + u.str());
                    m.join(u);
                    if(protocol == "ip")
                        BOOST_CHECK(is_ip_mcast_group_subscribed(group) == 0);
                }

            }
        }

        for(int i=0;i<13;i++)
        {
            for(int j=1;j<10;j++)
            {
                for(int port=1500;port<1505;port++)
                {
                    string group(base+lexical_cast<string>(i)+"."+lexical_cast<string>(j));
                    uri u(protocol+"://"+group + ":"+lexical_cast<string>(port));
                    BOOST_TEST_MESSAGE("leaving " + u.str());
                    m.leave(u);
                    if(protocol == "ip")
                        BOOST_CHECK(is_ip_mcast_group_subscribed(group) != 0);
                }
            }

        }
}

BOOST_AUTO_TEST_CASE(mass_join_different_sockets_different_groups)
{
        const string base(protocol+"://239.0.");

        //every join opens an new native socket, os depended max socket limit exists
        vector<shared_ptr<multicast_socket> > list;
        for(int i=0;i<13;i++)
        {
            shared_ptr<multicast_socket> m(new multicast_socket());
            list.push_back(m);

            for(int j=1;j<10;j++)
            {
                for(int k=1500;k<1505;k++)
                {
                    string group(base+lexical_cast<string>(i)+"."+lexical_cast<string>(j));
                    uri u(group + ":"+lexical_cast<string>(k));
                    BOOST_TEST_MESSAGE("joining " + u.str());
                    m->join(u);
                    if(protocol == "ip")
                        BOOST_CHECK(is_ip_mcast_group_subscribed(group) == 0);
                }
            }
        }
}


BOOST_AUTO_TEST_CASE(send_empty_packets)
{
    for(int i=0;i<50000;i++)
    {
        multicast_socket m;
        BOOST_CHECK_NO_THROW(m.send("ip://239.0.0.1:8592", 0, ""));
    }
}
BOOST_AUTO_TEST_CASE(send_packets)
{
    BOOST_TEST_MESSAGE("sending packets, size 1 - 1000000 bytes, step: 500 bytes");
    for(int i=1;i<1000000;i+=500)
    {

        multicast_socket m;
        char buf[i];
        BOOST_TEST_MESSAGE("Sending Packet with size: "<<i);
        BOOST_CHECK_NO_THROW(m.send("ip://239.0.0.1:7592", sizeof(buf), buf));
    }
}


/* undefined reference get_interfaces
BOOST_AUTO_TEST_CASE(get_interfaces)
{
        multicast_socket m;
        BOOST_CHECK_NO_THROW(s.get_interfaces());
        vector<hamcast::interface_id> v = s.get_interfaces();
}
*/

BOOST_AUTO_TEST_CASE(set_ttl)
{
    BOOST_TEST_MESSAGE("sending packets with TTL values from 0-20000");
    char buf[] = "TTL TEST";
    for(int i=-100;i<20000;i++)
    {
        multicast_socket m;

        BOOST_TEST_MESSAGE("Setting TTL to: "<<i<<" & sending 1 packet");
        if(i >0 && i< 255)
        {
            BOOST_CHECK_NO_THROW(m.set_ttl(i));
        }
        else
        {
            BOOST_CHECK_THROW(m.set_ttl(i), std::exception);
        }

        BOOST_CHECK_NO_THROW(m.send("ip://239.0.0.1.1234", sizeof(buf), buf));
    }
}

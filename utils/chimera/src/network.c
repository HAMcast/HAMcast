/*
** $Id: network.c,v 1.30 2007/04/04 00:04:49 krishnap Exp $
**
** Matthew Allen
** description:
*/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <pthread.h>
#include "network.h"
#include "host.h"
#include "message.h"
#include "log.h"
#include "semaphore.h"
#include "jrb.h"
#include "jval.h"
#include "dtime.h"

extern int errno;
#define SEND_SIZE NETWORK_PACK_SIZE

typedef struct
{
    int sock;
    JRB waiting;
    uint32_t seqstart, seqend;
    pthread_mutex_t lock;
    JRB retransmit;
} NetworkGlobal;

// allocate a new pointer and return it
PQEntry* get_new_pqentry()
{
    PQEntry* entry = (PQEntry *)malloc(sizeof(PQEntry));
    entry->desthost = NULL;
    entry->data = NULL;
    entry->datasize = 0;
    entry->retry = 0;
    entry->seqnum = 0;
    entry->transmittime = 0.0;

    return entry;
}

AckEntry* get_new_ackentry()
{
    AckEntry *entry = (AckEntry *)(malloc(sizeof(AckEntry)));
    entry->acked = 0;
    entry->acktime = 0.0;

    return entry;
}

/** network_address:
 ** returns the ip address of the #hostname#
 */
uint32_t network_address (void *networkglobal, const char *hostname)
{
    int is_addr;
    struct hostent *he;
    uint32_t addr;
    uint32_t local, local1;
    int i;
    NetworkGlobal *netglob = (NetworkGlobal *) networkglobal;

    /* apparently gethostbyname does not portably recognize ip addys */

#ifdef SunOS
    is_addr = inet_addr (hostname);
    if (is_addr == -1) {
        is_addr = 0;
    }
    else {
        memcpy (&addr, (struct in_addr *) &is_addr, sizeof (addr));
        is_addr = inet_addr ("127.0.0.1");
        memcpy (&local, (struct in_addr *) &is_addr, sizeof (addr));
        is_addr = inet_addr ("127.0.1.1");
        memcpy (&local1, (struct in_addr *) &is_addr, sizeof (addr));
        is_addr = 1;
    }
#else
    is_addr = inet_pton (AF_INET, hostname, (struct in_addr *) &addr);
    inet_pton (AF_INET, "127.0.0.1", (struct in_addr *) &local);
    inet_pton (AF_INET, "127.0.1.1", (struct in_addr *) &local1);
#endif

    if (is_addr && (addr != local) && (addr != local1)) {
        return (addr);
    }

    pthread_mutex_lock (&(netglob->lock));
    if (is_addr)
        he = gethostbyaddr ((char *) &addr, sizeof (addr), AF_INET);
    else
        he = gethostbyname (hostname);

    if (he == NULL) {
        pthread_mutex_unlock (&(netglob->lock));
        return (0);
    }

    /* make sure the machine is not returning localhost */
    addr = *(uint32_t *) he->h_addr_list[0];
    for (i = 1; (he->h_addr_list[i] != NULL) &&
                ((addr == local) || (addr == local1)); i++) {
        addr = *(uint32_t *) he->h_addr_list[i];
    }
    pthread_mutex_unlock (&(netglob->lock));

    return (addr);
}

/** network_init:
 ** initiates the networking layer by creating socket and bind it to #port#
 */

void *network_init (void *logs, int port)
{
    int sd;
    int ret;
    struct sockaddr_in saddr;
    const int one = 1;
    NetworkGlobal *ng;

    ng = (NetworkGlobal *) malloc (sizeof (NetworkGlobal));

    /* create socket */
    sd = socket (AF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
    {
        return (NULL);
    }
    if (setsockopt (sd, SOL_SOCKET, SO_REUSEADDR, (const void *) &one, sizeof (one)) == -1)
    {
        close (sd);
        return (NULL);
    }

    /* attach socket to #port#. */
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl (INADDR_ANY);
    saddr.sin_port = htons ((short) port);
    if (bind (sd, (struct sockaddr *) &saddr, sizeof (saddr)) < 0)
    {
        close (sd);
        return (NULL);
    }

    if ((ret = pthread_mutex_init (&(ng->lock), NULL)) != 0)
    {
        close (sd);
        return (NULL);
    }

    ng->sock = sd;
    ng->waiting = make_jrb();
    ng->seqstart = 0;
    ng->seqend = 0;
    ng->retransmit = make_jrb();

    return ((void *) ng);
}

/** Never returns. Keep retransmitting the failed packets.
 **/
void *retransmit_packets(void *state)
{
    JRB pqnode, node;
    ChimeraState *chstate = (ChimeraState *) state;
    NetworkGlobal *ng = (NetworkGlobal *) chstate->network;
    double now = 0;
    PQEntry* pqentry;

    while(1) {
        // wake up, get all the packets to be transmitted by now,
        //  send them again or delete them from the priqueue
        now = dtime();
        int resend = 0;

        pthread_mutex_lock (&(ng->lock));
        for(pqnode = jrb_first(ng->retransmit);
            pqnode != jrb_nil(ng->retransmit) && pqnode->key.d <= now;
            pqnode = jrb_next(pqnode)) {
            pqentry = (PQEntry *) pqnode->val.v;
            if (pqentry != NULL) {
                node = jrb_find_int (ng->waiting, pqentry->seqnum);
            }
            else {
                continue;
            }
            if (node != NULL) {
                AckEntry *ackentry = (AckEntry *)node->val.v;
                if(ackentry->acked == 0) {// means, packet not yet acked
                    resend = 1;
                    double transmittime = dtime();
                    network_resend(state, pqentry->desthost,
                                    pqentry->data, pqentry->datasize, 1,
                                    pqentry->seqnum, &transmittime);
                    pqentry->retry = pqentry->retry + 1;
                    if (pqentry->retry < MAX_RETRY) {
                        PQEntry *newentry = get_new_pqentry();
                        newentry->desthost = pqentry->desthost;
                        newentry->data = pqentry->data;
                        newentry->datasize = pqentry->datasize;
                        newentry->retry = pqentry->retry;
                        newentry->seqnum = pqentry->seqnum;
                        newentry->transmittime = transmittime;

                        jrb_insert_dbl (ng->retransmit,
                                    (transmittime+RETRANSMIT_INTERVAL),
                                                new_jval_v (newentry));
                    } else {
                        // max retransmission has expired,
                        // update the host stats,
                        // free up the resources
                        host_update_stat (pqentry->desthost, 0);
                        if (pqentry->data != NULL) {
                            free(pqentry->data);
                            pqentry->data = NULL;
                        }
                    }
                    // delete this node
                    jrb_delete_node(pqnode);
                } else { // packet is acked;
                    // update the host latency and the success measurements
                    host_update_stat (pqentry->desthost, 1);
                    double latency =
                            ackentry->acktime - pqentry->transmittime;
                    if(latency > 0) {
                        if (pqentry->desthost->latency == 0.0) {
                            pqentry->desthost->latency = latency;
                        } else {
                            pqentry->desthost->latency =
                                (0.9 * pqentry->desthost->latency) +
                                (0.1 * latency);
                        }
                    }
                    jrb_delete_node(node);
                    jrb_delete_node(pqnode);
                }
            }
        }
        pthread_mutex_unlock (&(ng->lock));
        sleep(RETRANSMIT_THREAD_SLEEP);
    }
}

/**
 ** network_activate:
 ** NEVER RETURNS. Puts the network layer into listen mode. This thread
 ** manages acknowledgements, delivers incomming messages to the message
 ** handler, and drives the network layer. It should only be called once.
 */
void *network_activate (void *state)
{
    fd_set fds;
    int ret, retack;
    char data[SEND_SIZE];
    struct sockaddr_in from;
    size_t socklen = sizeof (from);
    uint32_t ack, seq;
    JRB node;
    ChimeraState *chstate = (ChimeraState *) state;
    NetworkGlobal *ng = (NetworkGlobal *) chstate->network;

    FD_ZERO (&fds);
    FD_SET (ng->sock, &fds);

    while (1)
    {
        /* block until information becomes available */
        /*
        memcpy (&thisfds, &fds, sizeof (fd_set));
        ret = select (ng->sock + 1, &thisfds, NULL, NULL, NULL);
        if (ret < 0)
        {
            continue;
        }
        */
        /* receive the new data */
        ret = recvfrom (ng->sock, data, SEND_SIZE, 0,
              (struct sockaddr *) &from, (socklen_t *) &socklen);
        if (ret < 0) {
            continue;
        }
        memcpy (&ack, data, sizeof (uint32_t));
        ack = ntohl (ack);
        memcpy (&seq, data + sizeof (uint32_t), sizeof (uint32_t));
        seq = ntohl (seq);

        /* process acknowledgement */
        if (ack == 0) {
            pthread_mutex_lock (&(ng->lock));
            node = jrb_find_int (ng->waiting, seq);
            if (node != NULL)
            {
                AckEntry *entry = (AckEntry *)node->val.v;
                entry->acked = 1;
                entry->acktime = dtime();
            }
            pthread_mutex_unlock (&(ng->lock));
        }
        else if (ack == 1) {/* process receive and send acknowledgement */
            ack = htonl (0);
            memcpy (data, &ack, sizeof (uint32_t));
            retack =
            sendto (ng->sock, data, NETWORK_HEADER_SIZE, 0,
                (struct sockaddr *) &from, sizeof (from));
            if (retack < 0)
            {
              continue;
            }
            message_received (state,
                      data + NETWORK_HEADER_SIZE,
                      ret - NETWORK_HEADER_SIZE);
        }
        else if (ack == 2) {
            message_received (state,
                      data + NETWORK_HEADER_SIZE,
                      ret - NETWORK_HEADER_SIZE);
        }
    }
}

/**
 ** network_send: host, data, size
 ** Sends a message to host, updating the measurement info.
 */
int network_send (void* state, ChimeraHost* host, char* data, size_t size, uint32_t ack)
{
    struct sockaddr_in to;
    int ret;
    size_t send_size = size;
    uint32_t seq, seqnumbackup, ntype;
    //char s[NETWORK_PACK_SIZE];
    char *s = data;
    s = s - NETWORK_HEADER_SIZE;
//    void *semaphore;
    JRB node;
    JRB priqueue;
    double start;
    ChimeraState *chstate = (ChimeraState *) state;
    NetworkGlobal *ng;

    ng = (NetworkGlobal *) chstate->network;

    if (size > NETWORK_PACK_SIZE) {
        fprintf(stderr, "packet too large!\n");
        return (0);
    }

    if (ack != 1 && ack != 2) {
        fprintf(stderr, "packet ack type not supported!\n");
        return (0);
    }
    memset (&to, 0, sizeof (to));
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = host->address;
    to.sin_port = htons ((short) host->port);

    AckEntry *ackentry = get_new_ackentry();
    /* get sequence number and initialize acknowledgement indicator*/
    pthread_mutex_lock (&(ng->lock));
    if (ack == 1) {
        node = jrb_insert_int (ng->waiting, ng->seqend, new_jval_v(ackentry));
    } else {
        free(ackentry);
    }
    seqnumbackup = ng->seqend;
    seq = htonl (ng->seqend);
    ng->seqend++;		/* needs to be fixed to modplus */
    pthread_mutex_unlock (&(ng->lock));

    /* create network header */
    ntype = htonl (ack);
    memcpy (s, &ntype, sizeof (uint32_t));
    memcpy (s + sizeof (uint32_t), &seq, sizeof (uint32_t));
    //memcpy (s + (2 * sizeof (uint32_t)), data, size);
    send_size += NETWORK_HEADER_SIZE;

    /* send data */
    seq = ntohl (seq);
    start = dtime ();
    ret = sendto (ng->sock, s, send_size, 0, (struct sockaddr *) &to, sizeof (to));

    if (ret < 0) {
        fprintf(stderr, "sendto host addr: %u name: %s failed!\n", host->address, host->name);
        host_update_stat (host, 0);
        return (0);
    }

    if (ack == 1)
    {
        // insert a record into the priority queue with the following information:
        // key: starttime + next retransmit time
        // other info: destination host, seq num, data, data size
        PQEntry *pqrecord = get_new_pqentry();
        pqrecord->desthost = host;
        //pqrecord->data = data;
        pqrecord->data = malloc (send_size);
        memcpy(pqrecord->data,s,send_size);
        //pqrecord->data += NETWORK_HEADER_SIZE;
        pqrecord->datasize = send_size;
        pqrecord->retry = 0;
        pqrecord->seqnum = seqnumbackup;
        pqrecord->transmittime = start;

        pthread_mutex_lock (&(ng->lock));
        priqueue = jrb_insert_dbl (ng->retransmit, (start+RETRANSMIT_INTERVAL),
            new_jval_v (pqrecord));
        pthread_mutex_unlock (&(ng->lock));
    }
    return (1);
}

/**
 ** Resends a message to host
 */
int network_resend (void *state, ChimeraHost *host, char *data, int size, int ack, uint32_t seqnum, double *transtime)
{
    struct sockaddr_in to;
    int ret;
    //char s[NETWORK_PACK_SIZE];
    //double start;
    ChimeraState *chstate = (ChimeraState *) state;
    NetworkGlobal *ng = (NetworkGlobal *) chstate->network;

    memset (&to, 0, sizeof (to));
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = host->address;
    to.sin_port = htons ((short) host->port);

    //uint32_t seq = htonl (seqnum);

    /* create network header */
    //uint32_t ntype = htonl (ack);
    //memcpy (s, &ntype, sizeof (uint32_t));
    //memcpy (s + sizeof (uint32_t), &seq, sizeof (uint32_t));
    //memcpy (s + (2 * sizeof (uint32_t)), data, size);
    //send_size += NETWORK_HEADER_SIZE;

    /* send data */
    //seq = ntohl (seq);
    *transtime = dtime();
    ret = sendto (ng->sock, data, size, 0, (struct sockaddr *) &to, sizeof (to));

    if (ret < 0) {
        fprintf(stderr, "(re)sendto host addr: %u name: %s failed!\n", host->address, host->name);
        host_update_stat (host, 0);
        return (0);
    }

    return (1);
}

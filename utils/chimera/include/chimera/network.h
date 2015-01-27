/*
** Matthew Allen
** description:
*/

#ifndef _CHIMERA_NETWORK_H_
#define _CHIMERA_NETWORK_H_

#include <stdint.h>
#include "host.h"
#include "jrb.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 ** NETWORK_PACK_SIZE is the maximum packet size that will be handled by chimera network layer
 */
#define NETWORK_PACK_SIZE 65536
#define NETWORK_HEADER_SIZE (2 * sizeof(uint32_t))
/**
 ** TIMEOUT is the number of seconds to wait for receiving ack from the destination, if you want
 ** the sender to wait forever put 0 for TIMEOUT.
 */
#define TIMEOUT 1.0

typedef struct PriqueueEntry{
    ChimeraHost *desthost; // who should this message be sent to?
    char *data; // what to send?
    int datasize; // how big is it?
    int retry; // number of retries
    uint32_t seqnum; // seqnum to identify the packet to be retransmitted
    double transmittime; // this is the time the packet is transmitted (or retransmitted)
}PQEntry;

typedef struct AcknowledgEntry{
    int acked;
    double acktime; // the time when the packet is acked
}AckEntry;

/** network_address:
 ** returns the ip address of the #hostname#
 */
uint32_t network_address (void *networkglobal, const char *hostname);

/** network_init:
 ** initiates the networking layer by creating socket and bind it to #port#
 */
void *network_init (void *logs, int port);

/**
 ** network_activate:
 ** NEVER RETURNS. Puts the network layer into listen mode. This thread
 ** manages acknowledgements, delivers incomming messages to the message
 ** handler, and drives the network layer. It should only be called once.
 */
void *network_activate (void *state);

// retransmit packets that are not acknowledged in a 1 sec window
void *retransmit_packets(void *state);

/**
 ** network_send: host, data, size
 ** Sends a message to host, updating the measurement info.
 ** type are 1 or 2, 1 indicates that the data should be acknowledged by the
 ** receiver, and 2 indicates that no ack is necessary.
 */
int network_send (void* state, ChimeraHost* host, char* data, size_t size, uint32_t type);

int network_resend (void *state, ChimeraHost *host, char *data,
            int size, int ack, uint32_t seqnum, double *transtime);

#ifdef __cplusplus
}
#endif

#endif /* _CHIMERA_NETWORK_H_ */

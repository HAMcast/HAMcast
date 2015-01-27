/*
** $Id: message.h,v 1.20 2007/04/04 00:04:49 krishnap Exp $
**
** Matthew Allen
** description: 
*/

#ifndef _CHIMERA_MESSAGE_H_
#define _CHIMERA_MESSAGE_H_

#include <stdint.h>
#include "key.h"
#include "host.h"
#include "jrb.h"
#include "include.h"

#define DEFAULT_SEQNUM 0
#define RETRANSMIT_THREAD_SLEEP 1
#define RETRANSMIT_INTERVAL 1
#define MAX_RETRY 3

#define SEQNUM 0

#ifdef __cplusplus
extern "C"
{
#endif


/* message on the wire is encoded in the following way:
** [ key.dst ] [ key.src ] [ port ] [ type ] [ size ] [ data ] */

#define CHIMERA_HEADER_SIZE ((KEY_ARRAY_SIZE * sizeof(uint32_t)) \
                            + (KEY_ARRAY_SIZE * sizeof(uint32_t)) \
                            + sizeof(uint16_t) \
                            + sizeof(uint16_t) \
                            + sizeof(uint16_t))

typedef struct
{
    Key dst;        /* destination key, for overlay routing */
    Key src;        /* the real group id the payload is meant to */
    uint16_t port;  /* port */
    uint16_t type;  /* message type */
    uint16_t size;  /* payload size */
    char *payload;  /* payload, actual data for app */
    char *buffer;   /* message buffer */
//    uint32_t seqnum;		/* for future security enhancement */
} Message;


typedef void (*messagehandler_t) (ChimeraState *, Message *);

/**
 ** message_init: chstate, port
 ** Initialize messaging subsystem on port and returns the MessageGlobal * which 
 ** contains global state of message subsystem.
 ** message_init also initiate the network subsystem
 */
void *message_init (void *chstate, int port);

/** 
 ** message_received:
 ** is called by network_activate and will be passed received data and size from socket
 **
 */
void message_received (void *chstate, char *data, uint16_t size);

/**
 ** registers the handler function #func# with the message type #type#,
 ** it also defines the acknowledgment requirement for this type 
 */
void message_handler (void *chstate, uint16_t type, messagehandler_t func,
		      int ack);

/**
 ** message_send:
 ** sendt the message to destination #host# the retry arg indicates to the network
 ** layer if this message should be ackd or not
 */
int message_send (void *chstate, ChimeraHost * host, Message * message,
		  Bool retry);

/** 
 ** message_create: 
 ** creates the message to the destination #dest# the message format would be like:
 **  [ type ] [ size ] [ key.dst ] [ key.grp ] [ data ] 
 ** It returns the created message structure.
 ** 
 */
Message *message_create (Key dst, Key src, uint16_t port, uint16_t type, uint16_t size, const char *payload);

/**
 *  @brief: Free allocated memory for message 
 */
void message_free(Message *msg);

#ifdef __cplusplus
}
#endif

#endif /* _CHIMERA_MESSAGE_H_ */

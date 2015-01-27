/*
** $Id: message.c,v 1.37 2007/04/04 00:04:49 krishnap Exp $
**
** Matthew Allen
** description: 
*/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "chimera.h"
#include "key.h"
#include "message.h"
#include "job_queue.h"
#include "log.h"
#include "network.h"
#include "jval.h"

typedef struct
{
    JRB handlers;
    void *jobq;
    pthread_attr_t attr;
    pthread_mutex_t lock;
} MessageGlobal;

typedef struct
{
    int ack;
    messagehandler_t handler;
    int priority;
    int reply;
} MessageProperty;

void print_message (Message* msg)
{
    printf ("+++ Messsage +++\n");
    printf (" - type=%u\n", msg->type);
    printf (" - size=%u\n", msg->size);
    printf (" - payload=%s\n", msg->payload);
}

/**
 * @brief __write_header_to_buffer
 * @param msg
 * @param write
 */
void write_header_to_buffer (Message* msg, char* write)
{
    write_key_to_buffer (msg->dst, write);
    write += (KEY_ARRAY_SIZE * sizeof(uint32_t));
    write_key_to_buffer (msg->src, write);
    write += (KEY_ARRAY_SIZE * sizeof(uint32_t));
    write_u16_to_buffer (msg->port, write);
    write += sizeof(uint16_t);
    write_u16_to_buffer (msg->type, write);
    write += sizeof(uint16_t);
    write_u16_to_buffer (msg->size, write);
    //print_message (msg);
}

/**
 * @brief read_buffer
 * @param buf
 * @param len
 * @return pointer to Message
 */
Message* read_buffer (char* buf, uint16_t len)
{
    Message *msg = (Message *) malloc (sizeof(Message));
    //msg->buffer = buf;
    msg->buffer = (char *) malloc (NETWORK_HEADER_SIZE + len);
    msg->buffer += NETWORK_HEADER_SIZE;
    msg->buffer = memcpy (msg->buffer, buf, len);
    char* read = msg->buffer;
    msg->payload = msg->buffer + CHIMERA_HEADER_SIZE;
    msg->dst = read_key_from_buffer(read);
    read += (KEY_ARRAY_SIZE*sizeof(uint32_t));
    msg->src = read_key_from_buffer(read);
    read += (KEY_ARRAY_SIZE*sizeof(uint32_t));
    msg->port = read_u16_from_buffer(read);
    read += sizeof(uint16_t);
    msg->type = read_u16_from_buffer(read);
    read += sizeof(uint16_t);
    msg->size = read_u16_from_buffer(read);
    //print_message (msg);
    return msg;
}

/** 
 ** message_create: 
 ** creates the message to the destination #dest# the message format would be like:
 ** [ key.dst ] [key.src] [ port ] [ type ] [ size ] [ data ]
 ** It return the created message structure.
 ** 
 */
Message* message_create (Key dst, Key src, uint16_t port, uint16_t type, uint16_t size, const char *payload)
{
    Message* msg = (Message*) malloc (sizeof (Message));
    msg->buffer = (char *) malloc (NETWORK_HEADER_SIZE + CHIMERA_HEADER_SIZE + size + 1);
    msg->buffer += NETWORK_HEADER_SIZE;
    key_assign (&(msg->dst), dst);
    key_assign (&(msg->src), src);
    msg->port = port;
    msg->type = type;
    msg->size = size;
    msg->payload = msg->buffer + CHIMERA_HEADER_SIZE;
    memcpy (msg->payload, payload, size);
    msg->buffer[CHIMERA_HEADER_SIZE + size] = '\0';
    write_header_to_buffer (msg, msg->buffer);
    return (msg);
}

/** 
 ** message_free:
 ** free the message and the payload
 */
void message_free (Message * msg)
{
    if (msg != NULL) {
    //FIXME: causes SEGFAULTs due to double frees
        if (msg->buffer != NULL) {
            msg->buffer -= NETWORK_HEADER_SIZE;
            free (msg->buffer);
            msg->buffer = NULL;
        }
        free (msg);
        msg = NULL;
    }
}

/**
 ** message_init: chstate, port
 ** Initialize messaging subsystem on port and returns the MessageGlobal * which 
 ** contains global state of message subsystem.
 ** message_init also initiate the network subsystem
 */
void *message_init (void *chstate, int port)
{
    pthread_t tid;
    MessageGlobal *mg;
    ChimeraState *state = (ChimeraState *) chstate;

    mg = (MessageGlobal *) malloc (sizeof (MessageGlobal));
    state->message = (void *) mg;

    mg->handlers = make_jrb ();
    state->network = network_init (state->log, port);

    if (state->network == NULL)
    {
        return (NULL);
    }

    if (pthread_attr_init (&mg->attr) != 0) {
        fprintf(stderr, "pthread_attr_init: %s", strerror (errno));
        return (NULL);
    }
    if (pthread_attr_setscope (&mg->attr, PTHREAD_SCOPE_SYSTEM) != 0) {
        fprintf(stderr, "pthread_attr_setscope: %s", strerror (errno));
        return (NULL);
    }
    if (pthread_attr_setdetachstate (&mg->attr, PTHREAD_CREATE_DETACHED) != 0) {
        fprintf(stderr, "pthread_attr_setdetachstate: %s", strerror (errno));
        return (NULL);
    }

    if (pthread_mutex_init (&mg->lock, NULL) != 0) {
        fprintf(stderr, "pthread_mutex_init: %s", strerror (errno));
        return (NULL);
    }

    mg->jobq = job_queue_init (JOB_QUEUE_SIZE);

    if (pthread_create (&tid, &mg->attr, network_activate, (void *) state) != 0) {
        fprintf(stderr, "pthread_create for network_activate: %s",
                strerror (errno));
        return (NULL);
    }
    // FIXME: this may cause SEGFAULTs under certain circumstances
    if (pthread_create (&tid, &mg->attr, retransmit_packets, (void *) state) != 0) {
        fprintf(stderr, "pthread_create failed for retransmit_packets: %s",
                strerror (errno));
        return (NULL);
    }
    return ((void *) mg);
}

/**
 ** message_receiver:
 ** is called by the message_received func. this function will find the proper 
 ** handler for the message type and passe the handler and its required args to 
 ** the job_queue
 */
void message_receiver (void *chstate, Message * message)
{
    JRB node;
    MessageProperty *msgprop;
    messagehandler_t func;
    ChimeraState *state = (ChimeraState *) chstate;
    MessageGlobal *msgglob = (MessageGlobal *) state->message;

    /* find message handler */
    pthread_mutex_lock (&msgglob->lock);
    node = jrb_find_int (msgglob->handlers, message->type);

    if (node == NULL) {
        pthread_mutex_unlock (&msgglob->lock);
        message_free (message);
        return;
    }
    msgprop = node->val.v;
    func = msgprop->handler;

    if (func == NULL) {
        pthread_mutex_unlock (&msgglob->lock);
        message_free (message);
        return;
    }
    pthread_mutex_unlock (&msgglob->lock);

    /* call the handler */
    func (chstate, message);
    message_free (message);
}

/** 
 ** message_received:
 ** is called by network_activate and will be passed received data and size from socket
 **
 */
void message_received (void *chstate, char *data, uint16_t size)
{
    Message *message;
    ChimeraState *state = (ChimeraState *) chstate;
    MessageGlobal *msgglob = (MessageGlobal *) state->message;
    JobArgs *jargs;

    /*
     * message format on the wire
     * [ key.dst ] [ key.src ] [ port ] [ type ] [ size ] [ data ]
     *             [ key.grp ]
     */
    jargs = (JobArgs *) malloc (sizeof (JobArgs));
    message = read_buffer(data, size);

    jargs->state = state;
    jargs->msg = message;

    job_submit (msgglob->jobq, (FuncPtr) message_receiver, (void *) jargs, 0);
}

/**
 ** registers the handler function #func# with the message type #type#,
 ** it also defines the acknowledgment requirement for this type 
 */
void message_handler (void *chstate, uint16_t type, messagehandler_t func, int ack)
{
    JRB node;
    ChimeraState *state = (ChimeraState *) chstate;
    MessageGlobal *msgglob = (MessageGlobal *) state->message;
    MessageProperty *msgprop =
    (MessageProperty *) malloc (sizeof (MessageProperty));

    msgprop->handler = func;
    msgprop->ack = ack;

    /* add message handler function into the set of all handlers */
    pthread_mutex_lock (&msgglob->lock);
    node = jrb_find_int (msgglob->handlers, type);

    /* don't allow duplicates */
    if (node != NULL) {
        pthread_mutex_unlock (&msgglob->lock);
        return;
    }
    jrb_insert_int (msgglob->handlers, type, new_jval_v (msgprop));

    pthread_mutex_unlock (&msgglob->lock);
}

/**
 ** message_send:
 ** send the message to destination #host# the retry arg indicates to the network
 ** layer if this message should be ackd or not
 */
int message_send (void *chstate, ChimeraHost * host, Message * message,
                     Bool retry)
{
    char *data;
    size_t size = 0;
    int ret = 0;
    ChimeraState *state = (ChimeraState *) chstate;
    MessageGlobal *msgglob = (MessageGlobal *) state->message;
    JRB node;
    MessageProperty *msgprop;

    if (host == NULL) {
        fprintf (stderr, "no host given!\n");
        return (0);
    }

    /*
     * message format on the wire:
     * [ key.dst ] [ key.src ] [ port ] [ type ] [ size ] [ data ]
     */
    size = CHIMERA_HEADER_SIZE + message->size;
    data = message->buffer;

    /* get the message properties */
    pthread_mutex_lock (&msgglob->lock);
    node = jrb_find_int (msgglob->handlers, message->type);
    if (node == NULL) {
        fprintf(stderr, "no message handler for type: %d!\n", message->type);
        pthread_mutex_unlock (&msgglob->lock);
        return 0;
    }
    msgprop = node->val.v;
    pthread_mutex_unlock (&msgglob->lock);

    /* send the message */
    ret = network_send (state, host, data, size, msgprop->ack);
    return (ret);
}

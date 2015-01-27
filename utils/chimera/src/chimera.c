/*
**
** $Id: chimera.c,v 1.49 2006/09/07 04:12:07 krishnap Exp $
**
** Matthew Allen
** description: 
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "chimera.h"
#include "host.h"
#include "message.h"
#include "route.h"
#include "semaphore.h"
#include "key.h"
#include "log.h"
#include "dtime.h"
#include "network.h"
#include "include.h"

int chimera_encodehosts (void *logs, char *s, int size, ChimeraHost ** host)
{
    int i, j, ret;
    s[0] = 0;
    j=0;
    for (i = 0; host[i] != NULL; i++) {
        if ((ret = host_encode (s + j, size - j, host[i])) != -1) {
            j += ret;
        }
    }
    return j;
}

ChimeraHost **chimera_decodehosts (ChimeraState * state, char *s, size_t slen)
{
    ChimeraHost **host;
    int hostnum;
    int i, j;

    hostnum = slen/HOST_CODED_SIZE;
    host = (ChimeraHost **) malloc (sizeof (ChimeraHost *) * (hostnum + 1));

    /* gets the number of hosts in the lists and goes through them 1 by 1 */
    for (i = 0, j = 0; i < hostnum; i++) {
        host[i] = host_decode (state, s + j);
        j += HOST_CODED_SIZE;
    }
    host[i] = NULL;

    return (host);
}

/** 
 ** chimera_send_rowinfo:
 ** sends matching row of its table to the joining node while 
 ** forwarding the message toward the root of the joining node
 **
 */
void chimera_send_rowinfo (ChimeraState * state, Message * message)
{
    ChimeraHost **rowinfo;
    char s[NETWORK_PACK_SIZE];
    ChimeraHost *host;
    Message *msg;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    host = host_decode (state, message->payload);

    /* send one row of our routing table back to joiner #host# */
    rowinfo = route_row_lookup (state, host->key);
    size_t size = chimera_encodehosts (state->log, s, NETWORK_PACK_SIZE, rowinfo);
    msg = message_create (host->key, chglob->me->key, 0, CHIMERA_PIGGY, size, s);
    if (!message_send (state, host, msg, TRUE)) {
        fprintf(stderr, "Sending row information to node: %s:%d failed\n",
                host->name, host->port);
    }
    free (rowinfo);
    message_free (msg);
}

/** chimera_join_complete:
** internal function that is called at the destination of a JOIN message. This
** call encodes the leaf set of the current host and sends it to the joiner.
** 
*/

void chimera_join_complete (ChimeraState * state, ChimeraHost * host)
{
    char s[NETWORK_PACK_SIZE];
    Message *m;
    ChimeraHost **leafset;
    ChimeraGlobal *chglob = (ChimeraGlobal *) (state->chimera);

    /* copy myself into the reply */
    size_t slen = host_encode (s, sizeof(s), chglob->me);
    /* check to see if the node has just left the network or not */
    if (((dtime () - host->failuretime) < GRACEPERIOD) && (host->failuretime != 0)) {
        m = message_create (host->key, chglob->me->key, 0, CHIMERA_JOIN_NACK,
                            slen, s);
	    if (!message_send (state, host, m, TRUE)) {
            fprintf(stderr,"message_send NACK failed!\n");
        }
	}
    else {
        /* copy my leaf set into the reply */
        leafset = route_neighbors (state, LEAFSET_SIZE);
        slen += chimera_encodehosts (state->log, s+slen, sizeof(s)-slen, leafset);
        free (leafset);

        m = message_create (host->key, chglob->me->key, 0, CHIMERA_JOIN_ACK, slen, s);
        if (!message_send (state, host, m, TRUE)) {
            fprintf(stderr, "message_send ACK failed!\n");
        }
    }
    message_free (m);
}

/**
 ** chimera_check_leafset: runs as a separate thread. 
 ** it should send a PING message to each member of the leafset frequently and 
 ** sends the leafset to other members of its leafset periodically. 
 ** pinging frequecy is LEAFSET_CHECK_PERIOD.
 **
 */
void *chimera_check_leafset (void *chstate)
{
    char s[NETWORK_PACK_SIZE];
    Message *m;
    ChimeraHost **leafset;
    ChimeraHost **table;
    int i, count = 0;
    ChimeraState *state = (ChimeraState *) chstate;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    while (1) {
	    leafset = route_neighbors (state, LEAFSET_SIZE);
        for (i = 0; leafset[i] != NULL; i++) {
            if (chimera_ping (state, leafset[i]) == 1) {
			    leafset[i]->failuretime = dtime ();
            }
            if (leafset[i]->success_avg < BAD_LINK) {
                fprintf (stderr, "Deleting %s:%d \n", leafset[i]->name,
                                                      leafset[i]->port);
                route_update (state, leafset[i], 0);
            }
            host_release (state, leafset[i]);
		}
	    free (leafset);
        leafset = NULL;

	    table = route_get_table (state);
        for (i = 0; table[i] != NULL; i++) {
            if (chimera_ping (state, table[i]) == 1) {
			    table[i]->failuretime = dtime ();
                fprintf(stderr, "message send to host: %s:%d failed at time: %f!\n",
                        table[i]->name, table[i]->port, table[i]->failuretime);
			}
            if (table[i]->success_avg < BAD_LINK) {
                fprintf (stderr, "Deleting %s:%d \n", table[i]->name,
                                                      table[i]->port);
                route_update (state, table[i], 0);
            }
		    host_release (state, table[i]);
		}
	    free (table);
        table = NULL;

	    /* send leafset exchange data every  3 times that pings the leafset */
        if (count == 2) {
		    count = 0;
		    leafset = route_neighbors (state, LEAFSET_SIZE);
            size_t slen = host_encode (s, sizeof(s), chglob->me);
            slen += chimera_encodehosts (state->log, s+slen, sizeof(s)-slen, leafset);

            for (i = 0; leafset[i] != NULL; i++) {
                m = message_create (leafset[i]->key, chglob->me->key, 0,
                        CHIMERA_PIGGY, slen, s);
                if (!message_send (state, leafset[i], m, TRUE)) {
                    fprintf(stderr, "sending leafset update to %s:%d failed!\n",
                            leafset[i]->name, leafset[i]->port);
                    if (leafset[i]->success_avg < BAD_LINK) {
                        route_update (state, leafset[i], 0);
					}
				}
			    message_free (m);
			    host_release (state, leafset[i]);
			}

		    free (leafset);
            leafset = NULL;
		}
        else {
            count++;
        }
	    sleep (LEAFSET_CHECK_PERIOD);
	}
}

/** 
 ** chimera_check_leafset_init:
 ** initiates a separate thread that constantly checks to see if the leafset members  
 ** and table entries of the node are alive or not. 
 **
 */
int chimera_check_leafset_init (ChimeraState * state)
{

    pthread_attr_t attr;
    pthread_t tid;

    if (pthread_attr_init (&attr) != 0) {
        fprintf(stderr, "(CHIMERA)pthread_attr_init: %s", strerror (errno));
	    return (0);
	}
    if (pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM) != 0) {
        fprintf(stderr, "(CHIMERA)pthread_attr_setscope: %s", strerror (errno));
	    goto out;
	}
    if (pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED) != 0) {
        fprintf(stderr, "(CHIMERA)pthread_attr_setdetachstate: %s", strerror (errno));
	    goto out;
	}

    if (pthread_create(&tid, &attr, chimera_check_leafset, (void *) state) != 0) {
        fprintf(stderr, "(CHIMERA)pthread_create: %s", strerror (errno));
	    goto out;
	}
    return (1);

  out:
    pthread_attr_destroy (&attr);
    return (0);
}

/**
 ** chimera_join_denied
 ** internal function that is called when the sender of a JOIN message receives 
 ** the JOIN_NACK message type which is join denial from the current key root 
 ** in the network.
 */
void chimera_join_denied (ChimeraState * state, Message * message)
{
    ChimeraHost *host;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    host = host_decode (state, message->payload);

    fprintf(stderr, "JOIN request rejected from %s:%d !\n",
            host->name, host->port);
    sleep (GRACEPERIOD+1);

    fprintf(stderr, "Re-sending JOIN message to %s:%d !\n",
            chglob->bootstrap->name, chglob->bootstrap->port);

    chimera_join (state, chglob->bootstrap);
}


/**
 ** chimera_route:
 ** routes a message one step closer to its destination key. Delivers
 ** the message to its destination if it is the current host through the
 ** deliver upcall, otherwise it makes the route upcall 
 */
void chimera_route (ChimeraState * state, Key * key, Message * message,
                    ChimeraHost * host)
{
    ChimeraHost **tmp;
    Message *real;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    real = message;

    if (key_equal (*key, chglob->me->key)) {
        tmp = (ChimeraHost **) malloc (2 * sizeof (ChimeraHost*));
        tmp[0] = chglob->me;
    }
    else {
        tmp = route_lookup (state, *key, 1, 0);
    }

    /* this is to avoid sending JOIN request to the node that
     ** its information is already in the routing table  ****/

    if ((message->type == CHIMERA_JOIN) && (tmp[0] != NULL)
                         && (key_equal (tmp[0]->key, *key))) {
        free (tmp);
        tmp = route_lookup (state, *key, 2, 0);
        if (tmp[1] != NULL && key_equal (tmp[0]->key, *key))
        tmp[0] = tmp[1];
	}

    if (host == NULL && tmp[0] != chglob->me) {
	    host = tmp[0];
	}

    if (tmp[0] == chglob->me) {
	    host = NULL;
	}

    /* if I am the only host or the closest host is me, deliver the message */
    if (host == NULL) {
        if (chglob->deliver != NULL) {
		    chglob->deliver (key, real);
		}
        if (message->type == CHIMERA_JOIN) {
		    host = host_decode (state, message->payload);
		    chimera_join_complete (state, host);
		}
	}

    /* otherwise, route it */
    else {
        if (chglob->forward != NULL) {
		    //chglob->forward (&key, &real, &host);
		    chglob->forward (&key, &message, &host);
		}
	    //message = real;
	    while (!message_send (state, host, message, TRUE)) {
		    host->failuretime = dtime ();
            fprintf(stderr, "message send to host: %s:%d at time: %f failed!\n",
                    host->name, host->port, host->failuretime);

		    /* remove the faulty node from the routing table */
		    if (host->success_avg < BAD_LINK)
			    route_update (state, host, 0);

		    if (tmp != NULL)
	    	    free (tmp);
		    tmp = route_lookup (state, *key, 1, 0);
		    host = tmp[0];
            fprintf(stderr, "rerouting through %s:%d!\n", host->name, host->port);
		}

	    /* in each hop in the way to the key root nodes
	       send their routing info to the joining node  */

        if (message->type == CHIMERA_JOIN) {
            chimera_send_rowinfo (state, message);
        }
	}
    if (tmp != NULL) {
        free (tmp);
        tmp = NULL;
    }
}

/** 
 * chimera_join_acknowledge:
 * called when the current host is joining the network and has just revieced
 * its leaf set. This function sends an update message to all nodes in its
 * new leaf set to announce its arrival.
 */
void chimera_join_acknowledged (ChimeraState * state, Message * message)
{
    ChimeraHost **host;
    Message *m;
    char s[256];
    int i;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    size_t slen = host_encode (s, sizeof(s), chglob->me);
    host = chimera_decodehosts (state, message->payload, message->size);

    /* announce my arrival to the nodes in my leafset */
    for (i = 0; host[i] != NULL; i++) {
	    route_update (state, host[i], 1);
        m = message_create (host[i]->key, chglob->me->key, 0,
                            CHIMERA_UPDATE, slen, s);
        if (!message_send (state, host[i], m, TRUE)) {
            fprintf(stderr, "chimera_join_acknowledge: failed to update %s:%d\n",
                    host[i]->name, host[i]->port);
		}
	    message_free (m);
	}
    free (host);

    /* announce my arival to the nodes in my routing table */
    host = route_get_table (state);
    for (i = 0; host[i] != NULL; i++) {
        m = message_create (host[i]->key, chglob->me->key, 0,
                            CHIMERA_UPDATE, slen, s);
        if (!message_send (state, host[i], m, TRUE)) {
            fprintf(stderr, "chimera_join_acknowledge: failed to update %s:%d\n",
                    host[i]->name, host[i]->port);
        }
        message_free (m);
    }
    free (host);

    /* signal the chimera_join function, which is blocked awaying completion */
    sema_v (chglob->join);

    /* initialize the thread for leafset check and exchange */
    if (!(chimera_check_leafset_init (state))) {
        fprintf(stderr, "chimera_check_leafset_init FAILED \n");
	    return;
	}
}

/** 
 ** chimera_message: 
 ** routes the message through the chimera_route toward the destination
 **
 */
void chimera_message (ChimeraState * state, Message * message)
{
    chimera_route (state, &(message->dst), message, NULL);
}

void chimera_register (ChimeraState * state, uint16_t type, int ack)
{
    if (type < 10) {
        fprintf(stderr, "chimera_register: message integer types < 10 are reserved for system\n");
	    exit (1);
	}

    if (ack != 1 && ack != 2) {
        fprintf(stderr,
                "chimera_register: unrecognized ack value %i\n", ack);
	    exit (1);
	}
    message_handler (state, type, chimera_message, ack);
}

void chimera_update_message (ChimeraState * state, Message * message)
{
    ChimeraHost *host;
    host = host_decode (state, message->payload);
    route_update (state, host, 1);
}

/** 
 ** chimera_piggy_message:
 ** message handler for message type PIGGY ;) this used to be a piggy backing function 
 ** This function is respopnsible to add the piggy backing node information that is sent along with 
 ** other ctrl messages or separately to the routing table. the PIGGY message type is a separate 
 ** message type. 
 */
void chimera_piggy_message (ChimeraState * state, Message * message)
{
    ChimeraHost **piggy;
    int i;

    piggy = chimera_decodehosts (state, message->payload, message->size);

    for (i = 0; piggy[i] != NULL; i++) {

        if ((dtime () - piggy[i]->failuretime) > GRACEPERIOD) {
		    route_update (state, piggy[i], 1);
		}
        else {
            fprintf(stderr, "refused to add:%s to routing table\n",
                    get_key_string (&piggy[i]->key));
        }
	}
    free (piggy);
}

extern void route_keyupdate ();

void chimera_setkey (ChimeraState * state, Key key)
{

    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    key_assign (&(chglob->me->key), key);
    route_keyupdate (state->route, chglob->me);

}

/**
 ** chimera_ping: 
 ** sends a PING message to the host. The message is acknowledged in network layer.
 */
int chimera_ping (ChimeraState * state, ChimeraHost * host)
{
    char s[256];
    Message *message;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    size_t slen = host_encode (s, sizeof(s), chglob->me);

    if (host == NULL) {
	    return -1;
	}
    message = message_create (chglob->me->key, chglob->me->key, 0,
                              CHIMERA_PING, slen, s);
    host_update_stat (host, 0);
    if (!message_send (state, host, message, FALSE)) {
        fprintf(stderr, "failed to ping host %s:%d\n", host->name, host->port);
	    message_free (message);
	    return 1;
	}
    message_free (message);
    return 0;
}

void chimera_ping_reply (ChimeraState * state, Message * message)
{
    ChimeraHost *host;
    host = host_decode (state, message->payload);
    host_update_stat (host, 1);
}


/**
 ** chimera_init:
 ** Initializes Chimera on port port and returns the ChimeraState * which 
 ** contains global state of different chimera modules.
*/
ChimeraState *chimera_init (const char* host, int port)
{
    char name[256];
    struct hostent *he;
    ChimeraState *state;
    ChimeraGlobal *cg;
    //  mtrace();
    state = (ChimeraState *) malloc (sizeof (ChimeraState));
    cg = (ChimeraGlobal *) malloc (sizeof (ChimeraGlobal));
    state->chimera = (void *) cg;

    state->log = log_init ();
    log_direct (state->log, LOG_ERROR, stderr);
    key_init ();

    state->message = message_init ((void *) state, port);
    if (state->message == NULL) {
        return (NULL);
    }

    state->host = host_init (state->log, 64);
    if (state->host == NULL) {
        return (NULL);
    }

    if (host == NULL) {
        if (gethostname (name, 256) != 0) {
            return (NULL);
        }
        if ((he = gethostbyname (name)) == NULL) {
            return (NULL);
        }
        strcpy (name, he->h_name);
        cg->me = host_get (state, name, port);
    }
    else {
        cg->me = host_get (state, host, port);
    }
    char id[256];
    memset (id, 0, sizeof(id));
    inet_ntop (AF_INET, &(cg->me->address), id, sizeof(id));
    // append port to id
    sprintf (id + strlen (id), ":%d", port);
    key_makehash (state->log, &(cg->me->key), id);
    cg->deliver = NULL;
    cg->forward = NULL;
    cg->update = NULL;

    state->route = route_init (cg->me);

    message_handler (state, CHIMERA_JOIN, chimera_message, 1);
    message_handler (state, CHIMERA_JOIN_ACK, chimera_join_acknowledged, 1);
    message_handler (state, CHIMERA_UPDATE, chimera_update_message, 1);
    message_handler (state, CHIMERA_PIGGY, chimera_piggy_message, 1);
    message_handler (state, CHIMERA_JOIN_NACK, chimera_join_denied, 1);
    message_handler (state, CHIMERA_PING, chimera_ping_reply, 1);

    /* more message types can be defined here */

    pthread_mutex_init (&cg->lock, NULL);
    cg->join = sema_create (0);

    return (state);
}

/** 
 ** chimera_join:
 ** sends a JOIN message to bootstrap node and waits forever for the reply
 ** 
 */
void chimera_join (ChimeraState * state, ChimeraHost * bootstrap)
{
    char s[256];
    Message *message;
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;

    size_t slen = host_encode (s, sizeof(s), chglob->me);
    if (bootstrap == NULL) {
	    if (!(chimera_check_leafset_init (state))) {
            fprintf (stderr, "chimera_check_leafset_init FAILED \n");
        }
	    return;
	}

    chglob->bootstrap = host_get (state, bootstrap->name, bootstrap->port);

    message = message_create (chglob->me->key, chglob->me->key, 0,
                              CHIMERA_JOIN, slen, s);
    if (!message_send (state, bootstrap, message, TRUE)) {
        fprintf(stderr, "chimera_join: failed to contact bootstrap host %s:%d\n",
                bootstrap->name, bootstrap->port);
	}

    sema_p (chglob->join, 0.0);
    message_free (message);
}


/** chimera_send: 
 ** Route a message of type to key containing size bytes of data. This will
 ** send data through the Chimera system and deliver it to the host closest to the
 ** key. 
 */
void chimera_send (ChimeraState * state, Key dst, Key src, uint16_t port,
                   uint16_t type, uint16_t size, const char *data)
{
    Message *message;
    message = message_create (dst, src, port, type, size, data);
    chimera_message (state, message);
    message_free (message);
}

/** 
 ** chimera_forward:
 ** is called whenever a message is forwarded toward the destination the func upcall 
 ** should be  defined by the user application
 */
void chimera_forward (ChimeraState * state, chimera_forward_upcall_t func)
{
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;
    chglob->forward = func;
}

/** 
 ** chimera_deliver:
 ** is called whenever a message is delivered toward the destination the func upcall 
 ** should be  defined by the user application
 */
void chimera_deliver (ChimeraState * state, chimera_deliver_upcall_t func)
{
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;
    chglob->deliver = func;
}

/** 
 ** chimera_update:
 ** is called whenever a node joines or leaves the leafset the func upcall 
 ** should be  defined by the user application
 */
void chimera_update (ChimeraState * state, chimera_update_upcall_t func)
{
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;
    chglob->update = func;
}


/**
 ** called by route_update to upcall into the user's system 
 */
void chimera_update_upcall (ChimeraState * state, Key * k, ChimeraHost * h,
			    int joined)
{
    ChimeraGlobal *chglob = (ChimeraGlobal *) state->chimera;
    if (chglob->update != NULL) {
	    chglob->update (k, h, joined);
	}
}

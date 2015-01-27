/*
 * ** $Id: host.h,v 1.15 2006/06/07 09:21:28 krishnap Exp $
 * **
 * ** Matthew Allen
 * ** description:
 * */

#ifndef _HOST_H_
#define _HOST_H_

#include <stdint.h>
#include "key.h"
#include "jrb.h"
#include "include.h"

#define SUCCESS_WINDOW 20
#define GOOD_LINK 0.7
#define BAD_LINK 0.1

// encoded host: [ key (20) | addr (4) | port (2) ] = 26 bytes
#define HOST_CODED_SIZE (KEY_ARRAY_SIZE*sizeof(uint32_t) +sizeof(uint32_t) +sizeof(uint16_t))

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    char *name;
    uint32_t address;
    uint16_t port;
    int failed;
    double failuretime;
    double latency;
    double loss;
    double success;
    int success_win[SUCCESS_WINDOW];
    int success_win_index;
    float success_avg;
    Key key;
} ChimeraHost;

typedef struct
{
    void *network;
    void *message;
    void *route;
    void *log;
    void *host;
    void *chimera;
    JRB bootstrapMsgStore;	/* for future security enhancement */
    pthread_mutex_t bootstrapMutex;	/* for future security enhancement */
    void *certificateStore;	/* for future security enhancement */
    pthread_mutex_t certificateMutex;	/* for future security enhancement */
} ChimeraState;

/** host_get:
 ** gets a host entry for the given host, getting it from the cache if
 ** possible, or alocates memory for it
 */
ChimeraHost *host_get (ChimeraState * state, const char *hn, int port);

/** host_release:
 ** releases a host from the cache, declaring that the memory could be
 ** freed any time. returns NULL if the entry is deleted, otherwise it
 ** returns #host#
 */
void host_release (ChimeraState * state, ChimeraHost * host);

/** host_decode:
 ** decodes a string into a chimera host structure. This acts as a
 ** host_get, and should be followed eventually by a host_release.
 */
ChimeraHost *host_decode (ChimeraState * state, const char *s);

/** host_encode:
 ** encodes the #host# into a string, putting it in #s#, which has
 ** #len# bytes in it.
 */
int host_encode(char *s, size_t len, ChimeraHost * host);

/** host_update_stat:
 ** updates the success rate to the host based on the SUCCESS_WINDOW average
 */
void host_update_stat (ChimeraHost * host, int success);

/** host_init:
 ** initialize a host struct with a #size# element cache.
 */
void *host_init (void *logs, int size);

#ifdef __cplusplus
}
#endif

#endif /* _HOST_H_ */

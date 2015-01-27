#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "job_queue.h"

/* returns the first element of the list (queue) */
node *list_get (List * l)
{
    node *curr;
    curr = l->head;

    if (curr != NULL) {
        l->head = curr->next;
    }
    l->size--;
    return (curr);
}

/* creates a pointer to node and returns a pointer to it */
node *list_make_node (FuncPtr func, void *args, int args_size)
{
    node *nnew;

    nnew = (node *) malloc (sizeof (node));
    nnew->fp = func;
    nnew->args = args;
    nnew->next = NULL;

    return nnew;
}

/* insert a node new at the end of the list(queue) */
void list_insert (List * l, node * nnew)
{
    if (l->head == NULL) {
        l->head = nnew;
    }
    else {
        l->tail->next = nnew;
    }
    l->tail = nnew;
    l->size++;
}

int list_empty (List * l)
{
    if (l->head == NULL)
        return 1;
    else
        return 0;
}

/**
 ** initiate the list(queue).
 */
List *list_init ()
{
    List *nnew;

    nnew = (List *) malloc (sizeof (List));

    nnew->head = NULL;
    nnew->tail = NULL;
    nnew->size = 0;
    pthread_mutex_init (&nnew->access, NULL);
    pthread_cond_init (&nnew->empty, NULL);
    return (nnew);
}

void free_node (node * n)
{
    if (n->args != NULL) {
        free (n->args);
        n->args=NULL;
    }
    free (n);
    n=NULL;
}

/** get the queue mutex "access" then
 * if the queue is emapty it would go to sleep and release the mutex
 * else get the first job out of queue and execute it 
 */
void *job_exec (void *job_q)
{
    List *Q = (List *) job_q;
    node *tmp;
    JobArgs *jargs;

    while (1) {
        pthread_mutex_lock (&Q->access);
        while (list_empty (Q)) {
            pthread_cond_wait (&Q->empty, &Q->access);
        }
        tmp = list_get (Q);
        pthread_mutex_unlock (&Q->access);

        jargs = (JobArgs *) tmp->args;
        tmp->fp (jargs->state, jargs->msg);

        free_node (tmp);
    }
    return NULL;
}

/** get the queue mutex "access",
 * creat a new node and pass func,args,args_size,
 * add the new node to the queue, and 
 * signal the thread pool if the queue was empty.
 */
void job_submit (List * job_q, FuncPtr func, void *args, int args_size)
{

    int was_empty = 0;
    //  JobArgs * jargs = (jobArgs *)args;
    pthread_mutex_lock (&job_q->access);
    if (list_empty (job_q))
        was_empty = 1;

    list_insert (job_q, list_make_node (func, args, args_size));
    if (was_empty)
        pthread_cond_signal (&job_q->empty);

    pthread_mutex_unlock (&job_q->access);
}

/** initiate the queue and thread pool,
 * returns a pointer to the initiated queue. 
 */
List *job_queue_init (int pool_size)
{
    int i;
    List *Q;
    pthread_t *tids;

    tids = (pthread_t *) malloc (sizeof (pthread_t) * pool_size);
    /* Initialize the job queue */
    Q = list_init ();
    /* creat the thread pool */
    for (i = 0; i < pool_size; i++) {
        pthread_create (&tids[i], NULL, job_exec, (void *) Q);
    }
    return Q;
}

#ifndef _JOB_QUEUE_H_
#define _JOB_QUEUE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define JOB_QUEUE_SIZE 1

typedef void (*FuncPtr) (void *, void *);

/* job_queue node structure */
typedef struct node
{
    FuncPtr fp;			/* pointer to function */
    void *args;			/* functioin arguements */
    struct node *next;
} node;

/* queue_queue structure */
typedef struct
{
    node *head;
    node *tail;
    int size;
    pthread_mutex_t access;
    pthread_cond_t empty;
} List;

typedef struct
{
    void *state;
    void *msg;
} JobArgs;

/** job_exec: 
 ** if the queue,"job_q" is emapty it would go to sleep and releas the mutex
 **  else get the first job out of queue and execute it. 
 */
void *job_exec (void *job_q);

/** job_submit: 
 ** creat a new node and pass "func","args","args_size" 
 ** add the new node to the queue
 ** signal the thread pool if the queue was empty
 */
void job_submit (List * job_q, FuncPtr func, void *args, int args_size);

void list_insert (List * l, node * nnew);

/**job_queue_init:
 ** initiate the queue and thread pool of size "pool_size" returns a pointer
 ** to the initiated queue 
 */
List *job_queue_init (int pool_size);

#ifdef __cplusplus
}
#endif

#endif

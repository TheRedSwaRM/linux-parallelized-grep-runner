#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/syscall.h>
#include <sys/types.h>

// TODO: Run grep through this function
// TODO: access file system directory
// TODO: task queue
// DONE: Pass path and search_string as parameters in threads
struct thread_arguments{
    int worker_ID;
    char * search_string;
};

typedef struct __node_t {
    int value;
    struct __node_t *next;
} t_node;

typedef struct t_queue{
    struct node * head;
    struct node * tail;
    pthread_mutex_t head_lock, tail_lock;
} t_queue;

void Task_Queue_Init(){

}

t_queue *task_queue = NULL;
char buffer[250];

int ENQUEUER(int ID){
    printf("[%d] ENQUEUE READY\n", ID);
    // t_node *tmp = malloc(sizeof(t_node));
    // assert(tmp != NULL);
    // tmp->value = value;
    // tmp->next = NULL;

    // pthread_mutex_lock(&q->tail_lock);
    // task_queue->tail->next = tmp;
    // task_queue->tail = tmp;
    // pthread_mutex_unlock(&q->tail_lock);
}

char * DEQUEUER(int ID){
    printf("[%d] DEQUEUE READY\n", ID);
    // pthread_mutex_lock(&q->head_lock);
    // t_node *tmp = q->head;
    // t_node *new_head = tmp->next;
    // if (new_head == NULL) {
    //     pthread_mutex_unlock(&q->head_lock);
    //     return -1; // queue was empty
    // }
    // *value = new_head->value;
    // q->head = new_head;
    // pthread_mutex_unlock(&q->head_lock);
    // free(tmp);
    // return 0;
}

void * WORKER(void* args){

    struct thread_arguments * some_arguments = (struct thread_arguments *)args;
    // printf("thread works!; workerID: %d; search_string: %s\n", some_arguments->worker_ID, some_arguments->search_string);

    // Worker's Job:
    // run dequeue, return path, if path is directory, run enqueue, if path is file, run grep
    // TODO: FIGURE OUT STOPPING CONDITION
    printf("[%d] WORKER ON DUTY\n", some_arguments->worker_ID);
    DEQUEUER(some_arguments->worker_ID);
    
    ENQUEUER(some_arguments->worker_ID);
    // system();
}

int main(int argc, char *argv[]){ // {command} {workers N} {rootpath} {search string}
    if (argc != 4) return -1; //it should return something since argc should be 4
    int workers = atoi(argv[1]); // convert argv[1] into an integer 
    // INITIALIZE SEMAPHORES OR COND_VARS OR LOCKS
    pthread_t threads[workers];
    struct thread_arguments * const some_arg = malloc(workers*sizeof(struct thread_arguments));
    for(int i = 0; i<workers; i++){
        some_arg[i].worker_ID = i;
        some_arg[i].search_string = argv[3];
    }
    
    // TODO: ENQUEUE rootpath here!
    printf("argc output is: %d\n", argc);

    printf("cwd: %s\n", getcwd(buffer, sizeof(buffer)));

    // check if rootpath is absolute 
    // DONE: create number of threads based on variable workers
    for(int i = 0; i < workers; i++){
        pthread_create(&threads[i], NULL, WORKER, &some_arg[i]);
    }

    for(int i = 0; i < workers; i++){
        pthread_join(threads[i], NULL);
    }
    
    free(some_arg);
    return 0;
}
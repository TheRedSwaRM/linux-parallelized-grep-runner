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

char buffer[250];

int ENQUEUER(int ID, char * current_dir){
    printf("[%d] ENQUEUE READY\n",some_arguments->worker_ID);
}

int DEQUEUER(int ID, char * search){
    printf("[%d] DEQUEUE READY\n",some_arguments->worker_ID);
}

void * WORKER(void* args){

    struct thread_arguments * some_arguments = (struct thread_arguments *)args;
    printf("thread works!; workerID: %d; search_string: %s\n", some_arguments->worker_ID, some_arguments->search_string);

    printf("[%d] WORKER ON DUTY\n", some_arguments->worker_ID);
    DEQUEUER(some_arguments->worker_ID, some_arguments->search_string);
    ENQUEUER(some_arguments->worker_ID, taskqueue)
    printf("[%d] ENQUEUE READY\n",some_arguments->worker_ID);
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
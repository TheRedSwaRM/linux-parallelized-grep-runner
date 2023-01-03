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
// DONE: Pass path and search_string as parameters in threads
struct thread_arguments{
    int worker_ID;
    char * path;
    char * search_string;
};

void * parallel_grep(void* args){

    struct thread_arguments * some_arguments = (struct thread_arguments *)args;
    printf("thread works!; workerID: %d; path: %s; search_string: %s\n", some_arguments->worker_ID, some_arguments->path,some_arguments->search_string);

    // TODO: check if path is absolute or relative
    printf("path[0]: %c\n", some_arguments->path[0]);
    if (some_arguments->path[0]!='/'){ //root path is NOT absolute!
        continue;
    }
    else{ //root path is absolute!
        continue;
    }
    // system();
}

int main(int argc, char *argv[]){ // {command} {workers N} {rootpath} {search string}
    if (argc != 4) return -1; //it should return something since argc should be 4
    int workers = atoi(argv[1]); // convert argv[1] into an integer 
    pthread_t threads[workers];
    struct thread_arguments * const some_arg = malloc(workers*sizeof(struct thread_arguments));
    for(int i = 0; i<workers; i++){
        some_arg[i].worker_ID = i;
        some_arg[i].path = argv[2];
        some_arg[i].search_string = argv[3];
    }
    
    printf("argc output is: %d\n", argc);

    // Sanity Test: testing/checking the arguments passed in the multithreaded grep
    // argv[1]: Number of workers N
    // argv[2]: Root of directory tree to search (hereby referred to as rootpath)
    // argv[3]: Search string to be used by grep
    // printf("workers: %d\n", workers);
    // printf("path: %s\n", some_arg->path);
    // printf("search_string: %s\n", some_arg->search_string);

    // printf("working directory: %s\n", getcwd(buffer, sizeof(buffer)));

    // check if rootpath is absolute 
    // DONE: create number of threads based on variable workers
    for(int i = 0; i < workers; i++){
        pthread_create(&threads[i], NULL, parallel_grep, &some_arg[i]);
    }

    for(int i = 0; i < workers; i++){
        pthread_join(threads[i], NULL);
    }
    
    free(some_arg);
    return 0;
}
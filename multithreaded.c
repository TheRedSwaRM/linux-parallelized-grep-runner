#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <assert.h>

// TODO: Run grep
// TODO: access file system directory
// DONE: task queue
// DONE: Pass path and search_string as parameters in threads
struct thread_arguments{
    int worker_ID;
    char * search_string;
};

typedef struct t_node {
    char * path;
    struct t_node *next;
} t_node;

typedef struct t_queue{
    struct t_node * head;
    struct t_node * tail;
    pthread_mutex_t head_lock, tail_lock;
} t_queue;

t_queue *task_queue = NULL;
char buffer[250];
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int WORKER_WAIT_COUNTER = 0;
int no_more_jobs = 0;

void Task_Queue_Init(t_queue *q){
    t_node *tmp = malloc(sizeof(t_node));
    tmp->next = NULL;
    q->head = q->tail = tmp;
    pthread_mutex_init(&q->head_lock, NULL);
    pthread_mutex_init(&q->tail_lock, NULL);
}

int ENQUEUER(int ID, char * some_path){
    // printf("[%d] ENQUEUE READY\n", ID);
    if(some_path[0]!='/'){// if relative path, print absolute version
        // printf("yes\n");
        printf("[%d] ENQUEUE %s/%s\n", ID, buffer, some_path);
    }
    else{ //absolute path
        printf("[%d] ENQUEUE %s\n", ID, some_path);
    }
    pthread_mutex_lock(&task_queue->tail_lock);
    t_node *tmp = malloc(sizeof(t_node));
    
    assert(tmp!=NULL);
    tmp->path = some_path;
    tmp->next = NULL;

    if (task_queue->head==NULL){
        task_queue->head = tmp;
        task_queue->tail = tmp;
    }
    
    else{
        task_queue->tail->next = tmp;
        task_queue->tail = tmp;
    }
    // printf("[%d] nakapag-enqueue na dito!\n", ID);
    // printf("[%d] cond_signal success\n", ID);
    pthread_mutex_unlock(&task_queue->tail_lock);
    // pthread_cond_signal(&cond);
    // printf("[%d] tail unlock success\n", ID);
    return 0;
}

char * DEQUEUER(int ID){
    // printf("[%d] DEQUEUE READY\n", ID);
    redequeue:
    pthread_mutex_lock(&task_queue->head_lock);
    // printf("[%d] lock success\n", ID);
    if(task_queue->head==NULL){
        // printf("[%d] NULL\n", ID);
        
        pthread_mutex_unlock(&task_queue->head_lock);
        return NULL;
    }
    // printf("[%d] wait success\n", ID);
    t_node *tmp = task_queue->head;
    // printf("[%d] umabot dito\n", ID);
    // printf("[%d] task_queue->head from DEQUEUE: %p\n", ID, tmp);
    char * some_path = malloc(sizeof(char *)*(strlen(tmp->path)+1));
    // printf("[%d] some_path from DEQUEUE: %p\n", ID, &some_path);
    // printf("[%d] did segfault here?\n", ID);
    t_node *new_head = tmp->next;
    // printf("[%d] or here?\n", ID);
    strcpy(some_path, tmp->path);
    if(some_path[0]!=47){// if relative path, print absolute version
        printf("[%d] DIR %s/%s\n", ID, buffer, some_path);
    }
    else{ //absolute path
        printf("[%d] DIR %s\n", ID, some_path);
    }
    // printf("[%d] or maybe here?\n", ID);
    task_queue->head = new_head;
    // printf("[%d] queue adjustment success\n", ID);
    pthread_mutex_unlock(&task_queue->head_lock);
    // printf("[%d] head unlock success\n", ID);
    free(tmp->path);
    free(tmp);
    return some_path;
}

void * WORKER(void* args){

    struct thread_arguments * some_arguments = (struct thread_arguments *)args;
    // printf("thread works!; workerID: %d; search_string: %s\n", some_arguments->worker_ID, some_arguments->search_string);

    // Worker's Job:
    // run dequeue, return path, if path is directory, run enqueue, if path is file, run grep
    // DONE: FIGURE OUT STOPPING CONDITION - If N-1 workers are waiting, and the next worker is trying to wait as well
    // that would mean that there are no more ENQUEUEs needed to be done, and hence, 
    
    while(1){
        char * path = DEQUEUER(some_arguments->worker_ID);
        // printf("[%d] Path accessed by worker: %s\n", some_arguments->worker_ID, path);
        // DO TASKS HERE
        if (path == NULL){continue;}
        DIR * current_working_dir = opendir(path);
        // printf("[%d] Path accessed by worker: %s\n", some_arguments->worker_ID, path);


        while(1){
            if (current_working_dir == NULL){
                break;
            }
            // printf("[%d] kailangan 2x+ ito magprint\n",some_arguments->worker_ID);
            struct dirent * entry = readdir(current_working_dir);
            if (entry == NULL){
                // printf("[%d] what\n", some_arguments->worker_ID);
                break;
            }
            // printf("[%d] umabot ba dito yung isa\n", some_arguments->worker_ID);
            switch(entry->d_type){
                case DT_REG: {
                    // printf("FOUND FILE!\n");
                    // grep
                    char * grep_command = malloc(sizeof(char*)*(strlen("grep ")+strlen(path)+2+strlen(some_arguments->search_string)+3+strlen(entry->d_name)+2));
                    strcpy(grep_command, "grep '");
                    strcat(grep_command, some_arguments->search_string);
                    strcat(grep_command, "' '");
                    strcat(grep_command, path);
                    strcat(grep_command, "/");
                    strcat(grep_command, entry->d_name);
                    strcat(grep_command, "' ");
                    strcat(grep_command, "> /dev/null");

                    // printf("grep_command: %s\n", grep_command);
                    int return_value = system(grep_command); //TODO: REDIRECT STANDARD OUTPUT AND STANDARD ERROR
                    // printf("return value of grep: %d\n", return_value);
                    if (return_value!=0){ // NOT FOUND // DONE: CHECK IF PATH IS ABSOLUTE OR RELATIVE
                        if(path[0]!=47){// if relative path, print absolute version
                            printf("[%d] ABSENT %s/%s/%s\n", some_arguments->worker_ID, buffer, path, entry->d_name);
                        }
                        else{ //absolute path
                            printf("[%d] ABSENT %s/%s\n", some_arguments->worker_ID, path, entry->d_name);
                        }
                        
                    }
                    else{ // FOUND
                        if(path[0]!=47){// if relative path, print absolute version
                            printf("[%d] PRESENT %s/%s/%s\n", some_arguments->worker_ID, buffer, path, entry->d_name);
                        }
                        else{ //absolute path
                            printf("[%d] PRESENT %s/%s\n", some_arguments->worker_ID, path, entry->d_name);
                        }
                    }
                    // printf("[%d] tapos na ang grepping\n", some_arguments->worker_ID);
                    free(grep_command);
                    goto found_reg; 
                }
                case DT_DIR: {
                    if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")){// ENQUEUE PATH IN TASK QUEUE EXCEPT IF . OR ..
                        continue;
                    } 
                    char *tmp = malloc(sizeof(char*) * (strlen(path) + strlen(entry->d_name) + 1)); 
                    strcpy(tmp, path);
                    strcat(tmp, "/");
                    strcat(tmp, entry->d_name);
                    // printf("strlen of tmp: %ld\n", strlen(tmp));
                    ENQUEUER(some_arguments->worker_ID, tmp);

                    goto found_dir;
                }
                default: break;
            }
            found_dir:
            
            found_reg:
            // printf("[%d] nakaabot dito!\n", some_arguments->worker_ID);
        }   
        // printf("WORKER WAIT COUNTER: %d\n", WORKER_WAIT_COUNTER);
        closedir(current_working_dir);
    }
    
    return NULL;
}

int main(int argc, char *argv[]){ // {command} {workers N} {rootpath} {search string}
    if (argc != 4) return -1; //it should return something since argc should be 4
    int workers = atoi(argv[1]); // convert argv[1] into an integer 
    // DONE: INITIALIZE SEMAPHORES OR COND_VARS OR LOCKS

    task_queue = malloc(sizeof(t_queue));
    Task_Queue_Init(task_queue);

    pthread_t threads[workers];
    struct thread_arguments * const some_arg = malloc(workers*sizeof(struct thread_arguments));
    for(int i = 0; i<workers; i++){
        some_arg[i].worker_ID = i;
        some_arg[i].search_string = argv[3];
    }
    
    // --- Section: ENQUEUE initial rootpath here!
    char * temp_path = malloc(sizeof(char*)*(strlen(argv[2])+1));
    strcpy(temp_path, argv[2]);
    task_queue->head->path = temp_path;
    // ---


    printf("cwd: %s\n", getcwd(buffer, sizeof(buffer)));

    // --- Section: create number of threads based on variable workers
    for(int i = 0; i < workers; i++){
        pthread_create(&threads[i], NULL, WORKER, &some_arg[i]);
    }

    for(int i = 0; i < workers; i++){
        pthread_join(threads[i], NULL);
    }
    // ---
    free(task_queue);
    free(some_arg);
    return 0;
}
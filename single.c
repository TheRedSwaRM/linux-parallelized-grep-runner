#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/syscall.h>
#include <sys/types.h>

struct thread_arguments{
    int worker_ID;
    char * path;
    char * search_string;
};

// INSERT HERE: Task Queue
typedef struct node{
    char * path;
    struct node * next;
} t_node;


typedef struct t_queue{
    struct node * head;
    struct node * tail;
    //pthread_mutex_t head_lock, tail_lock;
} t_queue;

void task_queue_init(t_queue *q){
    t_node *tmp = malloc(sizeof(t_node));
    tmp->next = NULL;
    q->head = q->tail = tmp;
    // pthread_mutex_init(&q->head_lock, NULL);
    // pthread_mutex_init(&q->tail_lock, NULL);
}

t_queue *task_queue = NULL;
char buffer[250];

int ENQUEUER(int ID, char * current_dir){
    // printf("current_dir is: %s\n", current_dir);
    if(current_dir[0]!='/'){// if relative path, print absolute version
        // printf("yes\n");
        printf("[%d] ENQUEUE %s/%s\n", ID, buffer, current_dir);
    }
    else{ //absolute path
        printf("[%d] ENQUEUE %s\n", ID, current_dir);
    }

    int enQd = 0;
    t_node *tmp = malloc(sizeof(t_node));
    tmp->path = current_dir;
    tmp->next = NULL;
    // printf("new enqueued path: %s\n", tmp->path);
    // pthread_mutex_lock(&q->tail_lock);
    // check if task queue is empty
    if (task_queue->head==NULL){
        task_queue->head = tmp;
        task_queue->tail = tmp;
    }
    else{
        task_queue->tail->next = tmp;
        task_queue->tail = tmp;
    }
    // pthread_mutex_unlock(&q->tail_lock);
    // printf("new enqueued path (checker): %s\n", task_queue->head->path);
    // printf("new enqueued path (checker): %s\n", task_queue->tail->path);

    // we start the thingy in task queue
    // child object is a directory, enqueue path
    // if we enqueue, return 1, else this would imply that directory only has files, hence, return 0 because no more to enqueue
    return 0;
}

int DEQUEUER(int ID, char * search){
    // printf("DEQUEUER works!; workerID: %d; path: %s; search_string: %s\n", ID, current_dir, search);
    t_node *temp = task_queue->head;
    if (temp == NULL){
        return -1;
    }
    // printf("cwd: %s\n", buffer);
    char * some_path = temp->path;
    t_node *new_head = temp->next;
    task_queue->head = new_head;
    // printf("tempcheck: %s\n", temp->path);
    // printf("it reaches here!\n");
    
    // printf("path check: %s\n", some_path);
    // printf("next check: %s\n", task_queue->head->path);
    // exit(1);
    if(some_path[0]!=47){// if relative path, print absolute version
        printf("[%d] DIR %s/%s\n", ID, buffer, some_path);
    }
    else{ //absolute path
        printf("[%d] DIR %s\n", ID, some_path);
    }
    
    DIR * current_working_dir = opendir(some_path);

    while(1){
        if (current_working_dir == NULL){
            break;
        }
        struct dirent * entry = readdir(current_working_dir);
        if (entry == NULL){
            break;
        }
        
        switch(entry->d_type){
            case DT_REG: {
                // printf("FOUND FILE!\n");
                // grep
                char * grep_command = malloc(sizeof(char*)*(strlen("grep ")+strlen(some_path)+2+strlen(search)+3+strlen(entry->d_name)+2));
                strcpy(grep_command, "grep '");
                strcat(grep_command, search);
                strcat(grep_command, "' '");
                strcat(grep_command, some_path);
                strcat(grep_command, "/");
                strcat(grep_command, entry->d_name);
                strcat(grep_command, "' ");
                strcat(grep_command, "> /dev/null");

                // printf("grep_command: %s\n", grep_command);
                int return_value = system(grep_command); //TODO: REDIRECT STANDARD OUTPUT AND STANDARD ERROR
                // printf("return value of grep: %d\n", return_value);
                if (return_value!=0){ // NOT FOUND // DONE: CHECK IF PATH IS ABSOLUTE OR RELATIVE
                    if(some_path[0]!=47){// if relative path, print absolute version
                        printf("[%d] ABSENT %s/%s/%s\n", ID, buffer, some_path, entry->d_name);
                    }
                    else{ //absolute path
                        printf("[%d] ABSENT %s/%s\n", ID, some_path, entry->d_name);
                    }
                    
                }
                else{ // FOUND
                    if(some_path[0]!=47){// if relative path, print absolute version
                        printf("[%d] PRESENT %s/%s/%s\n", ID, buffer, some_path, entry->d_name);
                    }
                    else{ //absolute path
                        printf("[%d] PRESENT %s/%s\n", ID, some_path, entry->d_name);
                    }
                }

                free(grep_command);
                goto found_reg; 
            }
            case DT_DIR: {
                if (!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")){// ENQUEUE PATH IN TASK QUEUE EXCEPT IF . OR ..
                    continue;
                } 
                char *tmp = malloc(sizeof(char*) * (strlen(some_path) + strlen(entry->d_name) + 1)); 
                strcpy(tmp, some_path);
                strcat(tmp, "/");
                strcat(tmp, entry->d_name);
                // printf("strlen of tmp: %ld\n", strlen(tmp));
                ENQUEUER(ID, tmp);

                goto found_dir;
            }
            default: break;
        }

        found_dir:
        
        found_reg:
    }
    // free(some_path);
    free(temp->path);
    free(temp);
    closedir(current_working_dir);
    return 0;
}

void WORKER(void* args){
    int can_still_enqueue = 1;
    struct thread_arguments * some_arguments = (struct thread_arguments *)args;
        // deal with non-absolute path!! Which is faster or easier on the memory!! I think!!
    // printf("check worker before loop\n");
    while(1){
        // printf("while loop works!\n");
        // printf("current task queue path: %s\n", task_queue->head->path);
        if(task_queue->head != NULL){ // DEQUEUE TIME
            // printf("yes\n");
            int no = DEQUEUER(some_arguments->worker_ID, some_arguments->search_string);
            // printf("no: %d\n", no);
        }
        else{
            return;
        }
    }
}

int main(int argc, char *argv[]){ // {command} {workers N} {rootpath} {search string}
    if (argc != 4) return -1; //it should return something since argc should be 4
    int workers = 1; // convert argv[1] into an integer
    struct thread_arguments * const some_arg = malloc(workers*sizeof(struct thread_arguments));

    task_queue = malloc(sizeof(t_queue));
    task_queue_init(task_queue);

    getcwd(buffer, sizeof(buffer));
    
    // printf("argc output is: %d\n", argc);
    
    some_arg[0].worker_ID = 0;          // argv[1]: Number of workers N (for single.c, it's just 1)
    some_arg[0].path = argv[2];         // argv[2]: Root of directory tree to search (hereby referred to as rootpath)
    some_arg[0].search_string = argv[3];// argv[3]: Search string to be used by grep

    char * temp_path = malloc(sizeof(char*)*(strlen(some_arg[0].path)+1));
    strcpy(temp_path, some_arg[0].path);
    
    task_queue->head->path = temp_path;
    // printf("check\n");
    WORKER(some_arg);

    // printf("working directory: %s\n", getcwd(buffer, sizeof(buffer)));
    // while(1){
    //     if (task_queue->head == NULL){
    //         break;
    //     }
    //     task_queue->head->path;
    // }
    free(task_queue);
    free(some_arg);
    return 0;
}
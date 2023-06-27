#include "kernel/types.h"
#include "user/user.h"

void run(int listenfd){
    int my_num = 0;// 这个进程的质数，为读到的第一个数
    int pass_num = 0;
    int forked = 0;
    int pipes[2];
    while (1)
    {
        int read_len = read(listenfd, &pass_num, sizeof(int));
        if(read_len==0){
            close(listenfd);
            if(forked){
                close(pipes[1]);
                int child_pid = getpid();
                wait(&child_pid);
            }
            exit(0);
        }
        if(pass_num!=0&&my_num==0){
            my_num = pass_num;
            printf("prime %d\n", my_num);
        }
        // 开个子进程发出去
        if(pass_num%my_num!=0){
            if(!forked){
                pipe(pipes);
                forked = 1;
                int ret = fork();
                if(ret==0){
                    close(pipes[1]);
                    close(listenfd);
                    run(pipes[0]);
                }
                else
                {
                    close(pipes[0]);
                }
            }
            write(pipes[1], &pass_num, sizeof(int));
        }
    }
}

int main(int argc, char *argv[]){
    int pipes[2];
    pipe(pipes);
    for (int i = 2; i <= 35;i++){
        int ret = write(pipes[1], &i, sizeof(i));
        if(ret==0){
            exit(-1);
        }
    }
    close(pipes[1]);
    run(pipes[0]);
    exit(0);
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

#define NUM_CHILDREN 4
#define SEM_NAME "/sync_semaphore"

pid_t children[NUM_CHILDREN];
sem_t *sync_sem;

void handle_signal(int sig)
{
    printf("Parent received confirmation signal from child.\n");
}

void child_task(int id)
{
    sem_wait(sync_sem);
    printf("Child %d is starting its task.\n", id);
    sleep(2);
    printf("Child %d has completed its task.\n", id);
    kill(getppid(), SIGUSR1);

    exit(0);
}

int main()
{
    sync_sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 0);
    if (sync_sem == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);

    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            child_task(i + 1);
        }
        else if (pid > 0)
        {
            children[i] = pid;
        }
        else
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        sem_post(sync_sem);
    }

    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        waitpid(children[i], NULL, 0);
    }

    sem_close(sync_sem);
    sem_unlink(SEM_NAME);

    printf("Parent process is terminating.\n");
    return 0;
}

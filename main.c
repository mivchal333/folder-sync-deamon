#include <sys/types.h>
#include "funkcje.h"


int main(int argc, char *argv[]) {
    /* Our process ID and Session ID */
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    /* Change the file mode mask */
    umask(0);
    /* Open logs */
    openlog("sync-demon", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    if (argc < 5) {
        syslog(LOG_ERR, "Tried to run without required parameters");
        printf("Please run with at least two required parameters");
        exit(EXIT_FAILURE);
    }

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "Error related to child process");
        exit(EXIT_FAILURE);
    }
    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        syslog(LOG_ERR, "Chdir to root folder failed");
        exit(EXIT_FAILURE);
    }


    char *in, *out;
    int c;
    int sleepTime = 300;
    int switchSize;
    char *path1 = NULL;
    char *path2 = NULL;
    while ((c = getopt(argc, argv, "f:t:s:m")) != -1) {
        switch (c) {
            case 'f':
                in = optarg;
                    if (isCatalog(in)) {
                        path1 = optarg;
                    } else {
                        syslog(LOG_ERR, "From path is not a folder. Exiting");
                        printf("From path must specify folder");
                        exit(EXIT_FAILURE);
                    }
                break;
            case 't':
                out = optarg;
                if(isCatalog(out)){
                    path2 = optarg;
                } else{
                    syslog(LOG_ERR, "To path is not a folder. Exiting");
                    printf("To path must specify folder");
                    exit(EXIT_FAILURE);
                }
                break;
            case 's':
                sleepTime = atoi(optarg);
                break;
            case 'm':
                switchSize = atoi(optarg);
                break;
        }
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    syslog(LOG_DEBUG, "DEMON CONFIGURED");

   signal(SIGUSR1, WakeUpSignalHandler);

    while (1) {
        Usuwanie(path2, path1, path2);
        PrzegladanieFolderu(path1, path1, path2, switchSize);
        syslog(LOG_INFO, "Demon przeszedl w stan uspienia");
        if ((sleep(sleepTime)) == 0)
            syslog(LOG_INFO, "Demon sie obudzil");
    }
    closelog();
    exit(EXIT_SUCCESS);
    return 0;
}

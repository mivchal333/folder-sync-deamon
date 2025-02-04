#include <sys/types.h>
#include "sync_functions.h"

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
    int copySwitchSize;
    char *inPath = NULL;
    char *toPath = NULL;
    while ((c = getopt(argc, argv, "f:t:s:m")) != -1) {
        switch (c) {
            case 'f':
                in = optarg;
                if (isCatalog(in)) {
                    inPath = optarg;
                } else {
                    syslog(LOG_ERR, "From path is not a folder. Exiting");
                    printf("From path must specify folder");
                    exit(EXIT_FAILURE);
                }
                break;
            case 't':
                out = optarg;
                if (isCatalog(out)) {
                    toPath = optarg;
                } else {
                    syslog(LOG_ERR, "To path is not a folder. Exiting");
                    printf("To path must specify folder");
                    exit(EXIT_FAILURE);
                }
                break;
            case 's':
                sleepTime = atoi(optarg);
                break;
            case 'm':
                copySwitchSize = atoi(optarg);
                syslog(LOG_INFO, "argument m %d", copySwitchSize);
                break;
        }
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    signal(SIGUSR1, wakeUpSignalHandler);

    syslog(LOG_DEBUG, "Daemon started!");
    while (1) {
        delete(inPath, toPath);
        scanFolder(inPath, toPath, copySwitchSize);
        syslog(LOG_INFO, "Daemon goes sleep");
        if ((sleep(sleepTime)) == 0)
            syslog(LOG_INFO, "Daemon woke up");
    }
}


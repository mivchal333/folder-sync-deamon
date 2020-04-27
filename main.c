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
    int argument;
    int sleepTime = 300;
    int switchSize;
    struct stat s;
    char *path1 = NULL, *path2 = NULL;
    while ((argument = getopt(argc, argv, "i:o:s")) != -1) {
        switch (argument) {
            case 'i':
                in = optarg;
                if (stat(in, &s) == 0) {
                    if (!(s.st_mode & S_IFDIR)) {
                        printf("-i: Podany argument nie jest folderem");
                        syslog(LOG_ERR, "Podany argument nie jest folderem");
                        exit(EXIT_FAILURE);

                    } else {
                        path1 = optarg;
                    }
                }
                break;

            case 'o':
                out = optarg;
                if (stat(out, &s) == 0) {
                    if (s.st_mode & S_IFDIR) //sciezka jest katalogiem
                    {
                        path2 = optarg;
                    } else //sciezka nie jest katalogiem, wywal blad
                    {
                        printf("-o: Podany argument nie jest folderem");
                        syslog(LOG_ERR, "Podany argument nie jest folderem");
                        exit(EXIT_FAILURE);
                    }
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
    syslog(LOG_INFO, "Demon synchronizujacy dwa katalogi");
    if (signal(SIGUSR1, Logowanie) == SIG_ERR) {
        syslog(LOG_ERR, "Blad sygnalu!");
        exit(EXIT_FAILURE);
    }


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

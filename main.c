#include <sys/types.h>
#include "funkcje.h"


int main(int argc, char * argv[])
{
    openlog("PROJEKT", LOG_PID|LOG_CONS, LOG_USER);
    if(argc<5)
    {
        printf("Zbyt mala liczba argumentow wejsciowych!");
        syslog(LOG_ERR, "Zbyt mala liczba argumentow wejsciowych!");
        exit(EXIT_FAILURE);
    }
    pid_t pid, sid;

    pid = fork();

    if(pid<0)
    {
        syslog(LOG_ERR, "Nieprawidlowy identyfikator procesu child!");
        exit(EXIT_FAILURE);
    }
    if(pid>0)
    {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();
    if(sid<0)
    {
        syslog(LOG_ERR, "Blad z SessionID!");
        exit(EXIT_FAILURE);
    }
    if((chdir("/")) < 0)
    {
        syslog(LOG_ERR, "Problem ze zmiana katalogu roboczego!");
        exit(EXIT_FAILURE);
    }


    int wybor = 0, rozmiar = 50;
    char *in, *out;
    int spij = 5*60; //domyslna dlugosc snu dla demona
    struct stat s;
    char * sciezka_folderu1=NULL;
    char * sciezka_folderu2=NULL;
    while((wybor = getopt(argc, argv,"s:i:o:m:r")) != -1)
    {
        switch(wybor)
        {
        case 's': //argument z nowa wartoscia spania demona
            spij = atoi(optarg);
            break;

        case 'i':
            in = optarg;
            if(stat(in, &s) == 0)
            {
                if(s.st_mode & S_IFDIR) //sciezka jest katalogiem
                {
                    sciezka_folderu1 = optarg;
                }
                else //sciezka nie jest katalogiem, wywal blad
                {
                    printf("-i: Podany argument nie jest folderem");
                    syslog(LOG_ERR, "Podany argument nie jest folderem");
                    exit(EXIT_FAILURE);
                }
            }
            break;

        case 'o':
            out = optarg;
            if(stat(out, &s) == 0)
            {
                if(s.st_mode & S_IFDIR) //sciezka jest katalogiem
                {
                    sciezka_folderu2 = optarg;
                }
                else //sciezka nie jest katalogiem, wywal blad
                {
                    printf("-o: Podany argument nie jest folderem");
                    syslog(LOG_ERR, "Podany argument nie jest folderem");
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case 'm':
            rozmiar = atoi(optarg);
            break;
        }
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
	syslog(LOG_INFO, "Demon synchronizujacy dwa katalogi");
    if(signal(SIGUSR1, Logowanie)==SIG_ERR)
    {
        syslog(LOG_ERR, "Blad sygnalu!");
        exit(EXIT_FAILURE);
    }
    

    while(1)
    {
        Usuwanie(sciezka_folderu2,sciezka_folderu1,sciezka_folderu2);
        PrzegladanieFolderu(sciezka_folderu1,sciezka_folderu1,sciezka_folderu2,rozmiar);
        syslog(LOG_INFO, "Demon przeszedl w stan uspienia");
        if((sleep(spij))==0)
            syslog(LOG_INFO, "Demon sie obudzil");
    }
    closelog();
    exit(EXIT_SUCCESS);
    return 0;
}

#ifndef FUNKCJE_H_INCLUDED
#define FUNKCJE_H_INCLUDED
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <utime.h>
#include <syslog.h>
#include <fcntl.h>

off_t getFileSize(const char *in);
time_t getFileModificationTime(char* in);
mode_t getFilePermissions(char *in);
void changeParameters(char* in, char *out);
char *replaceCatalog2(char * path, char* catalogOnePath, char* catalogTwoPath);
char *replaceCatalog1(char * path, char* catalogOnePath, char* catalogTwoPath);
char *addToPath(char* path, char *add);
bool check(char * path, char* catalogOnePath, char* catalogTwoPath);
void delete(char * catalogPathName, char* catalogPathOne, char* catalogPathTwo);
void copy(char *in, char *out); //---
void mapping_copy(char *in, char *out); //---
void openfiles(char *in, char *out, int *inFile, int *outFile); //---
void closefiles(char *in, char *out, int *inFile, int *outFile, int opc); //---
void scanFolder(char * pathName, char* catalogPathOne, char* catalogPathTwo, int switchSize);
void wakeUpSignalHandler();
void syslogCom(int in, char *file);
bool isCatalog(char * path);
bool isFile(const struct dirent *file);
bool isLargeFile(int switchSize, const char *newPath);
#endif

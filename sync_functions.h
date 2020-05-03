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

int getFileSize(const char *filePath);

int getFileModificationTime(char *filePath);

mode_t getFilePermissions(char *in);

void updateFileModTimeAndPermissions(char *inFilePath, char *outFilePath);

char *addFileNameToPath(char *path, char *fileName);

bool isFileNeedSync(char *filename, char *inPath, char *outPath);

void delete(char *inPath, char *outPath);

void copy(char *in, char *out);

void mapping_copy(char *in, char *out);

void openfiles(char *in, char *out, int *inFile, int *outFile);

void closefiles(char *in, char *out, const int *inFile, const int *outFile, int opc);

void scanFolder(char *pathName, char *catalogPathTwo, int switchSize);

void wakeUpSignalHandler();

void syslogCom(int in, char *file);

bool isCatalog(char *path);

bool isFile(const struct dirent *file);

bool isLargeFile(int switchSize, const char *filePath);

void copyFile(char *inPath, char *outPath, int switchSize, char *tempPath);

bool isFileExists(char *filePath);

#endif

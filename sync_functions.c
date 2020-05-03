#include "sync_functions.h"

int getFileSize(const char *filePath) {
    struct stat size;
    if (stat(filePath, &size) == -1) {
        exit(EXIT_FAILURE);
    }
    return (int) size.st_size;
}

int getFileModificationTime(char *filePath) {
    struct stat attr;
    if (stat(filePath, &attr) == -1) {
        syslogCom(1, filePath);
        exit(EXIT_FAILURE);
    }
    return (int) attr.st_mtime;
}

mode_t getFilePermissions(char *in) {
    struct stat attr;
    if (stat(in, &attr) == -1) {
        syslogCom(2, in);
        exit(EXIT_FAILURE);
    }
    return attr.st_mode;
}

void updateFileModTimeAndPermissions(char *inFilePath, char *outFilePath) {
    struct utimbuf times;
    times.actime = 0;
    times.modtime = getFileModificationTime(inFilePath);
    if (utime(outFilePath, &times) == -1) {
        syslogCom(3, outFilePath);
        exit(EXIT_FAILURE);
    }
    mode_t inFileMode = getFilePermissions(inFilePath);
    if (chmod(outFilePath, inFileMode) == -1) {
        syslogCom(4, outFilePath);
        exit(EXIT_FAILURE);
    }
}

char *replaceCatalog1(char *path, char *catalogOnePath, char *catalogTwoPath) {
    char *fullPath = path + strlen(catalogOnePath);
    char *newPath = malloc(strlen(catalogTwoPath) + strlen(fullPath) + 1);
    strcpy(newPath, catalogTwoPath);
    strcat(newPath, fullPath);
    return newPath;
}

char *addFileNameToPath(char *path, char *fileName) {
    char *result = malloc(strlen(path) + 2 + strlen(fileName));
    strcpy(result, path);
    strcat(result, "/");
    strcat(result, fileName);
    return result;
}

bool isFileNeedSync(char *filename, char *inPath, char *outPath) {
    char *outFilePath = addFileNameToPath(outPath, filename);

    if (isFileExists(outFilePath)) {
        syslog(LOG_INFO, "File %s exist in out folder", filename);

        char *inFilePath = addFileNameToPath(inPath, filename);

        int fromFileModDate = getFileModificationTime(inFilePath);
        int toFileModDate = getFileModificationTime(outFilePath);

        return fromFileModDate != toFileModDate;
    } else {
        syslog(LOG_INFO, "File %s does not exist in out folder", filename);
        return true;
    }
}


void delete(char *inPath, char *outPath) {
    struct dirent *entryPointer;
    DIR *dir;
    dir = opendir(outPath);
    while ((entryPointer = readdir(dir))) {
        if (isFile(entryPointer)) {
            char *fileNameInOutCatalog = entryPointer->d_name;
            syslog(LOG_INFO, "Checking if file %s exist in from catalog", fileNameInOutCatalog);

            char *fileToCheckPath = addFileNameToPath(inPath, fileNameInOutCatalog);
            if (!isFileExists(fileToCheckPath)) {
                syslogCom(5, fileToCheckPath);

                char *fileToRemovePath = addFileNameToPath(outPath, fileNameInOutCatalog);
                syslog(LOG_INFO, "Removing file %s", fileToRemovePath);
                remove(fileToRemovePath);
            }
        }
    }
    closedir(dir);
}

void copy(char *in, char *out) {
    char buffor[16];
    int in_file, out_file;
    openfiles(in, out, &in_file, &out_file);

    int r_in, r_out;

    while ((r_in = read(in_file, buffor, sizeof(buffor))) > 0) {
        r_out = write(out_file, buffor, (ssize_t) r_in);
        if (r_out != r_in) {
            syslog(LOG_ERR, "Error while copying file %s to %s", in, out);
            exit(EXIT_FAILURE);
        }
    }
    closefiles(in, out, &in_file, &out_file, 1);

}

void mapping_copy(char *in, char *out) {
    int size = getFileSize(in);
    int in_file, out_file;
    openfiles(in, out, &in_file, &out_file);

    char *map = (char *) mmap(0, size, PROT_READ, MAP_SHARED | MAP_FILE, in_file, 0);

    write(out_file, map, size);
    munmap(map, size);

    closefiles(in, out, &in_file, &out_file, 0);
}

void openfiles(char *in, char *out, int *inFile, int *outFile) {
    if ((*inFile = open(in, O_RDONLY)) == -1 || (*outFile = open(out, O_CREAT | O_WRONLY | O_TRUNC, 0644)) == -1) {
        syslogCom(6, in);
        exit(EXIT_FAILURE);
    }
}


void closefiles(char *in, char *out, const int *inFile, const int *outFile, int opc) {
    close(*inFile);
    close(*outFile);
    updateFileModTimeAndPermissions(in, out);
    if (opc == 1)
        syslogCom(7, in);
    else
        syslogCom(8, in);
}

void scanFolder(char *inPath, char *outPath, int switchSize) {
    syslog(LOG_INFO, "Scanning catalog: %s", inPath);
    struct dirent *entryPointer;
    DIR *dir;
    dir = opendir(inPath);
    char *fileTempPath;
    while ((entryPointer = readdir(dir)) != NULL) {
        char *fileName = entryPointer->d_name;
        syslog(LOG_INFO, "Scanning entry: %s", fileName);
        if (isFile(entryPointer)) {
            syslog(LOG_INFO, "Entry: %s is file. Checking sync", fileName);
            fileTempPath = addFileNameToPath(inPath, fileName);
            bool fileNeedSync = isFileNeedSync(fileName, inPath, outPath);
            if (fileNeedSync) {
                syslog(LOG_INFO, "File: %s is need sync.", fileName);
                copyFile(inPath, outPath, switchSize, fileTempPath);
            }
        }
    }
    closedir(dir);
}

void copyFile(char *inPath, char *outPath, int switchSize, char *tempPath) {
    if (isLargeFile(switchSize, tempPath)) {
        mapping_copy(tempPath, replaceCatalog1(tempPath, inPath, outPath));
    } else {
        copy(tempPath, replaceCatalog1(tempPath, inPath, outPath));
    }
}

bool isLargeFile(int switchSize, const char *filePath) { return getFileSize(filePath) > switchSize; }

void wakeUpSignalHandler() {
    syslog(LOG_DEBUG, "Demon waked up");
}

bool isFile(const struct dirent *file) {
    return file != NULL && file->d_type == DT_REG;
}

bool isCatalog(char *path) {
    struct stat buf;
    if (stat(path, &buf) == 0) {
        return (buf.st_mode & S_IFDIR);
    }
    return false;
}

void syslogCom(int in, char *file) {
    switch (in) {
        case 1:
            syslog(LOG_ERR, "Error in getting date from file %s", file);
            break;
        case 2:
            syslog(LOG_ERR, "Error in getting chmod from file %s", file);
            break;
        case 3:
            syslog(LOG_ERR, "Error in setting date for file %s", file);
            break;
        case 4:
            syslog(LOG_ERR, "Error in setting chmod for file %s", file);
            break;
        case 5:
            syslog(LOG_INFO, "File %s not exist. Removing if from catalog", file);
            break;
        case 6:
            syslog(LOG_ERR, "File open error - %s", file);
            break;
        case 7:
            syslog(LOG_INFO, "File %s copied", file);
            break;
        case 8:
            syslog(LOG_INFO, "File %s copied using mapping", file);
            break;
    }
}

bool isFileExists(char *filePath) {
    syslog(LOG_ERR, "Checking is file exit %s", filePath);

    struct stat buffer;
    return (stat(filePath, &buffer) == 0);
}

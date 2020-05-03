#include "function.h"

void copyFile(char *inPath, char *outPath, int switchSize, char *tempPath);

off_t getFileSize(const char *in) {
    struct stat size;
    if (stat(in, &size) == 0) {
        return size.st_size;
    }
    return -1;
}

time_t getFileModificationTime(char *in) {
    struct stat attr;
    if (stat(in, &attr) == -1) {
        syslog(LOG_ERR, "Error while reading modification date. File: %s", in);
        exit(EXIT_FAILURE);
    }
    return attr.st_mtime;
}

mode_t getFilePermissions(char *in) {
    struct stat attr;
    if (stat(in, &attr) == -1) {
        syslog(LOG_ERR, "Error while reading permissions. File: %s", in);
        exit(EXIT_FAILURE);
    }
    return attr.st_mode;
}

void changeParameters(char *in, char *out) {
    struct utimbuf times;
    times.actime = 0;
    times.modtime = getFileModificationTime(in);
    if (utime(out, &times) == -1) {
        syslog(LOG_ERR, "Error in changing file times: %s", out);
        exit(EXIT_FAILURE);
    }
    mode_t inFileMode = getFilePermissions(in);
    if (chmod(out, inFileMode) == -1) {
        syslog(LOG_ERR, "Error while setting new file permissions: %s", out);
        exit(EXIT_FAILURE);
    }
}

char *replaceCatalog2(char *path, char *catalogOnePath, char *catalogTwoPath) {
    char *fullPath = path + strlen(catalogTwoPath);
    char *newPath = malloc(strlen(catalogOnePath) + strlen(fullPath) + 1);
    strcpy(newPath, catalogOnePath);
    strcat(newPath, fullPath);
    return newPath;
}

char *replaceCatalog1(char *path, char *catalogOnePath, char *catalogTwoPath) {
    char *fullPath = path + strlen(catalogOnePath);
    char *newPath = malloc(strlen(catalogTwoPath) + strlen(fullPath) + 1);
    strcpy(newPath, catalogTwoPath);
    strcat(newPath, fullPath);
    return newPath;
}

char *addToPath(char *path, char *add) {
    char *newPath = malloc(strlen(path) + 2 + strlen(add));
    strcpy(newPath, path);
    strcat(newPath, "/");
    strcat(newPath, add);
    newPath[strlen(path) + 1 + strlen(add)] = '\0';
    return newPath;
}

bool isFileNeedSync(char *path, char *catalogOnePath, char *catalogTwoPath) {
    bool result = 0;
    char *pathName = path + strlen(catalogOnePath);
    char *searching = malloc(strlen(pathName));
    char *newPath = replaceCatalog1(path, catalogOnePath, catalogTwoPath);

    int i = strlen(newPath);
    for (i; newPath[i] != '/'; i--);
    strcpy(searching, newPath + i + 1);
    newPath[i] = '\0';
    struct dirent *file;
    DIR *dirOpenPath;
    dirOpenPath = opendir(newPath);

    while ((file = readdir(dirOpenPath))) {
        if (strcmp(file->d_name, searching) == 0) {
            free(searching);
            if ((file->d_type) == DT_DIR)  //    GDY JEST FOLDEREM
            {
                return 0;
            } else {
                int date1 = (int) getFileModificationTime(path), date2 = (int) getFileModificationTime(
                        addToPath(newPath, file->d_name));
                if (date1 == date2) {
                    return 0;
                } else {
                    return 1;
                }
            }
        } else {
            result = 1;
        }
    }
    closedir(dirOpenPath);
    return result;
}


void delete(char *catalogPathName, char *catalogPathOne, char *catalogPathTwo) {
    struct dirent *file;
    DIR *path;
    path = opendir(catalogPathName);
    while ((file = readdir(path))) {
        char *newPath = addToPath(catalogPathName, file->d_name);
        if (access(replaceCatalog2(newPath, catalogPathOne, catalogPathTwo), F_OK) == -1) {
            syslog(LOG_INFO, "Usunieto plik %s", newPath);
            remove(newPath);
        }
    }
    closedir(path);
}

void copy(char *in, char *out) {
    char buffor[16];
    int in_file, out_file;
    openfiles(in, out, &in_file, &out_file);

    int r_in, r_out;

    while ((r_in = read(in_file, buffor, sizeof(buffor))) > 0) {
        r_out = write(out_file, buffor, (ssize_t) r_in);
        if (r_out != r_in) {
            perror("Error!");
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
        syslog(LOG_ERR, "File open error!");
        exit(EXIT_FAILURE);
    }
}


void closefiles(char *in, char *out, int *inFile, int *outFile, int opc) {
    close(*inFile);
    close(*outFile);
    changeParameters(in, out);
    if (opc == 1)
        syslog(LOG_INFO, "File copied!");
    else
        syslog(LOG_INFO, "File copied using mapping!");
}

void scanFolder(char *inPath, char *outPath, int switchSize) {
    syslog(LOG_INFO, "Scanning catalog: %s\n", inPath);
    struct dirent *entryPointer;
    DIR *dir;
    dir = opendir(inPath);
    char *fileTempPath;
    while ((entryPointer = readdir(dir)) != NULL) {
        char* fileName = entryPointer->d_name;
        syslog(LOG_INFO, "Scanning entry: %s\n", fileName);
        if (isFile(entryPointer))
        {
            syslog(LOG_INFO, "Entry: %s is file. Checking sync\n", fileName);
            fileTempPath = addToPath(inPath, fileName);
            bool fileNeedSync = isFileNeedSync(fileTempPath, inPath, outPath);
            if (fileNeedSync) {
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

bool isLargeFile(int switchSize, const char *newPath) { return getFileSize(newPath) > switchSize; }


void wakeUpSignalHandler() {
    syslog(LOG_DEBUG, "Demon waked up");
}

bool isFile(const struct dirent *file) {
    char *fileTypeString;
    switch (file->d_type) {
        case DT_REG : {
            syslog(LOG_INFO, "Entry: %s type is: %s\n", file->d_name, "regular file");
            return true;
        }
        case DT_UNKNOWN:
            fileTypeString = "Unknown type";
            break;
        case DT_DIR:
            fileTypeString = "directory";
            break;
        case DT_FIFO:
            fileTypeString = "a named pipe";
            break;
        case DT_SOCK:
            fileTypeString = "local domain socket";
            break;
        case DT_CHR:
            fileTypeString = "character device";
            break;
        case DT_BLK:
            fileTypeString = "block device";
            break;
        case DT_LNK:
            fileTypeString = "symbolic link";
    }
    syslog(LOG_INFO, "Entry: %s type is: %s\n", file->d_name, fileTypeString);
    return false;
}


bool isCatalog(char *path) {
    struct stat buf;
    if (stat(path, &buf) == 0) {
        return (buf.st_mode & S_IFDIR);
    }
    return false;
}
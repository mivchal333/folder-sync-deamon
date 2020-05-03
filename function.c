#include "function.h"

off_t getFileSize(char *in) {
    struct stat size;
    if (stat(in, &size) == 0) {
        return size.st_size;
    }
    return -1;
}

time_t getFileDate(char *in)
{
    struct stat date;
    if (stat(in, &date) == -1) {
        syslog(LOG_ERR, "Blad z pobraniem daty modyfikacji dla pliku %s", in);
        exit(EXIT_FAILURE);
    }
    return date.st_mtime;
}

mode_t getFilePermissions(char *in) {
    struct stat mod;
    if (stat(in, &mod) == -1) {
        syslog(LOG_ERR, "Blad pobrania chmod dla pliku %s", in);
        exit(EXIT_FAILURE);
    }
    return mod.st_mode;
}

void changeParameters(char *in, char *out) {
    struct utimbuf date;
    date.actime = 0;
    date.modtime = getFileDate(in);
    if (utime(out, &date) != 0) {
        syslog(LOG_ERR, "Blad zwiazany z data modyfikacji!");
        exit(EXIT_FAILURE);
    }
    mode_t oldFile = getFilePermissions(in);
    if (chmod(out, oldFile) != 0) {
        syslog(LOG_ERR, "Blad ustawienia uprawnien do pliku!");
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

bool check(char *path, char *catalogOnePath, char *catalogTwoPath) {
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
                int date1 = (int) getFileDate(path), date2 = (int) getFileDate(
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

void scanFolder(char *pathName, char *catalogPathOne, char *catalogPathTwo, int switchSize) {
    printf("Scanning catalog: %s\n", pathName);
    struct dirent *file;
    DIR *path;
    path = opendir(pathName);
    char *newPath;
    while ((file = readdir(path))) {
        printf("%s  \n", file->d_name);
        if ((file->d_type) == DT_REG)// GDY nie jest folderem
        {
            newPath = addToPath(pathName, file->d_name);
            int i;
            if ((i = check(newPath, catalogPathOne, catalogPathTwo)) == 1) {
                if (getFileSize(newPath) > switchSize) {
                    mapping_copy(newPath, replaceCatalog1(newPath, catalogPathOne, catalogPathTwo));
                } else {
                    copy(newPath, replaceCatalog1(newPath, catalogPathOne, catalogPathTwo));
                }
            }
        }
    }
    closedir(path);
}

void WakeUpSignalHandler(int sig) {
    syslog(LOG_DEBUG, "Demon waked up");
}

bool isCatalog(char *in) {
    struct stat s;
    if (stat(in, &s) == 0) {
        if (s.st_mode & S_IFDIR) //sciezka jest katalogiem
        {
            return true;
        } else //sciezka nie jest katalogiem, wywal blad
        {
            return false;
        }
    }
}
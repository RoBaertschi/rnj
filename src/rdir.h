// ml2 ft=c
#ifndef RDIR_H_
#define RDIR_H_

#include <assert.h>
#include <stdbool.h>
enum rdir_entrytypes {
    RDIR_ENTRYTYPE_FILE = 0,
    RDIR_ENTRYTYPE_SYMLINK,
    RDIR_ENTRYTYPE_DIRECTORY,
    // Unknown or not suported entry type
    RDIR_ENTRYTYPE_UNKNOWN,

    RDIR_ENTRYTYPE_MAX,
};

struct rdir_dir;
struct rdir_entry {
    char* name;
    enum rdir_entrytypes type;
};

// Returns NULL on error.
struct rdir_dir *rdir_open_dir(const char* dir);
// Returns NULL if it has reached the end of the directory or on an error. There is no difference currently.
struct rdir_entry *rdir_read_dir(struct rdir_dir* dir);
// Creates a dir, returns NULL on success, else the error as a string.
// The Error string only lives until the next call to strerror() on LINUX (maybe posix).
const char* rdir_mdkir(const char* dir);
// Does only remove an empty directory or a file, returns NULL on success, else the error as a string.
// The Error string only lives until the next call to strerror() on LINUX (maybe posix).
const char* rdir_unlink(const char* dir);

char rdir_path_seperator(void);

// This returns true, if the directory exists, if it does not or an error ocurred, it will return false.
bool rdir_exists(const char* dir);

// Allocates using malloc() so you shoud free with free()
// TODO: Returns NULL on error. Currently no way to get error. On posix errno is set.
char* rdir_realpath(const char* dir);

void rdir_destroy_dir(struct rdir_dir* dir);
void rdir_destroy_entry(struct rdir_entry* entry);

// Entry has to be valid. This will be asserted.
const char* rdir_entrytype_str(enum rdir_entrytypes entry);

#ifdef RDIR_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>

#if defined (__unix__) || defined (__APPLE__) && defined (__MACH__)
#define RDIR_POSIX

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#else
#error "Unsupported operating system"
#endif

struct rdir_dir {
#ifdef RDIR_POSIX
    DIR* posix_dir;
#endif // RDIR_POSIX
};

// Returns NULL on error.
struct rdir_dir *rdir_open_dir(const char* directory) {
    struct rdir_dir *dir = (struct rdir_dir*) malloc(sizeof(struct rdir_dir));
    if (dir == NULL) {
        return dir;
    }
    memset(dir, 0, sizeof(struct rdir_dir));

#ifdef RDIR_POSIX
    dir->posix_dir = opendir(directory);
#endif // RDIR_POSIX

    return dir;
}

struct rdir_entry *rdir_read_dir(struct rdir_dir* dir) {
    struct rdir_entry *entry = (struct rdir_entry*) malloc(sizeof(struct rdir_entry));
    if (entry == NULL) {
        return entry;
    }

#ifdef RDIR_POSIX
    struct dirent *posix_entry = readdir(dir->posix_dir);
    if (posix_entry == NULL) {
        return NULL;
    }

    entry->name = (char*) malloc(strlen(posix_entry->d_name) * sizeof(char));
    if (entry->name == NULL) {
        return NULL;
    }
    strcpy(entry->name, posix_entry->d_name);


    switch (posix_entry->d_type) {
    case DT_REG:
        entry->type = RDIR_ENTRYTYPE_FILE;
        break;
    case DT_LNK:
        entry->type = RDIR_ENTRYTYPE_SYMLINK;
        break;
    case DT_DIR:
        entry->type = RDIR_ENTRYTYPE_DIRECTORY;
        break;
    default:
        entry->type = RDIR_ENTRYTYPE_UNKNOWN;
        break;
    }
#endif // RDIR_POSIX

    return entry;
}

const char* rdir_mdkir(const char* dir) {
#ifdef RDIR_POSIX
    errno = 0;
    if(mkdir(dir, 0755) != 0) {
        return strerror(errno);
    }
#endif // RDIR_POSIX
    return NULL;
}

const char* rdir_unlink(const char* dir) {
#ifdef RDIR_POSIX
    errno = 0;
    if(unlink(dir)) {
        return strerror(errno);
    }

    return NULL;

#endif // RDIR_POSIX
}

char rdir_path_seperator(void) {
#ifdef RDIR_POSIX
    return '/';
#endif // RDIR_POSIX
}

bool rdir_exists(const char* dir) {
#ifdef RDIR_POSIX
    errno = 0;
    DIR* dirdesc = opendir(dir);
    if (dirdesc) {
        closedir(dirdesc);
        return true;
    } else {
        return false;
    }
#endif // RDIR_POSIX
}

char* rdir_realpath(const char* dir) {
#ifdef RDIR_POSIX
    return realpath(dir, NULL);
#endif // RDIR_POSIX
}

void rdir_destroy_dir(struct rdir_dir* dir) {
    free(dir);
}
void rdir_destroy_entry(struct rdir_entry* entry) {
    free(entry->name);
    free(entry);
}


const char* rdir_entrytype_str(enum rdir_entrytypes entry) {
    assert(entry < RDIR_ENTRYTYPE_MAX && "invalid rdir_entrytypes passed to rdir_entrytype_str");
    assert(entry >= 0 && "invalid rdir_entrytypes passed to rdir_entrytype_str");
    switch (entry) {
    case RDIR_ENTRYTYPE_FILE:
        return "file";
    case RDIR_ENTRYTYPE_SYMLINK:
        return "symlink";
    case RDIR_ENTRYTYPE_DIRECTORY:
        return "directory";
    case RDIR_ENTRYTYPE_UNKNOWN:
        return "unknown";
    case RDIR_ENTRYTYPE_MAX:
    default:
        return "invalid";
  }
}

#endif // RDIR_IMPLEMENTATION
#endif // RDIR_H_

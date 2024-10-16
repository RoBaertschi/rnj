// ml2 ft=c
#ifndef RDIR_H_
#define RDIR_H_

#include <assert.h>
#include <string.h>
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
// Creates a dir, returns false on error.
bool rdir_mdkir(const char* dir);
// Does only remove an empty directory or a file
bool rdir_unlink(const char* dir);

void rdir_destroy_dir(struct rdir_dir* dir);
void rdir_destroy_entry(struct rdir_entry* entry);

// Entry has to be valid. This will be asserted.
const char* rdir_entrytype_str(enum rdir_entrytypes entry);

#ifdef RDIR_IMPLEMENTATION
#include <stdlib.h>

#if defined (__unix__) || defined (__APPLE__) && defined (__MACH__)
#define RDIR_POSIX

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
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

bool rdir_mdkir(const char* dir) {
#ifdef RDIR_POSIX
    if(mkdir(dir, 0755) != 0) {
        return false;
    }
#endif // RDIR_POSIX
    return true;
}

bool rdir_unlink(const char* dir) {
#ifdef RDIR_POSIX
    int status = unlink(dir);
    return status == 0;
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

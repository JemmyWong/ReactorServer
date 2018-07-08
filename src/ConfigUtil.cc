#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h> /* isblank*/

#include "ConfigUtil.h"

/*
pointer meaning:

...port...=...8000...
   |  |   |   |  |
  *fs |   |   |  *be    f->forward  b-> back
      *fe |   *bs       s->start    e-> end
          *equal
*/

int get_config(char *file, char *key, char *value) {
    int find = 0;
    if (!file || !key || !value) return find;

    FILE *fd = fopen(file, "r");
    if (!fd) {
        fprintf(stderr, "open config.ini error: %s\n", strerror(errno));
        exit(1);
    }

    char left[255], right[255], line[255];
    char *fs, *fe, *equal, *bs,*be, *ite;

    while (fgets(line, 255, fd) != NULL) {
        ite = line;
        equal = strchr(line, '=');

        // find key
        while (isblank(*ite)) ++ite;
        fs = ite;
        if (*fs == '#') continue;
        while (!isblank(*ite)) ++ite;
        fe = ite - 1;
        strncpy(left, fs, fe - fs + 1);
        left[fe - fs + 1] = '\0';
        if (strcmp(left, key) != 0) continue;

        // get value
        find = 1;
        ite = equal + 1;
        while (isblank(*ite)) ++ite;
        bs = ite;
        while (!isblank(*ite) && *ite != '\0') ++ite;
        be = ite - 1;

        strncpy(value, bs, be - bs + 1);
        value[be - bs + 1] = '\0';

        break;
    }
    
    fclose(fd);
    return find;
}

pthread_once_t ConfigFile::ponce = PTHREAD_ONCE_INIT;

void ConfigFile::init(string fileName_) {
    if (fileName.empty()) {
        fileName = std::move(fileName_);
    }
    pthread_once(&ponce, &ConfigFile::initKeyValue);
}

void ConfigFile::initKeyValue() {

}
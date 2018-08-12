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

    while (fgets(line, 255, fd) != nullptr) {
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

void ConfigFile::init(const string &path_) {
    string path;
    if (path.find("/") == string::npos) {
        path.assign("/root/" + path);
    }

    FILE *fd = fopen(path.c_str(), "r");
    if (!fd) {
        fprintf(stderr, "open config.conf error: %s\n", strerror(errno));
        slog_error("open config.conf error: %s\n", strerror(errno));
        exit(1);
    }
    /* clean old data */
    confMap_.clear();
    char line[255];
    char *fs, *fe, *equal, *bs,*be, *ite;

    while (fgets(line, 255, fd) != nullptr) {
        ite = line;
        equal = strchr(line, '=');
        if (!equal) continue;

        // find key
        while (isblank(*ite)) ++ite;
        fs = ite;
        if (*fs == '#') continue;
        while (!isblank(*ite) && (ite != equal)) ++ite;
        fe = ite;

        // get value
        ite = equal + 1;
        while (isblank(*ite)) ++ite;
        bs = ite;
        while (!isblank(*ite) && *ite != '\0') ++ite;
        be = ite;
        string key = string(fs, fe-fs);
        string value = string(bs, be-bs);
        if (!key.empty() && !value.empty()) {
            confMap_.insert(make_pair(key, value));
        }
    }

    fclose(fd);
}

const string &ConfigFile::getValue(const string &key) {
    return confMap_[key];
}

void ConfigFile::setValue(const string &key, const string &value) {
    confMap_[key] = value;
}

/* global variable */
ConfigFile &config = Singleton<ConfigFile>::instance();
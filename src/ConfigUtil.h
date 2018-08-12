#ifndef _CONFIGUTIL_H_
#define _CONFIGUTIL_H_

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstring>
#include <map>
#include <pthread.h>

#include "Slog.h"
#include "Singleton.h"

using namespace std;

int get_config(char *file, char *key, char *value);

class ConfigFile {
public:
    ConfigFile() = default;
    ConfigFile(ConfigFile &) = delete;

    void init(const string &path);
    const string &getValue(const string &key);
    void setValue(const string &key, const string &value);
private:
    std::map<string, string> confMap_;
};

#endif
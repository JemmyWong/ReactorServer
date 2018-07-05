#ifndef _CONFIGUTIL_H_
#define _CONFIGUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <map>
#include <pthread.h>

/*
char configPath[255] = "../config.ini";
*/

using namespace std;

int get_config(char *file, char *key, char *value);

class ConfigFile {
public:
    void init(string fileName);
    static void initKeyValue();
    string getValue(string key);
    int setValue(string key, string value);
private:
    string fileName;
    std::map<string, string> confMap;
    static pthread_once_t ponce;
};

#endif
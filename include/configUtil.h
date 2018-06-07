#ifndef _CONFIGUTIL_H_
#define _CONFIGUTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
char configPath[255] = "../config.ini";
*/

int get_config(char *file, char *key, char *value);

#endif
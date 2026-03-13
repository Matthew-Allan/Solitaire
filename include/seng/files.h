#ifndef FILES_H
#define FILES_H

#include <stdio.h>

char *getPath(const char *rel);
FILE *openFile(const char *rel);
char *getFileText(const char *path);

#endif
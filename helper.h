#ifndef P3_HELPER_H
#define P3_HELPER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>

// prototypes of all functions
void* Free(void *ptr);
char** strTokenize(const char *str, const char *delimiters, size_t *numOfTokens, const char *specialTokens);
char* strStrip(const char *str, const char* delimiters);
void printStrTokens(char **tokens, size_t numOfTokens, const char *delimiter);
char* strCombineTokens(char **tokens, size_t numOfTokens, const char *delimiter);
char* strReplace(const char *str, const char *oldSubStr, const char *newSubStr, ssize_t numOfOccurrences);
void* freeArrayOfStrings(char **array, size_t numOfStrings);
char* strdup(const char *str);
char** strDupArrayOfStrings(char **array, size_t numOfStrings);
char* read_file(int fd);

#endif //P3_HELPER_H

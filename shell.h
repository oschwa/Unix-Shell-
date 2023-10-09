#ifndef SHELL_H
#define SHELL_H

#include <assert.h>  // assert
#include <fcntl.h>   // O_RDWR, O_CREAT
#include <stdbool.h> // bool
#include <stdio.h>   // printf, getline
#include <stdlib.h>  // calloc
#include <string.h>  // strcmp
#include <unistd.h>  // execvp
#include <sys/wait.h> //waitpid

#define MAXLINE 80
#define PROMPT "osh> "

#define RD 0
#define WR 1

bool equal(char *a, char *b);
int fetchline(char **line);
int interactiveShell();
int runTests();
void processLine(char *line);
void executeCommand( char * command,  char * commands[],  bool waitFlag);
int main();

#endif

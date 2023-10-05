#include "shell.h"

int main(int argc, char **argv)
{
  if (argc == 2 && equal(argv[1], "--interactive"))
  {
    return interactiveShell();
  }
  else
  {
    return runTests();
  }
}

// interactive shell to process commands
int interactiveShell()
{
  bool should_run = true;
  char *line = calloc(1, MAXLINE);
  while (should_run)
  {
    printf(PROMPT);
    fflush(stdout);
    int n = fetchline(&line);
    printf("read: %s (length = %d)\n", line, n);
    // ^D results in n == -1
    if (n <= 0 || equal(line, "exit"))
    {
      should_run = false;
      continue;
    }

    // split string by spaces
    char *spaceToken = strtok(line, " ");

    // create pointer to array to store all commands
    char *commands[MAXLINE];
    int arguementsCounter = 0;

    // parse through string and extract all commands
    while (spaceToken != NULL)
    {
      commands[arguementsCounter++] = spaceToken;
      spaceToken = strtok(NULL, " ");
    }

    char *currComm = NULL;
    bool waitFlag = false;
    bool hasActiveCommand = false;

    //the code below will parse through the string and print each command that needs to execute one by one. This will parse through seperators "&" and ";"
    for (int i = 0; i < arguementsCounter; i++)
    {
      if (strcmp(commands[i], ";") == 0)
      {
        waitFlag = true;
        if (hasActiveCommand)
        {
          printf("Last Command Formed: %s\n", currComm);
          free(currComm);  // Free previously allocated memory
          currComm = NULL; // Reset pointer to NULL for the next command
        }
      }
      else if (strcmp(commands[i], "&") == 0)
      {
        waitFlag = false;
        if (hasActiveCommand)
        {
          printf("Last Command Formed: %s\n", currComm);
          free(currComm);  // Free previously allocated memory
          currComm = NULL; // Reset pointer to NULL for the next command
        }
      }
      else
      {
        // Allocate memory for the current command
        if (currComm == NULL)
        {
          currComm = malloc(strlen(commands[i]) + 1);
          strcpy(currComm, commands[i]);
          hasActiveCommand = true;
        }
        else
        {
          // Reallocate memory for currComm and concatenate the current command
          currComm = realloc(currComm, strlen(currComm) + strlen(commands[i]) + 2); // +2 for space and null terminator
          strcat(currComm, " ");
          strcat(currComm, commands[i]);
        }
      }
    }
    if (currComm != NULL)
    {
      printf("Last Command Formed: %s\n", currComm);
      // free(currComm); // Free the memory for currComm if there is something that is still not parsed yet
    }
  }
  free(line);
  return 0;
}

void processLine(char *line) { printf("processing line: %s\n", line); }

int runTests()
{
  printf("*** Running basic tests ***\n");
  char lines[7][MAXLINE] = {
      "ls", "ls -al", "ls & whoami ;", "ls > junk.txt", "cat < junk.txt",
      "ls | wc", "ascii"};
  for (int i = 0; i < 7; i++)
  {
    printf("* %d. Testing %s *\n", i + 1, lines[i]);
    processLine(lines[i]);
  }

  return 0;
}

// return true if C-strings are equal
bool equal(char *a, char *b) { return (strcmp(a, b) == 0); }

// read a line from console
// return length of line read or -1 if failed to read
// removes the \n on the line read
int fetchline(char **line)
{
  size_t len = 0;
  size_t n = getline(line, &len, stdin);
  if (n > 0)
  {
    (*line)[n - 1] = '\0';
  }
  return n;
}
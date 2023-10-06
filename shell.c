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
    //set arguement to NULL here so that exec executes properly and knows to stop
    commands[arguementsCounter++] = NULL;
    //Fork into a child process for executing command.
    executeCommand(commands[0], commands);
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

// fork() into a child process.
// execvp() to run command with Unix API.
void executeCommand(char * command, char * commands[])
{
  //  Process ID's are used to differentiate
  //  between parent and child. 
  pid_t parent = getpid();
  pid_t pid = fork();

  //  If the child process was unsuccessfully created, 
  //  then return an error message.
  if (pid == -1)
  {
    printf("%s, %s", "ERROR: Invalid Unix Command", command);
  }
  /**
   * TODO: Oliver speaking: Command execution works without waitpid(), I just
   * added it here because without it the "osh>" prompt disappears and only
   * appears in output. My plan is to come back and add processing of the "&"
   * and ";" options, with the prompt returning. 
   * 
   * Then, I will focus on polishing fork()/execvp()/wait() so I can move on 
   * to dup2() and pipe(). I hope to be done by either Saturday or early Sunday.
   * Let me know if this works for you. 
  */
  //  If the process is a parent process, then
  //  parent must wait for child's exit.
  else if (pid > 0) 
  {
    int s;
    waitpid(pid, &s, 0);
  }
  //  Else, the command is processed with execvp()
  //  and a Unix command is executed. 
  else
  {
    execvp(command, commands);
    _exit(EXIT_FAILURE);
  }
  //  Flush stcout so that prompt remains 
  //  after process is finished.
  fflush(stdout);
  return;
}

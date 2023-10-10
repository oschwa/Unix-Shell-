#include "shell.h"

struct instructions{
  char* command[MAXLINE];
  char* arguements[MAXLINE];
};

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
    processLine(line);
  }
  free(line);
  return 0;
}

void processLine(char *line)
{
  printf("processing line: %s\n", line);

  // split string by spaces
  char *spaceToken = strtok(line, " ");

  // create pointer to array to store all commands
  char *commands[MAXLINE];
  int argumentsCounter = 0;

  // parse through string and extract all commands
  while (spaceToken != NULL)
  {
    commands[argumentsCounter++] = spaceToken;
    spaceToken = strtok(NULL, " ");
  }

  char *cmd = NULL;
  char *cmdArgs[argumentsCounter + 1];
  bool waitFlag = true;

  // the code below will parse through the string and print each command that needs to execute one by one. This will parse through seperators "&" and ";"

  int j = 0;
  for (int i = 0; i < argumentsCounter; i++)
  {
    if (strcmp(commands[i], ";") == 0 || strcmp(commands[i], "&") == 0)
    {
      waitFlag = (strcmp(commands[i], ";") == 0);
      cmdArgs[j] = NULL; // Set the current position to NULL
      executeCommand(cmd, cmdArgs, waitFlag);
      free(cmd);
      cmd = NULL;
      memset(cmdArgs, '\0', sizeof(cmdArgs));
      j = 0;
    }
    else
    {
      if (cmd == NULL)
      {
        // Allocate memory for the current command
        cmd = malloc(strlen(commands[i]) + 1);
        strcpy(cmd, commands[i]);
      }
      cmdArgs[j] = commands[i];
      ++j;
      }

      if (i == argumentsCounter - 1)
      {
        cmdArgs[j] = NULL;
        executeCommand(cmd, cmdArgs, waitFlag);
        free(cmd);
        cmd = NULL;
        memset(cmdArgs, '\0', sizeof(cmdArgs));
        j = 0;
      }
    }
}

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
void executeCommand(char *command, char *commands[],
                    bool waitFlag)
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
  //  If the process is a parent process, then
  //  parent must wait for child's exit. If "&"
  //  was specified then run concurrent.
  else if (pid > 0 && waitFlag)
  {
    waitpid(pid, NULL, 0);
    return;
  }
  //  Else, the command is processed with execvp()
  //  and a Unix command is executed.
  else if (pid == 0)
  {
    execvp(command, commands);
  }
}
/*
Authors: Oliver Schwab & Disha Mahajan
Description: Working Unix Shell capable of using fork(), execvp(), dup2(), pipe() system calls to perform the following actions:
Opening a pipe by using the "|" symbol between commands
Redirecting input and output between commands using '<' and '>'
Running commands concurrently with the '&' symbol

This is the implementation file which contains the main method.

*/
#include "shell.h"

char *hist;

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

void executeCommandsToPipe(char *cmd, char *cmdArgs[], char *arguments[], int currIndex, int numOfArguments)
{
  //  Second command arguments pointers
  // printf("Num of args: %d and curr index :%d\n", numOfArguments, currIndex);
  // printf("arguements at curr are: %s\n", arguments[currIndex]);
  char *otherCmdArgs[numOfArguments - currIndex];
  char *otherCmd = NULL;
  int pointer = 0;
  //  Obtain command that pipe will provide input for.
  int i = currIndex + 1;
  while (i < numOfArguments)
  {
    if (otherCmd == NULL)
    {
      otherCmd = malloc(strlen(arguments[i]) + 1);
      strcpy(otherCmd, arguments[i]);
    }
    otherCmdArgs[pointer] = malloc(strlen(arguments[i]) + 1);
    strcpy(otherCmdArgs[pointer++], arguments[i]);
    ++i;
  }
  otherCmdArgs[pointer] = NULL;

  //  Fork a child process, that forks another child process.
  int fileDescriptor[2];
  pipe(fileDescriptor);
  pid_t pid = fork();

  //  If the current process is the first child process, place
  //  contents of first command execution into standard output.
  //  This is read later by the second child process.
  if (pid == 0)
  {
    //  In the array that serves as the structure for the pipe, the
    //  indexes 0 and 1 represent the read-end and the write-end respectively.
    close(fileDescriptor[0]);
    dup2(fileDescriptor[1], STDOUT_FILENO);
    close(fileDescriptor[1]);
    execvp(cmd, cmdArgs);
    exit(1);
  }
  else
  {
    pid = fork();
    //  If the current process is the second child process, then
    //  read contents of previous command from standard input.
    if (pid == 0)
    {
      close(fileDescriptor[1]);
      dup2(fileDescriptor[0], STDIN_FILENO);
      close(fileDescriptor[0]);
      //  In this case, the second command does need a null
      //  terminator added.
      execvp(otherCmd, otherCmdArgs);
      exit(1);
    }
    else
    {
      //  Original parent process ensures that the
      //  write/read of the pipe are fully closed.
      //  Then, the process waits for children.
      int status;
      close(fileDescriptor[0]);
      close(fileDescriptor[1]);
      waitpid(pid, &status, 0);
    }
  }

  for (int j = 0; j < numOfArguments - currIndex; j++)
  {
    free(otherCmdArgs[j]);
  }
  free(otherCmd);
}

void inputRedirect(char *firstParam, char *file)
{
  pid_t parent = getpid();
  // fork so that we can continue looping after
  pid_t pid = fork();
  if (pid == 0)
  { // check if child process
    // open file for reading
    int file_desc = open(file, O_RDONLY);
    if (file_desc == -1) // in the case we couldn't open the file for whatever reason
    {
      printf("File for input redirect %s could not open successfully\n", file);
      close(file_desc);
    }
    else
    {
      // tells system to read file from standard input not keyboard
      dup2(file_desc, STDIN_FILENO);
      char *command[] = {firstParam, NULL};
      bool flag = false;
      execvp(firstParam, command);
      close(file_desc);
      return;
    }
  }
  else
  {
    waitpid(pid, NULL, 0);
    return;
  }
}

void outputRedirect(char *firstParam, char *file, char *args[])
{
  pid_t parent = getpid();
  // fork so that we can continue looping after
  pid_t pid = fork();
  if (pid == 0)
  { // check if child process
    // open file for reading
    int file_desc = open(file, O_WRONLY);
    if (file_desc == -1) // in the case we couldn't open the file for whatever reason
    {
      printf("File for output redirect %s could not open successfully\n", file);
      close(file_desc);
    }
    else
    {
      // tells system to write to file from standard input not keyboard
      dup2(file_desc, STDOUT_FILENO);
      bool flag = false;
      execvp(firstParam, args);
      close(file_desc);
      return;
    }
  }
  else
  {
    waitpid(pid, NULL, 0);
    return;
  }
}

void parseAndExecute(char *line)
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
  if (strcmp(commands[argumentsCounter - 1], ";") != 0)
  {
    commands[argumentsCounter++] = ";";
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
    else if (strcmp(commands[i], "<") == 0 || strcmp(commands[i], ">") == 0)
    { // detect input redirection
      cmdArgs[j] = NULL;
      // check which type of redirection and go to corressponding function
      //  commands [i+1] grabs the file name
      (strcmp(commands[i], "<") == 0) ? inputRedirect(cmd, commands[i + 1]) : outputRedirect(cmd, commands[i + 1], cmdArgs);
      // clear mem for the next input
      free(cmd);
      cmd = NULL;
      memset(cmdArgs, '\0', sizeof(cmdArgs));
      j = 0;
      break;
    }
    else if (strcmp(commands[i], "|") == 0)
    {
      //  method for opening pipe and transmitting command data between
      //  processes.
      cmdArgs[j] = NULL;
      executeCommandsToPipe(cmd, cmdArgs, commands, j, argumentsCounter - 1);
      //  TODO: Given that the pipe implementation only supports
      //  two commands, and not chaining, the inner loop is exited
      //  after one pipe is detected. This returns to the prompt.
      break;
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
void processLine(char *line)
{
  // check if history command is envoked
  if (strcmp(line, "!!") == 0)
  {
    parseAndExecute(hist);
  }
  else if (strcmp(line, "ascii") == 0)
  {
    printf("     ________\n");
    printf("    /        \\\n");
    printf("   /    __    \\\n");
    printf("  /    /  \\    \\\n");
    printf(" /____/____\\____\\\n");
    printf("|        __      |\n");
    printf("|      |   |     |\n");
    printf("|______|___|_____|\n");
  }
  else
  {
    // add current command to history and parse
    hist = strdup(line);
    parseAndExecute(line);
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
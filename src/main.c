#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  // Uncomment this block to pass the first stage


  // REPL loop
  // Wait for user input
  bool running = true;
  char input[100];
  while(running)
  {
    printf("$ ");
    //read
    fgets(input, 100, stdin);
    // null terminate the input
    input[strlen(input) - 1] = '\0';
    // check for exit
    
    char* token = strtok(input, " ");

    if(strcmp(token, "exit"))
    {
      while(token != NULL)
      {
        token = strtok(input, " ");
        if(strcmp(token, "0"))
        {
          running = false;
          exit(0);
        }
        else if (strcmp(token, "1"))
        {
          exit(1);
        }
      }

    }
    else
    {
        printf("%s: command not found\n", input);
    }
  
    setbuf(stdout, NULL);

  }
  


  return 0;
}

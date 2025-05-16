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
    while(token != NULL)
    {
      if(strcmp(token, "exit") == 0)
      {
        token = strtok(NULL, " ");
        if(strcmp(token, "0") == 0)
        {
          running = false;
          exit(0);

        }
        else if(strcmp(token, "1") == 0)
        {
          exit(1);
        }
      }
      else if(strcmp(token, "echo") == 0)
      {
        printf("input string: %s", input);
        char echo[100];
        echo[0] = '\0';
        token = strtok(input, " ");
        token = strtok(NULL," ");

        while(token != NULL)
        {
          if(strlen(echo) > 0){
            strcat(echo," ");
          }
          strcat(echo,token);
          token = strtok(NULL, " ");
          
        } 
        printf("reached echo: %s",echo);
      }
      else
      {
        printf("%s: command not found\n", input);
      }
      token = strtok(NULL, " ");
    }
  
    setbuf(stdout, NULL);

  }
  


  return 0;
}

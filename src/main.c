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
  char token_input[100];

  while(running)
  {
    printf("$ ");
    //read
    fgets(input, 100, stdin);
    // null terminate the input
    input[strlen(input) - 1] = '\0';
    strcpy(token_input,input);
    // check for exit
    
    char* token = strtok(token_input, " ");
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
        
        char echo[100];
        echo[0] = '\0';
        char echo_input[100];
        strcpy(echo_input,input);
        //printf("input string: %s", echo_input);
        token = strtok(echo_input, " ");
        token = strtok(NULL," ");

        while(token != NULL)
        {
          if(strlen(echo) > 0){
            strcat(echo," ");
          }
          strcat(echo,token);
          token = strtok(NULL, " ");
          
        } 
        printf("%s",echo);
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

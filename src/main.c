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
    //printf("%s is the token at line 30",token);
    // parse the command while checking for builtins
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
        token = strtok(echo_input, " ");
        token = strtok(NULL," ");

        while(token != NULL)
        {
          if(strlen(echo) > 0)
          {
            strcat(echo," ");
          }
          strcat(echo,token);
          token = strtok(NULL, " ");
          
        } 
        printf("%s\n",echo);
      }
      else if(strcmp (token, "type") == 0)
      {
        // grab the next token (Ideally echo, exit, etc.)
        token = strtok(NULL,"");
        //printf("%s is the curr token, %s");
        if(strcmp(token,"exit") == 0 || strcmp(token,"echo") == 0 || strcmp(token,"type") == 0)
        {
          printf("%s is a shell builtin\n", token);
        }
        else
        {
          char type_err[100];
          strcat(type_err,token);
          // I already grab the first token so just check if there are any more)
          token = strtok(NULL," ");
          //printf("%s, this is the token at 84",token);
          while(token != NULL)
          {
            if(strlen(type_err) > 0)
            {
            strcat(type_err," ");
            }
            strcat(type_err,token);
            token = strtok(NULL, " ");
            //printf("%s, this is the token at 93",token);
          }
          type_err[strlen(type_err) - 1] = '\0';
          printf("type %s: command not found\n", type_err);
        }

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

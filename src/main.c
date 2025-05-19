#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

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
        //printf("enters else if branch");
        // grab the next token (Ideally echo, exit, etc.)
        token = strtok(NULL,"");
        //printf("%s is the curr token, %s");
        if(strcmp(token,"exit") == 0 || strcmp(token,"echo") == 0 || strcmp(token,"type") == 0)
        {
          printf("%s is a shell builtin\n", token);
        }
        else
        {
          // is it a PATH?
          char *path = getenv("PATH");
          // path isn't null and path isn't an empty string
          if(path != NULL && path[0] != '\0')
          {

            // copy it over to consume the tokens
            char token_path[100];
            strcpy(token_path,path);
            // meant to hold extracted paths
            char ind_path[100];
            char* path_token = strtok(token_path,":");
            bool FOUND = false;
            while(path_token != NULL && FOUND == false)
            {
              strcat(ind_path,path_token);
              //grabbed a path
              strcat(ind_path,"/");
              strcat(ind_path,token);
              //tack on the user-given cmd : is it executable?
              if(access(ind_path,X_OK) == 0)
              {
                printf("%s is %s\n",token,ind_path);
                FOUND = true;
                // get out of loop, do not go into else branch!
                running = false;
              }
              // keep going until they are all checked, null out ind path first
              ind_path[0] = '\0';
              path_token = strtok(NULL,":");

            }
            // uh oh, no more left to check, default into the type else case

          }
          // general type invalid case
          else
          {
            
            char type_err[100];
            strcat(type_err,token);
            // I already grab the first token so just check if there are any more)
            //printf("%s, this is the token at 85",token);
            //debugging
            token = strtok(NULL," ");
            
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
            type_err[strlen(type_err)] = '\0';
            printf("%s: not found\n", type_err);
            type_err[0] = '\0';
          }
        }
        continue;
      }
      else
      {
        //printf("enters else branch");
        printf("%s: command not found\n", input);
        
      }
      token = strtok(NULL, " ");
    }
  
    setbuf(stdout, NULL);

  }
  


  return 0;
}

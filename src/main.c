#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

bool echo(char* token, char input[]);
bool is_path(char ind_path[], char *token);
void cd(char* token);
void cd_relative(char* token);
bool single_quote(char output[], char* token, char input[]);

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  // Uncomment this block to pass the first stage

  // REPL loop
  // Wait for user input
  bool running = true;
  char input[100];
  char token_input[100];
  char ind_path[100];

  while (running)
  {
    printf("$ ");
    // read
    fgets(input, 100, stdin);
    // null terminate the input
    input[strlen(input) - 1] = '\0';
    strcpy(token_input, input);
    // check for exit

    char *token = strtok(token_input, " ");
    bool print = false;
    // printf("%s is the token at line 30",token);
    //  parse the command while checking for builtins
    while (token != NULL && print == false)
    {
      if (strcmp(token, "exit") == 0)
      {
        token = strtok(NULL, " ");
        if (strcmp(token, "0") == 0)
        {
          running = false;
          exit(0);
        }
        else if (strcmp(token, "1") == 0)
        {
          exit(1);
        }
      }
      else if (strcmp(token, "echo") == 0)
      {
       print = echo(token, input);
      }
      else if (strcmp(token, "pwd") == 0)
      {
        ind_path[0] = '\0';
        if(getcwd(ind_path,sizeof(ind_path)) != NULL)
        {
          printf("%s\n",ind_path);
        }
        
      }
      else if (strcmp(token, "cd") == 0)
      {
        token = strtok(NULL, " ");
        cd(token);
      }
      else if (strcmp(token, "type") == 0)
      {
        // printf("enters else if branch");
        //  grab the next token (Ideally echo, exit, etc.)
        token = strtok(NULL, " ");
        // printf("%s is the curr token, %s");

        // very general case for standard checks on existing commands using type
        if (strcmp(token, "exit") == 0 || strcmp(token, "echo") == 0 || strcmp(token, "type") == 0 || strcmp(token, "pwd") == 0)
        {
          printf("%s is a shell builtin\n", token);
          print = true;
          // break;
        }
        else
        {
          // is it a PATH?
          // char ind_path[100];
          print = is_path(ind_path, token);
          if (print == true)
          {
            printf("%s is %s\n", token, ind_path);
          }
          // general type invalid case
          else
          {
            char type_err[100];
            type_err[0] = '\0';
            while (token != NULL)
            {
              if (strlen(type_err) > 0)
              {
                strcat(type_err, " ");
              }
              strcat(type_err, token);
              token = strtok(NULL, " ");
              // printf("%s, this is the token at 93",token);
            }
            type_err[strlen(type_err)] = '\0';
            printf("%s: not found\n", type_err);
            // type_err[0] = '\0';
            print = true;
            // break;
          }
        }
        // continue;
      }
      else if (is_path(ind_path, token) == true) // call boolean function to check if the path is good)
      {
        // ignore commment lol
        char *argv[10];
        char argv_token_parse[100];
        strcpy(argv_token_parse, input);
        char *argv_token = strtok(argv_token_parse, " ");
        // TODO : handle max args allowed later?
        int argc = 0;
        while (argv_token != NULL && argc < 9)
        {
          argv[argc] = argv_token;
          argc++;
          argv_token = strtok(NULL, " ");
        }
        argv[argc] = NULL;

        pid_t process = fork();
        int status;
        if (process == 0)
        {
          // inside the child process
          execvp(argv[0], argv);
          return 1; // exec failed
        }
        else
        {
          // parent process, wait
          waitpid(process, &status, 0);
        }
      }
      else
      {
        // printf("enters else branch");
        printf("%s: command not found\n", input);
        print = true;
        // break;
      }
      token = strtok(NULL, " ");
    }

    setbuf(stdout, NULL);
  }

  return 0;
}
bool echo(char* token, char input[])
{
  bool print = false;
  char echo[100];
  echo[0] = '\0';
  char echo_input[100];

  strcpy(echo_input, input);
  token = strtok(echo_input, " ");
  token = strtok(NULL, " ");

  print = single_quote(echo,token,input);
  if(!print)
  {
    while (token != NULL)
    {
      if (strlen(echo) > 0)
      {
        strcat(echo, " ");
      }
      strcat(echo, token);
      token = strtok(NULL, " ");
    }
    printf("%s\n", echo);
    print = true;
  }

  

  return print;
}
bool is_path(char ind_path[], char *token)
{
  // is it a PATH?
  bool print = false;
  char *path = getenv("PATH");
  // path isn't null and path isn't an empty string
  if (path != NULL && path[0] != '\0')
  {
    // copy it over to consume the tokens
    char token_path[100];
    strcpy(token_path, path);
    // meant to hold extracted paths
    // ! char ind_path[100];
    char *path_token = strtok(token_path, ":");
    bool FOUND = false;
    while (path_token != NULL && FOUND == false)
    {
      ind_path[0] = '\0';
      strcat(ind_path, path_token);
      // grabbed a path
      strcat(ind_path, "/");
      strcat(ind_path, token);
      // tack on the user-given cmd : is it executable?
      if (access(ind_path, X_OK) == 0)
      {
        FOUND = true;
        // get out of loop, do not go into else branch!
        print = true;
        return print;
        // break;
      }
      // keep going until they are all checked, null out ind path first

      path_token = strtok(NULL, ":");
    }
    // uh oh, no more left to check, default into the type else case
    return print;
  }
}
void cd(char* token)
{
  
  // consume CD token, perform absolute / relative path check
  // absolute path case : check suing chdir()
  if(token[0] == '/')
  {
    
    if(chdir(token) != 0)
    {
      printf("cd: %s: No such file or directory\n", token);
    }
  }
  else if(token[0] == '~') 
  {
    char *home = getenv("HOME");
    chdir(home);
  }
  else
  {
    cd_relative(token);
  }
        
}
void cd_relative(char* token)
{
  char tokenize_cwd[100];
  tokenize_cwd[0] = '\0';
  getcwd(tokenize_cwd, sizeof(tokenize_cwd));
  //printf("%s cur cwd\n", tokenize_cwd);


  char cwd_tokens[10][100];
  memset(cwd_tokens,0,sizeof(cwd_tokens));

  char* token_cwd = strtok(tokenize_cwd, "/");
  // printf("%s cwd token\n",token_cwd);
  
  int num_cwd_tokens = 0;
 
  //printf("tokenize_cwd %s\n", tokenize_cwd);
  //printf("tokenize_rel %s\n", tokenize_rel);
  while(token_cwd != NULL )
  {

      strcpy(cwd_tokens[num_cwd_tokens],token_cwd);
      token_cwd = strtok(NULL,"/"); 
      //printf("%s cwd token\n",cwd_tokens[num_cwd_tokens]);
      num_cwd_tokens++;
    
  }
  int num_rel_tokens = 0;
  char tokenize_rel[100];
  tokenize_rel[0] = '\0';
  strcpy(tokenize_rel,token); // token is going to be holding the whole path

  char rel_tokens[10][100]; // so I can hold multiple strings at a time
  memset(rel_tokens,0,sizeof(rel_tokens));
  char* token_rel = strtok(tokenize_rel,"/");
  while(token_rel != NULL)
  {
      strcpy(rel_tokens[num_rel_tokens],token_rel);
      token_rel = strtok(NULL,"/");
      //printf("%s rel token\n",rel_tokens[num_rel_tokens]);
      num_rel_tokens++;
    
  }
  //printf("past the tokenizing while loop");
  // // now while loop to construct the new path based on relative
  char new_path[100];
  new_path[0] = '\0';
  int index = 0;

  while(index < num_rel_tokens)
  {
    // move up to the parent directory, get rid of child dir token
    if(strcmp(rel_tokens[index],"..") == 0)
    {
      strcpy(cwd_tokens[num_cwd_tokens-1],"\0");
      //printf("%s cwd_token case 1\n", cwd_tokens[num_cwd_tokens]);
      num_cwd_tokens--;
    }
    // dir name that is NOT just the cwd, append to path
    else if(strcmp(rel_tokens[index],".") != 0)
    {
      strcpy(cwd_tokens[num_cwd_tokens],rel_tokens[index]);
      //printf("%s cwd token case 2\n", cwd_tokens[num_cwd_tokens]);
      num_cwd_tokens++;
    }
    index++;
  }
  // //printf("past the construction while loop");
  // // cat together
  index = 0;
  strcat(new_path,"/");
  while(index < num_cwd_tokens)
  {
    
    strcat(new_path,cwd_tokens[index]);
    index++;
    if(index != num_cwd_tokens)
    {
      strcat(new_path,"/");
    }


  }
  new_path[strlen(new_path)] = '\0';
  //printf("%s\n",new_path);
  
  if(chdir(new_path) != 0)
    {
      printf("cd: %s: No such file or directory\n", token);
    }
    
}
bool single_quote(char output[], char* token, char input[])
{
  output[0] = '\0';
  bool print = false;
  if(token[0] == '\'')
  {
    // need to retokenize it differently based on ' instead of " "
    char quote_input[100];
    strcpy(quote_input,input);
    int quote_input_index = 0;
    int output_index = 0;
    bool single_quote_mode = false;
    bool closed_quote = false;
    //bool done_quote = false;
    int quote_input_max = strlen(quote_input);
    while( quote_input_index < quote_input_max)
    {
      // haven't found quote and mode isn't activated yet
      if(single_quote_mode)
      {
        // looks like an unnecessary check, but its fencepost solution T-T
        if(quote_input[quote_input_index] != '\'')
        {
          output[output_index] = quote_input[quote_input_index];
          output_index++;
        }

      }
      // quote case
      if(quote_input[quote_input_index] == '\'')
      {
        if(!single_quote_mode)
        {
          single_quote_mode = true;
        }
        else if(single_quote_mode)
        {
          // done with quote
          single_quote_mode = false;
          closed_quote = true;
        }
        
        
      }
      else if(!single_quote_mode && closed_quote && quote_input[quote_input_index] == ' ')
        {
          output[output_index] = ' ';
          output_index++;
          closed_quote = false;
        }
      // if we haven't found a quote and mode isn't activated, just keep going
      quote_input_index++;
    }
    output[output_index] = '\0';
    printf("%s\n", output);
    print = true;
  }
  return print;
}

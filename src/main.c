#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>

void raw_terminal_parsing(char input[], char* cmd_history[], int history_index);
bool history(char* cmd_history[], int history_index,bool list, char input[], char* token);
bool echo(char* token, char input[]);
bool is_path(char ind_path[], char *token);
void cd(char* token);
void cd_relative(char* token);
bool cat(char* token, char input[]);
bool single_quote(char* output_args[], char output[], char* token, char input[]);

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  // Uncomment this block to pass the first stage

  // REPL loop
  // Wait for user input
  char* cmd_history[100];
  bool running = true;
  // making input buffer large so it can run in both raw and regular mode safely
  char input[1024];
  char token_input[1024];
  char ind_path[100];
  int history_index = 0;
  while (running)
  {
    printf("$ ");
    // read
    // ! need to replace line 39 with my logic
    //fgets(input, 100, stdin);
    raw_terminal_parsing(input,cmd_history,history_index);

    // check for redirection
    char extracted_input[1024]; // cmd input only
    char redir_input[1024];
    bool redir = false;
    strcpy(token_input, input);
    char *redir_token = strtok(token_input);
    bool divert_input = false;
    while(redir_token != NULL)
    {
      if(redir_token != ">" && !divert_input)
      {
        strcat(extracted_input,redir_token);
        divert_input = true;
      }
      else if(redir_token == ">" || divert_input)
      {
        strcat(redir_input,redir_token);
        redir = true;
      }
    }
    // ! remember to set token input buffer correctly later
    token_input[0] = "\0";
    if(redir)
    {
      strcpy(token_input,extracted_input);
    }
    else
    {
      strcpy(token_input,input);
    }

    
    //printf("%s token input", token_input);
    char *token = strtok(token_input, " ");
    history(cmd_history,history_index,false,input,token);
    history_index++;
    

    
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
      else if(strcmp(token,"history") == 0)
      {
        print = history(cmd_history,history_index,true,input,token);
      }
      else if (strcmp(token, "echo") == 0)
      {
       print = echo(token, input);
      }
      else if (strcmp(token, "cat") == 0)
      {
       print = cat(token, input);
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
        if (strcmp(token, "exit") == 0 || strcmp(token, "echo") == 0 || strcmp(token, "type") == 0 || strcmp(token, "pwd") == 0 || strcmp(token,"history") == 0)
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

void raw_terminal_parsing(char input[], char* cmd_history[], int history_index)
{
  struct termios ori_termios;
  tcgetattr(STDIN_FILENO,&ori_termios);
  // begin setting flags on the copied attr
  struct termios raw_termios = ori_termios;
  // ! might need to mess with what flags I have set later on too
  raw_termios.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw_termios);
  // note : \n is basically when I should stop my parsing
  int pos = 0;
  char c;
  int history_pos = history_index;
  bool done = false;
  bool was_printed = false;
  while(!done)
  {
    read(STDIN_FILENO,&c,1);
    // alphabetical char case
    if( c >= 32 && c <= 126)
    {
      input[pos] = c;
      pos++;

    }
    // backspace case
    else if(c == 127 && pos > 0)
    {
      pos--;
      input[pos] = '\0';
      printf("\b \b");
      
      
    }
    // up arrow or back arrow case ( need to handle replacing input with history contents as well here!)
    else if(c == '\x1b')
    {
      input[pos] = c;
      pos++;
      read(STDIN_FILENO,&c,1);
      if(c == '[')
      {
        input[pos] = c;
        pos++;
        read(STDIN_FILENO,&c,1);
        if(c == 'A')
        {
          // at the point where we know the input is up arrow, no need to copy to buffer anymore b/c 
          // it will be replaced with history

          // up arrow, read from the 
          if(history_pos > 0)
          {
            history_pos--;
          }
          strcpy(input,cmd_history[history_pos]);
          pos = strlen(input);
          

        }
        else if(c == 'B')
        {
          if(history_pos < history_index - 1)
          {
            history_pos++;
          }
          strcpy(input,cmd_history[history_pos]);
          pos = strlen(input);
        }
        printf("\r\033[K");
        printf("$ %s",input);
        was_printed = true;
      }
    }
    // enter case
    else if(c == 10 || c == 13)
    {
      input[pos] = '\0';
      if(was_printed)
      {
        printf("\n");
        //printf("REPL input before exec: '%s'\n", input);
      }
      else
      {
        printf("%s\n",input);
      }
      
      done = true;
    }
  }

  tcsetattr(STDIN_FILENO,TCSAFLUSH,&ori_termios);

}

// ! instead of passing the redir bool in here, I'll have to pass it in through the other functions for cmds
void redir_output(char redir_input)
{

}
bool history(char* cmd_history[], int history_index,bool list, char input[], char* token)
{
  bool print = false;
  // if history cmd is invoked, do smth
  if(list)
  {
    int history_limit = 0;
    token = strtok(NULL," ");
    // I'll assume that its a number to make it easier ig
    if(token != NULL)
    {
      history_limit = atoi(token);
      history_limit++;
    }
    // list out the history 
    //printf("%d %d\n",history_limit,history_index);
    int i = (history_limit > 0) ? ((history_index+1)-history_limit) : 0;
    for(; i < history_index; i++)
    {
      printf("   %d  %s\n",i+1,cmd_history[i]);
    }
  }
  else
  {
    // add to the history
    cmd_history[history_index] = strdup(input);
    print = true;

  }
  return print;
}
bool echo(char* token, char input[])
{
  bool print = false;
  char echo[100];
  char* output_args[100];
  echo[0] = '\0';
  char echo_input[100];

  strcpy(echo_input, input);
  token = strtok(echo_input, " ");
  token = strtok(NULL, " ");

  print = single_quote(output_args,echo,token,input);
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

bool cat(char* token, char input[])
{
  bool print = false;
  char cat[100];
  char* output_args[100];
  cat[0] = '\0';
  char cat_input[100];

  strcpy(cat_input, input);
  token = strtok(cat_input, " ");
  token = strtok(NULL, " ");

  // this will just get rid of the single quotes i guess
  print = single_quote(output_args,cat,token,input);

  // so now I want to loop through the tokens
  // open the file
  // read contents 
  // direct to stdout ( should be a general case, as stdout should be modified if redir tokens exist)
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
bool single_quote(char* output_args[], char output[], char* token, char input[])
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

    char arg_arr[100];
    int arg_index = 0;
    int arg_string_index = 0;
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
  // allright time to retokenize the ouput array so I can put it into the args array
 
  return print;
}

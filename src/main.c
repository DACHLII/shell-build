#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>

// TODO : work on memory safety

// helper function definitions
void raw_terminal_parsing(char input[], char* cmd_history[], int history_index);
bool history(char* cmd_history[], int history_index,bool list, char input[], char* token);
bool echo(char* token, char input[]);
bool is_path(char ind_path[], char *token);
void cd(char* token);
void cd_relative(char* token);
bool cat(char* token, char input[]);
bool single_quote(char* output_args[], char output[], char* token, char input[]);

// constant definitions
#define CHAR_BUFFER_SIZE 1024
#define STR_BUFFER_SIZE 100
#define NUM_TOKENS 10




int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);


  char* cmd_history[STR_BUFFER_SIZE];
  bool running = true;
  char input[CHAR_BUFFER_SIZE];
  char token_input[CHAR_BUFFER_SIZE];
  char ind_path[STR_BUFFER_SIZE];
  int history_index = 0;

  while (running)
  {
    printf("$ ");
    
    

    // instead of using fgets to parse input, alternatively parse char by char to read for key presses
    raw_terminal_parsing(input,cmd_history,history_index);

    // redirection check section
    char extracted_input[STR_BUFFER_SIZE]; // cmd input only
    char redir_input[STR_BUFFER_SIZE];
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
   
    token_input[0] = "\0";
    if(redir)
    {
      strcpy(token_input,extracted_input);
    }
    else
    {
      strcpy(token_input,input);
    }

    // store input in history and begin consuming input
    char *token = strtok(token_input, " ");
    history(cmd_history,history_index,false,input,token);
    history_index++;
    
    // used to verify the success of a cmd helper function called in main
    bool print = false;
   
    // TODO : formally store the builtin commands in a data structure
    //  parse the cmd while checking for builtins
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
      // TODO : move type into it's own helper function for consistency purposes
      else if (strcmp(token, "type") == 0)
      {
        //  grab the next token (Ideally echo, exit, etc.)
        token = strtok(NULL, " ");

        // very general case for standard checks on existing commands using type
        // TODO : attempt to shorten this string of boolean checks later
        if (strcmp(token, "exit") == 0 || strcmp(token, "echo") == 0 || strcmp(token, "type") == 0 
        || strcmp(token, "pwd") == 0 || strcmp(token,"history") == 0)
        {
          printf("%s is a shell builtin\n", token);
          print = true;

        }
        else
        {
          // check to see if the type cmd is accompanied by a valid path
          print = is_path(ind_path, token);
          if (print)
          {
            printf("%s is %s\n", token, ind_path);
          }
          // type invalid case
          else
          {
            char type_err[STR_BUFFER_SIZE];
            type_err[0] = '\0';
            while (token != NULL)
            {
              if (strlen(type_err) > 0)
              {
                strcat(type_err, " ");
              }
              strcat(type_err, token);
              token = strtok(NULL, " ");
              
            }
            type_err[strlen(type_err)] = '\0';
            printf("%s: not found\n", type_err);
            print = true;
          }
        }
      // check to see if the input is a valid path, if so, execute the input
      else if (is_path(ind_path, token)) 
      {
        // set up argv and argc for exec purposes
        char *argv[NUM_TOKENS];
        char argv_token_parse[STR_BUFFER_SIZE];
        strcpy(argv_token_parse, input);
        char *argv_token = strtok(argv_token_parse, " ");
        // TODO : handle max args allowed 
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
        printf("%s: command not found\n", input);
        print = true;
      }
      token = strtok(NULL, " ");
    }
    // after each iteration of REPL loop, clear out user input buffer
    setbuf(stdout, NULL);
  }

  return 0;
}

/*
function that replaces what fgets() previously did in my shell with char by char parsing of input,
with the purpose of detecting key presses ( in this use case, for detected up and down arrow keys
to recall previously typed user input in history)
input[] provides user input, cmd_history[] stores the previously invoked cmds, and history_index
keeps track of where the last invoked cmd was
*/
void raw_terminal_parsing(char input[], char* cmd_history[], int history_index)
{
  // work to enable raw input in the terminal when user is typing
  tcgetattr(STDIN_FILENO,&ori_termios);
  struct termios raw_termios = ori_termios;
  raw_termios.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw_termios);

  int pos = 0;
  char c;
  int history_pos = history_index;
  bool done = false;
  bool was_printed = false;
  int begin_gen = 32, end_gen = 126, backspace = 127;
  int new_line = 10, carriage_return = 13;
  

  while(!done)
  {
    read(STDIN_FILENO,&c,1);
    // general char case
    if( c >= begin_gen && c <= end_gen)
    {
      input[pos] = c;
      pos++;

    }
    // backspace case
    else if(c == backspace && pos > 0)
    {
      pos--;
      input[pos] = '\0';
      printf("\b \b");
      
      
    }
    // up arrow or back arrow case 
    // begin with checking for escape character
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
        // separate cases for up and down arrow (denoted either by 'A' or 'B')
        // TODO : make this section of code less redundant, if possible
        if(c == 'A')
        {
          // up arrow, so recall latest command
          if(history_pos > 0)
          {
            history_pos--;
          }
          strcpy(input,cmd_history[history_pos]);
          pos = strlen(input);
        }
        else if(c == 'B')
        {
          // down arrow, so recover command inputted later than current ocmmand
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
    else if(c == new_line || c == carriage_return)
    {
      input[pos] = '\0';
      if(was_printed)
      {
        printf("\n");
      }
      else
      {
        printf("%s\n",input);
      }
      
      done = true;
    }
  }
  // set the termios back to what they originally were, to allow functionality of the rest 
  // of the shell once user input is successfully processed in
  tcsetattr(STDIN_FILENO,TCSAFLUSH,&ori_termios);

}

// TODO : empty function, implement only after I implement redirection property in cat()
/*
handles the redirection of terminal output into another file. redir_output holds the tokens that
are related to redirection
*/
void redir_output(char redir_input)
{

}

/*
stores old input entered by the user into history, and shows a list of old commands to the user
if typedef is invoked
cmd_history stores old cmds, history_index tracks what cmds to print, list indicates invokation 
of typedef, input[] is the user's input, and token is the rest of the input past "history"
*/
bool history(char* cmd_history[], int history_index,bool list, char input[], char* token)
{
  bool print = false;
  // if history cmd is invoked
  if(list)
  {
    int history_limit = 0;
    token = strtok(NULL," ");
    // assumes that the next token is a number
    // TODO : don't assume, put an error check here for the user
    if(token != NULL)
    {
      history_limit = atoi(token);
      history_limit++;
    }

    // list out the history 
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
/*
echos the input given by the user into the terminal; also 
supports single quotes. token contains remaining input after "echo", and input 
contains untouched user input
*/
bool echo(char* token, char input[])
{
  bool print = false;
  char echo[STR_BUFFER_SIZE];
  char* output_args[STR_BUFFER_SIZE];
  echo[0] = '\0';
  char echo_input[STR_BUFFER_SIZE];

  strcpy(echo_input, input);
  token = strtok(echo_input, " ");
  token = strtok(NULL, " ");

  // eliminates single quotes from the input; prints out as normal if not successful
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

/*
currently WIP. only setup is complete
*/
bool cat(char* token, char input[])
{
  bool print = false;
  char cat[STR_BUFFER_SIZE];
  char* output_args[STR_BUFFER_SIZE];
  cat[0] = '\0';
  char cat_input[STR_BUFFER_SIZE];

  strcpy(cat_input, input);
  token = strtok(cat_input, " ");
  token = strtok(NULL, " ");

  // TODO : fix single quote support for cat args
  print = single_quote(output_args,cat,token,input);

  // so now I want to loop through the tokens
  // open the file
  // read contents 
  // direct to stdout ( should be a general case, 
  as stdout should be modified if redir tokens exist)
}

/*
checks if the given args is a valid PATH. 
ind_path holds the token for the path, and token holds the user given cmd
*/
bool is_path(char ind_path[], char *token)
{
  // is it a PATH?
  bool print = false;
  char *path = getenv("PATH");
  // path isn't null and path isn't an empty string
  if (path != NULL && path[0] != '\0')
  {
    // copy it over to consume the tokens
    char token_path[STR_BUFFER_SIZE];
    strcpy(token_path, path);
    // meant to hold extracted paths
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
      }
      // keep going until they are all checked, null out ind path first

      path_token = strtok(NULL, ":");
    }
    // failed if fell through here, default into usual type case
    return print;
  }
}
/*
changes directories given a directory by the user, provided through token
*/
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
/*
includes support for relative paths, using token as the user's desired dir to change to
*/
void cd_relative(char* token)
{
  char tokenize_cwd[STR_BUFFER_SIZE];
  tokenize_cwd[0] = '\0';
  getcwd(tokenize_cwd, sizeof(tokenize_cwd));


  char cwd_tokens[NUM_TOKENS][STR_BUFFER_SIZE];
  memset(cwd_tokens,0,sizeof(cwd_tokens));
  char* token_cwd = strtok(tokenize_cwd, "/");
  int num_cwd_tokens = 0;

  // tokenize the cwd 
  while(token_cwd != NULL )
  {

      strcpy(cwd_tokens[num_cwd_tokens],token_cwd);
      token_cwd = strtok(NULL,"/"); 
      num_cwd_tokens++;
    
  }

  int num_rel_tokens = 0;
  char tokenize_rel[STR_BUFFER_SIZE];
  tokenize_rel[0] = '\0';
  strcpy(tokenize_rel,token); // token is going to be holding the whole path

  char rel_tokens[NUM_TOKENS][STR_BUFFER_SIZE]; 
  memset(rel_tokens,0,sizeof(rel_tokens));
  char* token_rel = strtok(tokenize_rel,"/");

  // tokenize the rel path tokens
  while(token_rel != NULL)
  {
      strcpy(rel_tokens[num_rel_tokens],token_rel);
      token_rel = strtok(NULL,"/");
      num_rel_tokens++;
    
  }

  char new_path[STR_BUFFER_SIZE];
  new_path[0] = '\0';
  int index = 0;

  // compares rel token and absolute tokens to  piece together the new path depending on 
  // what the rel token shorthand indicates
  while(index < num_rel_tokens)
  {
    // move up to the parent directory, get rid of child dir token
    if(strcmp(rel_tokens[index],"..") == 0)
    {
      strcpy(cwd_tokens[num_cwd_tokens-1],"\0");
      num_cwd_tokens--;
    }
    // dir name that is NOT just the cwd, append to path
    else if(strcmp(rel_tokens[index],".") != 0)
    {
      strcpy(cwd_tokens[num_cwd_tokens],rel_tokens[index]);
      num_cwd_tokens++;
    }
    index++;
  }

  index = 0;
  strcat(new_path,"/");

  // put the new path together
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
  
  if(chdir(new_path) != 0)
  {
    printf("cd: %s: No such file or directory\n", token);
  }
    
}

/*
extracts single quotes from the input, allowing the terminal to read the input as it would 
normally
output_args holds the new args for cat, output holds the new output, token holds the clipped input,
and input[] holds the user input
*/
bool single_quote(char* output_args[], char output[], char* token, char input[])
{
  output[0] = '\0';
  bool print = false;
  if(token[0] == '\'')
  {
    // need to retokenize it differently based on ' 
    char quote_input[STR_BUFFER_SIZE];
    strcpy(quote_input,input);
    int quote_input_index = 0;
    int output_index = 0;

    // set up for ensuring that quotes are extracted cleanly
    char arg_arr[STR_BUFFER_SIZE];
    int arg_index = 0;
    int arg_string_index = 0;
    bool single_quote_mode = false;
    bool closed_quote = false;
    int quote_input_max = strlen(quote_input);

    while( quote_input_index < quote_input_max)
    {
      // haven't found quote and mode isn't activated yet
      if(single_quote_mode)
      {
        // looks like an unnecessary check, but it's a fencepost solution T-T
        if(quote_input[quote_input_index] != '\'')
        {
          output[output_index] = quote_input[quote_input_index];
          output_index++;
        }

      }
      // found a quote in the input, extract it out
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
      // specific case needed to conserve spaces in original input 
      else if(!single_quote_mode && closed_quote && quote_input[quote_input_index] == ' ')
      {
        output[output_index] = ' ';
        output_index++;
        closed_quote = false;
      }
      // if we haven't found a quote and mode isn't activated, keep going
      quote_input_index++;
    }

    output[output_index] = '\0';
    printf("%s\n", output);
    print = true;

  }

  return print;
}

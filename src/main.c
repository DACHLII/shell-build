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
    

    if(strcmp(input, "exit 0"))
    {
      running = false;
      printf("reached exit 0");
      exit(0);
    }
    else if(input, "exit 1") {
      printf("reached exit 1");
      exit(1);
    }
    else
    {
      printf("reached else");
      printf("%s: command not found\n", input);
    }
  
    setbuf(stdout, NULL);

  }
  


  return 0;
}

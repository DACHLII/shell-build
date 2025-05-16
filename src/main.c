#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  // Uncomment this block to pass the first stage
 printf("$ ");

  // REPL loop
  // Wait for user input
  char input[100];
  while( input != "exit")
  {
    //read
    fgets(input, 100, stdin);
    // null terminate the input
    input[strlen(input) - 1] = '\0';
    printf("%s: command not found\n", input);
    setbuf(stdout, NULL);

  }
  


  return 0;
}

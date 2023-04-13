/*************************************************************************
 * shell.c                                                               *
 *                                                                       *
 * Wendy Slattery                                                        *
 * Operating Systems                                          *
 * 09/08/20                                                              *
 *                                                                       *
 * Project 1: Tiny Shell                                                 *
 * Develop a command line interpreter that may be run in interactive and *
 * batch modes, incorporate multi-threading and end upon a quit command. *
 * - template starter based on mtu-shell.c by Dr. Gang-Ryung Uh          *
 * - parser starter tutorial by Derek Yohn                               *
 *************************************************************************/

/* ----------------------------------------------------------------- */
/* PROGRAM  shell.c                                                  */
/*    This program reads in an input line, parses the input line     */
/* into tokens, and use execvp() to execute the command.             */
/* ----------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define MAXCMD 5
#define MAXARGS 5
#define MAXLINE 512
#define MAXLINEPLUS 513

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int exit_flag = 0;

char *com[MAXARGS];
char *cmds[MAXCMD];
char line[MAXLINEPLUS];

/* helper functions */

// functions to flush buffers
void reset_com() {
  memset(com, 0, MAXARGS);
}

void reset_cmds() {
  memset(cmds, 0, MAXCMD);
}

void reset_line() {
  memset(line, 0, MAXLINEPLUS);
}

// methods which loop until a null and no more coms in list
void dump_com() {
  int i;
  for(i = 0; com[i]; ++i) {
    printf("  com[%d] = %s\n", i, com[i]);
  }
}

void dump_cmds() {
  int i;
  for(i = 0; cmds[i]; ++i){
    printf("  cmds[%d] = %s\n", i, cmds[i]);
  }
}

/* ----------------------------------------------------------------- */
/* FUNCTION execute:                                                 */
/*    This function receives a command line argument list with the   */
/* first one being a file name followed by its arguments.  Then,     */
/* this function forks a child process to execute the command using  */
/* system call execvp().                                             */
/* ----------------------------------------------------------------- */

void  execute(char **argv)
{
    pid_t  pid;
    int    status;

    if ((pid = fork()) < 0) {     /* fork a child process           */
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    }
    else if (pid == 0) {                 /* for the child process:*/
        if (execvp(*argv, argv) < 0) {     /*   execute the command */
            printf("*** ERROR: exec failed\n");
            exit(1);
        }
    }
    else {                               /* for the parent:       */
        while (wait(&status) != pid);       /*   wait for completion */
        printf("PID %d exited with status %d\n", pid, status);

    }
}


/* ----------------------------------------------------------------- */
/* FUNCTION  parse:                                                  */
/*    This function takes an input line and parse it into tokens.    */
/* It first replaces all white spaces with zeros until it hits a     */
/* non-white space character which indicates the beginning of an     */
/* argument.  It saves the address to argv[], and then skips all     */
/* non-white spaces which constitute the argument.                   */
/* ----------------------------------------------------------------- */

void  parse(char *line, char **argv)
{
  /* Parsing variables */
  char *token = NULL;
  char *ptr = NULL;
  char *endptr = NULL;

  /* Delimiters  */
  static char *cmd_separator = ";";
  static char *delimiter = " \t\n";

  int i = 0;
  int j = 0;


  /* Pre-process the line to keep c-string intact */
  endptr = strchr(line, 0);  // find the end of string
  ptr = strchr(line, 34);    // find first double quote
  if(ptr != NULL && *ptr != 34) {
    if(*ptr == 32) *ptr = -2;  // convert spaces to special masking character
    ++ptr;
  }

  /* Tokenize first round - collect command strings */
  i = 0;
  token = strtok(line, cmd_separator);

  if(strcmp(token, "quit") == 0)
     exit_flag = 1;

  while (token != NULL) {
    if(strcmp(token, " ") != 0) {
      cmds[i] = token;
      //printf("cmds[%d]: %s\n", i, cmds[i]);
      ++i;
    }
    token = strtok(NULL, cmd_separator);
  } // end first tokenization while
  cmds[i] = NULL;  // null terminate final position to ensure ending




  /* second tokenization - process command strings */
  for(i = 0; cmds[i]; ++i) {
    j = 0;
    //printf("Processing command '%s'...\n", cmds[i]);
    token = strtok(cmds[i], delimiter);

    if(strcmp(token, "quit") == 0)
      exit_flag = 1;

    while(token != NULL) {
      com[j++] = token;
      token = strtok(NULL, delimiter);
    } // end second tokenization while
    com[j] = NULL;    // terminate final position

    //printf("Parsed command = \n");
    //dump_com();

    execute(com);

    reset_com();
    //printf("done.\n\n");
  }// end cmds parse for

  *argv = (char *)'\0';            /* mark the end of argument list  */

}


/* ----------------------------------------------------------------- */
/*                  The main program starts here                     */

int main(int argc, char *argv[])
{
  char  *c = NULL;
  _Bool is_batch = argc > 1;

  reset_com();
  reset_cmds();
  reset_line();


  if(is_batch)
  {
    printf("Entered Batch Mode\n\n");
      FILE *infile = fopen(argv[1], "r");
      if(infile == NULL) {
          perror("Error, there is no input file.\n");
          exit(1);
      }

      while (fgets(line, sizeof(line), infile)) {
          /* check for overflow  */
          if(strlen(line) > MAXLINE) {
              perror("Input line exceeds maximum length. Command ignored. Retry. \n");
              printf("Shell -> ");
              continue;
          }

          /* skip lines that start with whitespace, null, or return */
          if(isspace(line[0]) || line[0] == 0 || line[0] == 13) {
              perror("Input line does not start with character. Command ignored. Retry. \n");
              printf("Shell -> ");
              continue;
          }


          line[strlen(line) - 1] = '\0';       // remove the trailing \n

          parse(line, com);


          reset_line();
          reset_cmds();
          reset_com();
      }

      fclose(infile);
  }

  else                                                  /* interactive mode  */
  {
      printf("Shell -> ");                      /* display a prompt*/
      while (fgets(line, sizeof(line), stdin)) {       /* repeat until EOF... */

          /* check for overflow  */
          if(strlen(line) > MAXLINE) {
              perror("Input line exceeds maximum length. Command ignored. Retry. \n");
              printf("Shell -> ");
              continue;
          }

          /* skip lines that start with whitespace, null, or return */
          if(isspace(line[0]) || line[0] == 0 || line[0] == 13) {
              perror("Input line does not start with character. Command ignored. Retry. \n");
              printf("Shell -> ");
              continue;
          }

          parse(line, com);                  /*   parse the line     */

          if (strcmp(com[0], "exit") == 0)  /* is it an "exit"?     */
              exit(0);               /*    exit if it is     */


          reset_line();
          reset_cmds();
          printf("Shell -> ");       /*   display a prompt             */
      }
  }

  return 0;
}

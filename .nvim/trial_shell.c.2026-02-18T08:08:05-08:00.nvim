#define _POSIX_C_SOURCE 200809L

#include "msgs.h"
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define CWD_SIZE 4000
#define MAX_ARGS 128

// general error messages to use.

const char *help_msg = FORMAT_MSG("help", HELP_HELP_MSG);
const char *exit_msg = FORMAT_MSG("exit", EXIT_HELP_MSG);
const char *pwd_msg = FORMAT_MSG("pwd", PWD_HELP_MSG);
const char *cd_msg = FORMAT_MSG("cd", CD_HELP_MSG);

volatile sig_atomic_t sigint_received = 0;

// function to be signal handler
// when called ctrl+c
void sigint_handler(int sig) {
  (void)sig; // unused parameter suppresed
  sigint_received = 1;
  write(STDERR_FILENO, "\n", 1);

  // display information instead of ctrl+c
  write(STDOUT_FILENO, help_msg, strlen(help_msg));
  write(STDOUT_FILENO, exit_msg, strlen(exit_msg));
  write(STDOUT_FILENO, pwd_msg, strlen(pwd_msg));
  write(STDOUT_FILENO, cd_msg, strlen(cd_msg));

  char cwd[CWD_SIZE];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    write(STDOUT_FILENO, cwd, strlen(cwd));
  }
  write(STDOUT_FILENO, "$ ", 2);
}

void shell(void) {

  char cwd[CWD_SIZE];
  char *line = NULL;
  size_t len = 0;
  char prev_dir[CWD_SIZE] = "";

  // signint handler
  struct sigaction sa;
  sa.sa_handler = sigint_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    write(STDERR_FILENO, "FAiled to setup signint\n", 24);
    return;
  }

  // infinite loop
  while (1) {

    // clean up background process like sleep
    int status;
    // WNOHNAG return value from MAN page: return value depends on child process
    while (waitpid(-1, &status, WNOHANG) > 0) {
      // remove completed background process
    }

    // check current path
    if (!sigint_received) {

      if (getcwd(cwd, sizeof(cwd)) != NULL) {
        write(STDOUT_FILENO, cwd, strlen(cwd));
        write(STDERR_FILENO, "$ ", 2);
      }
      // reset flag
      sigint_received = 0;

      // read input
      ssize_t nread = getline(&line, &len, stdin);

      if (nread == -1) {
        // write(STDOUT_FILENO, "\n", 1);

        if (feof(stdin)) {
          break;
        }

        if (errno == EINTR) {
          clearerr(stdin); // clear error state
          continue;
        }
        break;
      }
      // strip new line
      if (nread > 0 && line[nread - 1] == '\n') {
        line[nread - 1] = '\0';
        nread--;
      }
      if (nread == 0) {
        continue;
      }

      // tokenization with strtok_r
      char *args[MAX_ARGS];
      char *token;
      char *saveptr;
      int arg_c = 0;
      // background process
      int background = 0;

      token = strtok_r(line, " \t", &saveptr);
      while (token != NULL && arg_c < MAX_ARGS - 1) {

        if (strcmp(token, "&") == 0) {
          background = 1;
          break;
        }

        // check if & is at the end of a token
        int token_length = strlen(token);

        if (token_length > 0 && token[token_length - 1] == '&') {
          background = 1;
          // remove &
          token[token_length - 1] = '\0';

          if (strlen(token) > 0) {
            args[arg_c++] = token;
          }
          break;
        }

        args[arg_c++] = token;
        token = strtok_r(NULL, " \t", &saveptr);
      }

      args[arg_c] = NULL;

      //// -------------------------------------
      // start of checking TASK 1 Commands
      if (arg_c == 0) {
        continue;
      }

      // background mode
      // if (background) {
      //   write(STDOUT_FILENO, "NOTE: BACKGROUND MODE \n", 23);
      //  }

      // TASK 2: Internal COMMANDS
      // Need to identify before fork, but after Parsing.

      // Check exit
      // usings strcmp check each command

      if (strcmp(args[0], "exit") == 0) {

        // if too many arguments handle error
        if (arg_c > 1) {
          write(STDERR_FILENO, "exit: too many arguments\n", 25);
          continue;
        }

        free(line);
        return;
      }

      // PWD IMPLEMENTATON
      // pwd
      if (strcmp(args[0], "pwd") == 0) {
        // check if arguments given
        if (arg_c > 1) {
          // display error, temp later.. change with msg.h error
          write(STDERR_FILENO, "pwd: too many arguments.\n", 25);
          continue;
        }

        // get current directory
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
          // failed
          write(STDERR_FILENO, "pwd: failed\n", 12);
        } else {
          // display with write
          write(STDOUT_FILENO, cwd, strlen(cwd));
          write(STDOUT_FILENO, "\n", 1);
        }
        continue;
      }

      // ---------------------------------------------------------
      // CD IMPLEMENTATION
      // check cd

      if (strcmp(args[0], "cd") == 0) {
        if (arg_c > 2) {
          write(STDERR_FILENO, FORMAT_MSG("cd", TMA_MSG),
                strlen(FORMAT_MSG("cd", TMA_MSG)));
          continue;
        }

        char curr_dir[CWD_SIZE];
        if (getcwd(curr_dir, sizeof(curr_dir)) == NULL) {
          // getcwd error msg
          write(STDERR_FILENO, FORMAT_MSG("cd", GETCWD_ERROR_MSG),
                strlen(FORMAT_MSG("cd", GETCWD_ERROR_MSG)));
          continue;
        }

        char targeted_dir[CWD_SIZE];

        // case 1: cd with no arguments -> go to home

        if (arg_c == 1) {
          uid_t uid = getuid();
          struct passwd *pw = getpwuid(uid);

          if (pw == NULL || pw->pw_dir == NULL) {
            write(STDERR_FILENO, FORMAT_MSG("cd", "Unable to access user info"),
                  strlen(FORMAT_MSG("cd", "Unable to access user info")));
            continue;
          }

          strcpy(targeted_dir, pw->pw_dir);

        }

        // case 2: ~

        else if (args[1][0] == '~') {
          uid_t uid = getuid();
          struct passwd *pw = getpwuid(uid);

          if (pw == NULL || pw->pw_dir == NULL) {
            write(STDERR_FILENO, FORMAT_MSG("cd", "Unable to access user info"),
                  strlen(FORMAT_MSG("cd", "Unable to access user info")));
            continue;
          }

          if (strlen(args[1]) == 1) {
            strcpy(targeted_dir, pw->pw_dir);
          }

          else if (args[1][1] == '/') {
            strcpy(targeted_dir, pw->pw_dir);
            // concatentate string with strcat() , copy with strcpy
            strcat(targeted_dir, args[1] + 1);
          } else {
            write(STDERR_FILENO, FORMAT_MSG("cd", "Invalid Path!"),
                  strlen(FORMAT_MSG("cd", "Invalid Path!")));
            continue;
          }
        }

        // case 3: -
        else if (strcmp(args[1], "-") == 0) {
          if (strlen(prev_dir) == 0) {
            write(STDERR_FILENO, FORMAT_MSG("cd", "No previous directory"),
                  strlen(FORMAT_MSG("cd", "No previous directory")));
            continue;
          }
          strcpy(targeted_dir, prev_dir);
        }

        // case 4: else
        else {
          strcpy(targeted_dir, args[1]);
        }

        // change directory
        if (chdir(targeted_dir) != 0) {
          write(STDERR_FILENO, FORMAT_MSG("cd", CHDIR_ERROR_MSG),
                strlen(FORMAT_MSG("cd", CHDIR_ERROR_MSG)));
        } else {
          strcpy(prev_dir, curr_dir);
        }
        continue;
      }
      // HELP Implementation  TASK 2;
      if (strcmp(args[0], "help") == 0) {
        // check if invalid argument
        if (arg_c > 2) {
          const char *msg = FORMAT_MSG("help", TMA_MSG);
          write(STDERR_FILENO, msg, strlen(msg));
          continue;
        }

        const char *help_msg = FORMAT_MSG("help", HELP_HELP_MSG);
        const char *exit_msg = FORMAT_MSG("exit", EXIT_HELP_MSG);
        const char *pwd_msg = FORMAT_MSG("pwd", PWD_HELP_MSG);
        const char *cd_msg = FORMAT_MSG("cd", CD_HELP_MSG);
        if (arg_c == 1) {
          write(STDOUT_FILENO, help_msg, strlen(help_msg));
          write(STDOUT_FILENO, exit_msg, strlen(exit_msg));
          write(STDOUT_FILENO, pwd_msg, strlen(pwd_msg));
          write(STDOUT_FILENO, cd_msg, strlen(cd_msg));
        }

        // check arguments for each case
        else {
          if (strcmp(args[1], "help") == 0) {
            write(STDOUT_FILENO, help_msg, strlen(help_msg));
          } else if (strcmp(args[1], "exit") == 0) {
            write(STDOUT_FILENO, exit_msg, strlen(exit_msg));
          } else if (strcmp(args[1], "pwd") == 0) {
            write(STDOUT_FILENO, pwd_msg, strlen(pwd_msg));
          } else if (strcmp(args[1], "cd") == 0) {
            write(STDOUT_FILENO, cd_msg, strlen(cd_msg));
          } else {
            char buf[256];
            int len = snprintf(buf, sizeof(buf), "%s: %s\n", args[1],
                               EXTERN_HELP_MSG);
            if (len > 0) {
              write(STDOUT_FILENO, buf, len);
            }
          }
        }
        continue;
      }

      // -------- TASK 0

      if (arg_c > 0 && strcmp(args[arg_c - 1], "&") == 0) {
        background = 1;
        args[arg_c - 1] = NULL;
        arg_c--;
      }

      // debug
      //   write(STDOUT_FILENO, "DEBUG: fork begining", 20);
      //    write(STDOUT_FILENO, args[0], strlen(args[0]));

      pid_t cpid = fork();

      if (cpid < 0) {
        // display parsed tokens
        // shell error
        const char *msg = FORMAT_MSG("shell", FORK_ERROR_MSG);
        write(STDERR_FILENO, msg, strlen(msg));
        continue;
      }

      // child process
      if (cpid == 0) {

        setpgid(0, 0);
        // list arguments from system call exec in a array of args
        execvp(args[0], args);

        const char *msg = FORMAT_MSG("shell", EXEC_ERROR_MSG);
        write(STDERR_FILENO, msg, strlen(msg));

        exit(1);
      } else {
        // parent
        if (!background) {
          // wait for child
          pid_t result;
          int status;
          // retry if interuppted with signal
          do {
            result = waitpid(cpid, &status, 0);
          } while (result == -1 && errno == EINTR);

          if (result == -1) {
            const char *msg = FORMAT_MSG("shell", WAIT_ERROR_MSG);
            write(STDERR_FILENO, msg, strlen(msg));
          }
        }
      }
    }
    free(line);
  }
}

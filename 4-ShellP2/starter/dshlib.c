#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"
#include <errno.h>

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

void parse_input(char *input, cmd_buff_t *cmd);

static int last_rc = 0;

int exec_local_cmd_loop()
{
    cmd_buff_t cmd;

    // TODO IMPLEMENT MAIN LOOP

    while (1) {
        
        printf("%s", SH_PROMPT);

        // TODO IMPLEMENT parsing input to cmd_buff_t *cmd_buff

        char input[SH_CMD_MAX];
        if (fgets(input, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            return OK;
        }
        input[strcspn(input, "\n")] = '\0';
    
        if (strlen(input) == 0) {
            continue; 
        }

        parse_input(input, &cmd);


        // TODO IMPLEMENT if built-in command, execute builtin logic for exit, cd (extra credit: dragon)
        // the cd command should chdir to the provided directory; if no directory is provided, do nothing

        Built_In_Cmds builtin = match_command(cmd.argv[0]);
        if (builtin != BI_NOT_BI) {
            exec_built_in_cmd(&cmd);
            continue; 
        }

        // TODO IMPLEMENT if not built-in command, fork/exec as an external command
        // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"

        pid_t pid = fork();
        if (pid == 0) {
            execvp(cmd.argv[0], cmd.argv);
            int real_error = errno;
            perror("execvp failed"); 
            exit(real_error);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                last_rc = WEXITSTATUS(status);
                switch (last_rc) {
                    case 0:
                        break;
                    case ENOENT:
                        printf("Command not found in PATH\n");
                        break;
                    case EACCES:
                        printf("Permission denied\n");
                        break;
                    case EISDIR:
                        printf("Directory, not an executable\n");
                        break;
                    case ENOEXEC:
                        printf("File found, but not a valid executable format\n");
                        break;
                    case ENOTDIR:
                        printf("Path component, not a directory\n");
                        break;
                    case E2BIG:
                        printf("Too many arguments—argument list is too large\n");
                        break;
                    case ENOMEM:
                        printf("System out of memory—can't load new process\n");
                        break;
                    default:
                        printf("Unknown error (errno=%d)\n", last_rc);
                }
            } else {
                last_rc = 1;
            }

        } else {
            perror("fork failed");
        }
    }

    return OK;
}

// Reset the cmd_buff_t struct
int clear_cmd_buff(cmd_buff_t *cmd) {
    for (int i = 0; i < cmd->argc; i++) {
        free(cmd->argv[i]);  
    }
    cmd->argc = 0;
    return OK;
}

// Check for built in commands
Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, "exit") == 0) return BI_CMD_EXIT;
    if (strcmp(input, "cd") == 0) return BI_CMD_CD;
    if (strcmp(input, "rc") == 0) return BI_RC; 
    if (strcmp(input, "dragon") == 0) return BI_CMD_DRAGON;
    return BI_NOT_BI;
}
extern void print_dragon();

// Handle built-in commands
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (cmd->argc == 0) return BI_NOT_BI;  

    switch (match_command(cmd->argv[0])) {
        case BI_CMD_EXIT:
            exit(OK);
        
        case BI_CMD_CD:
            if (cmd->argc < 2) {
                return BI_EXECUTED;
            }
            if (chdir(cmd->argv[1]) != 0) {
                perror("cd failed");
            }
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                setenv("PWD", cwd, 1);
            }
            return BI_EXECUTED;
        

        case BI_RC: 
            printf("%d\n", last_rc);
            return BI_EXECUTED;
        
        case BI_CMD_DRAGON: 
            print_dragon();
            return BI_EXECUTED;

        default:
            return BI_NOT_BI;
    }
}


void parse_input(char *input, cmd_buff_t *cmd) {
    clear_cmd_buff(cmd);
    bool in_quotes = false;
    char *arg = malloc(strlen(input) + 1);
    int arg_index = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '"') {
            in_quotes = !in_quotes;
            continue;
        }

        if (!in_quotes && isspace(input[i])) {
            if (arg_index > 0) {
                arg[arg_index] = '\0';

                if (cmd->argc >= CMD_ARGV_MAX) {
                    printf("error: too many arguments (max %d)\n", CMD_ARGV_MAX);
                    free(arg);
                    return;
                }


                cmd->argv[cmd->argc++] = strdup(arg);
                arg_index = 0;
            }
            continue;
        }

        arg[arg_index++] = input[i];
    }

    if (arg_index > 0) {
        arg[arg_index] = '\0';

        if (cmd->argc >= CMD_ARGV_MAX) {
            printf("error: too many arguments (max %d)\n", CMD_ARGV_MAX);
            free(arg);
            return;
        }
        cmd->argv[cmd->argc++] = strdup(arg);
    }

    cmd->argv[cmd->argc] = NULL;
    free(arg);
}



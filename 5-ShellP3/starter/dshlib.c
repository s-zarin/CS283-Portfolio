#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"


static int last_rc = 0;
extern void print_dragon();
void parse_input(char *input, cmd_buff_t *cmd);

/* Helper functions for command buffer management */
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL) {
        return ERR_MEMORY;
    }
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int close_cmd_buff(cmd_buff_t *cmd_buff) {
    free_cmd_buff(cmd_buff);
    clear_cmd_buff(cmd_buff);
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    if (cmd_line == NULL || cmd_buff == NULL) {
        return ERR_MEMORY;
    }

    clear_cmd_buff(cmd_buff);
    
    if (alloc_cmd_buff(cmd_buff) != OK) {
        return ERR_MEMORY;
    }
    
    strcpy(cmd_buff->_cmd_buffer, cmd_line);
    
    char *token = strtok(cmd_buff->_cmd_buffer, " \t");
    while (token != NULL && cmd_buff->argc < CMD_ARGV_MAX - 1) {
        cmd_buff->argv[cmd_buff->argc++] = token;
        token = strtok(NULL, " \t");
    }
    
    cmd_buff->argv[cmd_buff->argc] = NULL;
    return OK;
}

void parse_input(char *input, cmd_buff_t *cmd) {
    clear_cmd_buff(cmd);
    bool in_quotes = false;
    char *arg = malloc(strlen(input) + 1);
    int arg_index = 0;

    while (*input && isspace((unsigned char)*input)) {
        input++;
    }

    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '"') {
            in_quotes = !in_quotes;
        } else if (!in_quotes && isspace((unsigned char)input[i])) {
            if (arg_index > 0) {
                arg[arg_index] = '\0';
                if (cmd->argc >= CMD_ARGV_MAX - 1) {
                    fprintf(stderr, "error: too many arguments (max %d)\n", CMD_ARGV_MAX - 1);
                    free(arg);
                    return;
                }
                cmd->argv[cmd->argc++] = strdup(arg);
                arg_index = 0;
            }
        } else {
            arg[arg_index++] = input[i];
        }
    }

    if (arg_index > 0) {
        arg[arg_index] = '\0';
        if (cmd->argc >= CMD_ARGV_MAX - 1) {
            fprintf(stderr, "error: too many arguments (max %d)\n", CMD_ARGV_MAX - 1);
            free(arg);
            return;
        }
        cmd->argv[cmd->argc++] = strdup(arg);
    }

    cmd->argv[cmd->argc] = NULL;
    free(arg);
}


int build_cmd_list(char *cmd_line, command_list_t *clist) {
    if (!cmd_line || !clist) {
        return ERR_MEMORY;
    }

    clist->num = 0;

    char *cmd_copy = strdup(cmd_line);
    if (!cmd_copy) {
        return ERR_MEMORY;
    }

    char *cmd_part = strtok(cmd_copy, "|");

    while (cmd_part != NULL && clist->num < CMD_MAX) {
        while (isspace((unsigned char)*cmd_part)) {
            cmd_part++;
        }

        char *end = cmd_part + strlen(cmd_part) - 1;
        while (end > cmd_part && isspace((unsigned char)*end)) {
            *end-- = '\0';
        }

        if (strlen(cmd_part) > 0) {
            cmd_buff_t *this_cmd = &clist->commands[clist->num];
            clear_cmd_buff(this_cmd);

            parse_input(cmd_part, this_cmd);
            if (this_cmd->argc > 0) {
                clist->num++;
            }
        }

        cmd_part = strtok(NULL, "|");
    }

    free(cmd_copy);

    if (clist->num == 0) {
        return WARN_NO_CMDS;
    }

    return OK;
}


int free_cmd_list(command_list_t *clist) {
    for (int i = 0; i < clist->num; i++) {
        cmd_buff_t *cmd = &clist->commands[i];
        for (int j = 0; j < cmd->argc; j++) {
            free(cmd->argv[j]);
        }
        clear_cmd_buff(cmd);
    }
    clist->num = 0;
    return OK;
}

/* Built-in command handling */
Built_In_Cmds match_command(const char *input) {
    if (input == NULL) {
        return BI_NOT_BI;
    }
    
    if (strcmp(input, EXIT_CMD) == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    } else if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    }
    
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (cmd == NULL || cmd->argc == 0 || cmd->argv[0] == NULL) {
        return BI_NOT_BI;
    }
    
    Built_In_Cmds builtin = match_command(cmd->argv[0]);
    
    switch (builtin) {
        case BI_CMD_EXIT:
            printf("exiting...\n");
            return BI_CMD_EXIT;
            
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
            
        case BI_CMD_DRAGON:
            print_dragon();
            return BI_EXECUTED;
            
        default:
            return BI_NOT_BI;
    }
}

/* Command execution functions */
int exec_cmd(cmd_buff_t *cmd) {
    if (cmd == NULL || cmd->argc == 0) {
        return WARN_NO_CMDS;
    }
    
    Built_In_Cmds builtin = exec_built_in_cmd(cmd);
    if (builtin == BI_EXECUTED) {
        return OK;
    } else if (builtin == BI_CMD_EXIT) {
        return OK_EXIT;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return ERR_EXEC_CMD;
    } else if (pid == 0) {
        execvp(cmd->argv[0], cmd->argv);
        perror("execvp failed");
        exit(errno);
    } else {
        int status;
        while (waitpid(-1, NULL, 0) > 0);
        
        if (WIFEXITED(status)) {
            last_rc = WEXITSTATUS(status);
        } else {
            last_rc = 1;
        }
    }
    
    return OK;
}

int execute_pipeline(command_list_t *clist) {
    if (clist == NULL || clist->num == 0) {
        return WARN_NO_CMDS;
    }

    int pipes[CMD_MAX-1][2];
    pid_t pids[CMD_MAX];

    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe creation failed");
            return ERR_EXEC_CMD;
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork failed");
            return ERR_EXEC_CMD;
        } else if (pids[i] == 0) {
            if (i > 0) {
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    perror("dup2 failed on stdin");
                    exit(1);
                }
            }

            if (i < clist->num - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 failed on stdout");
                    exit(1);
                }
            }

            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp failed");
            exit(1);
        }
    }

    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    int last_status = 0;
    for (int i = 0; i < clist->num; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (i == clist->num - 1) {
            if (WIFEXITED(status)) {
                last_status = WEXITSTATUS(status);
            } else {
                last_status = 1;
            }
        }
    }

    return last_status;
}



/* Main command loop */
int exec_local_cmd_loop() {
    char cmd_line[SH_CMD_MAX];
    command_list_t cmd_list;
    int rc;
    
    while (1) {

        if (!fgets(cmd_line, sizeof(cmd_line), stdin)) {
            fprintf(stderr, "\n");
            break;
        }
        
        cmd_line[strcspn(cmd_line, "\n")] = '\0';
        
        if (strlen(cmd_line) == 0) {
            continue;
        }
        
        if (strcmp(cmd_line, EXIT_CMD) == 0) {
            printf("exiting...\n");
            return OK;
        }
        
        rc = build_cmd_list(cmd_line, &cmd_list);
        
        if (rc == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD);
            continue;
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        } else if (rc != OK) {
            printf("Error parsing command: %d\n", rc);
            continue;
        }
        
        Built_In_Cmds builtin = exec_built_in_cmd(&cmd_list.commands[0]);
        if (builtin == BI_EXECUTED) {
            free_cmd_list(&cmd_list);
            continue;
        } else if (builtin == BI_CMD_EXIT) {
            free_cmd_list(&cmd_list);
            fprintf(stderr, "%s", SH_PROMPT);
            return OK;
        } else {
            rc = execute_pipeline(&cmd_list);
            
            if (rc == OK_EXIT) {
                free_cmd_list(&cmd_list);
                fprintf(stderr, "%s", SH_PROMPT);
                return OK;
            } else if (rc != OK) {
                printf("Error executing command: %d\n", rc);
            }
        }

        fprintf(stderr, "%s", SH_PROMPT);

        free_cmd_list(&cmd_list);
    }
    fprintf(stderr, "%s", SH_PROMPT);

    return OK;
}



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if (strlen(cmd_line) == 0)
    {
        return WARN_NO_CMDS;
    }

    memset(clist, 0, sizeof(command_list_t));

    char *token;
    char *rest = cmd_line;
    int count = 0;

    while ((token = strsep(&rest, PIPE_STRING)) != NULL)
    {
        if (count >= CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }

        token += strspn(token, " ");
        if (*token == '\0') continue;

        char *cmd_copy = strdup(token);
        if (!cmd_copy)
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        char *arg_pos = strchr(cmd_copy, ' ');
        if (arg_pos)
        {
            *arg_pos = '\0';
            arg_pos++;
            while (*arg_pos == ' ') arg_pos++;
        }

        strncpy(clist->commands[count].exe, cmd_copy, EXE_MAX - 1);
        clist->commands[count].exe[EXE_MAX - 1] = '\0';

        if (arg_pos && *arg_pos)
        {
            strncpy(clist->commands[count].args, arg_pos, ARG_MAX - 1);
            clist->commands[count].args[ARG_MAX - 1] = '\0';
        }
        else
        {
            strcpy(clist->commands[count].args, "");
        }

        free(cmd_copy);
        count++;
    }

    clist->num = count;
    return OK;
}
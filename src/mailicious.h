#ifndef MAILICIOUS_H
#define MAILICIOUS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "parser.h"
#include "emfs.h"
#include "utils.h"
#include "env.h"

/*command line max length*/
#define CMD_MAX_LEN 300

/*return by user command function*/
#define CMD_SUCCESSFUL 1
#define CMD_ABORTED 0

/*used in import*/
#define ACCEPT_WARNING 1;
#define NO_WARNING 0;

void load_conf_data();
void init_program_data(char *_arg0);
int load_mailing_list(char *_list_name, mailing_list_t *_mailing_list);
int list();
int import_email(char **_args, int _argc);
int export_email(char **_args, int _argc);
int search(char **_args, int _argc);
int update(char **_args, int _argc);
int delete_email(char **_args, int _argc);
int remove_mailing_list(char **_args, int _argc);
int ping(char **_args, int _argc);
int help();
int execute_command(int _argc, char **_args);

#endif
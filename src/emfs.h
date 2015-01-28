#ifndef EMFS_H
#define EMFS_H

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /*need by ftruncate*/
#include <sys/types.h> /*need by ftruncate*/
#include <sys/stat.h>
#include <dirent.h>
#include "utils.h"
#include "env.h"

#define DB_PATH "db/"

#define INDEX_HASH_TABLE_SIZE 65640

#define LIST_CREATED 0
#define LIST_EXIST 1

#define NO_EMAIL 0

#define EMAIL_RECORD_FORMAT "%-254s\n%c\n%-10u\n%-10u\n"
#define EMAIL_FORMAT "%-254s\n"
#define WARN_FORMAT "%c\n"
#define OFFSET_FORMAT "%-10u\n"
#define OFFSET_FORMAT_LEN 11
#define WARN_RECORD_POS 255L
#define PREV_RECORD_POS 257L
#define NEXT_RECORD_POS 268L

#define WARN_CHAR '*'
#define NO_WARN_CHAR ' '

#define EMAIL_EXIST 0
#define EMAIL_ADDED 1

#define EMAIL_NOT_EXIST 0 
#define EMAIL_UPDATED 3

#define MAX 256
#define DATA_SIZE 2048
#define NO_HOST -2
#define NO_RESPONSE 0

typedef struct free_slot_t
{
	unsigned int pos;
	struct free_slot_t *next;
} free_slot_t;

typedef struct
{
	FILE *f_index;
	FILE *f_fslot;
	FILE *f_email;
	unsigned int index_table[INDEX_HASH_TABLE_SIZE];
	free_slot_t *free_slot;
} mailing_list_t;

typedef struct
{
	char email[RFC_EMAIL_MAX_LEN];
	char warning;
	unsigned int email_pos;
	unsigned int prev_email_pos;
	unsigned int next_email_pos;
} email_node_t;

typedef struct record_list_t
{
	unsigned int pos;
	struct record_list_t *next_pos;
} record_list_t;

typedef struct domain_list_t
{
	char *domain;
	record_list_t *record_list;
	struct domain_list_t *next_domain;
} domain_list_t;

unsigned char char_value(unsigned char _c);
unsigned int email_hash(char *_email);
int create_new_list(char *_list_name);
void load_list(mailing_list_t *_mailing_list, char *_list_name);
void close_list(mailing_list_t *_mailing_list);
void read_email_record(email_node_t *_email_node, mailing_list_t *_mailing_list, unsigned int _pos);
unsigned int write_email_record(mailing_list_t *_mailing_list, char *_email, char _warn, unsigned int _prev, unsigned int _next);
int add_email(mailing_list_t *_mailing_list, char *_email, char _warn);
void delete_email_record(mailing_list_t *_mailing_list, unsigned int _pos);
unsigned int search_email_record(mailing_list_t *_mailing_list, char *_email);
int print_list(mailing_list_t *_mailing_list, char *_dir_path, int _list_size, int _no_warn, char *_separator_token);
int update_email(mailing_list_t *_mailing_list, char *_old_email, char *_new_email, char _warn);
void delete_emails(mailing_list_t *_mailing_list, char *_delete_list_path);
void check_domain(mailing_list_t *_mailing_list, char *_out_name);
float ping_domain(char *domain);

#endif
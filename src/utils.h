#ifndef UTILS_H
#define UTILS_H

#define RFC_LP_MAX_LEN 64
#define RFC_DOM_MAX_LEN 252
#define RFC_EMAIL_MAX_LEN 254

#define LINE_MAX_LEN 300

/*character type costants*/
#define INVALID_CHAR -1
#define AT_CHAR 0
#define ALPHA_CHAR 1
#define NUM_CHAR 2
#define P_CHAR 3
#define HYP_CHAR 4
#define UND_CHAR 5

typedef struct email_t
{
	char *localpart;
	char *domain;
	int localpart_len;
	int domain_len;
} email_t;


email_t *create_email();
void init_email(email_t *_email);
char *get_email(char *_email_str, email_t *_email);
int char_analysis(char _email_char);
int strexp(char ***_args, int *_argc, char *_str, char *_delim);

#endif
#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "emfs.h"
#include "utils.h"
#include "env.h"

#define DOM_FILE_PATH "domain" /*domains file path*/
#define DOM_HASH_TABLE_SIZE 19683 /*27^3+27^2+27 (27 = a-z + ' ')*/

/*email part costants*/
#define LOCALPART 0
#define LAST_DOMAIN 1

/*localpart and domain lexical parse result costants*/
#define LP_CHAR_OK 1 /*char accepted*/
#define LP_COMPLETED 3 /*parse completed*/
#define LP_CHAR_NOK -1 /*char rejected*/
#define LP_TOO_LONG -3 /*exceeds maximum length*/
#define LP_IC_OUT -5 /*exceeds invalid char tolerance*/
#define LP_EMPTY -7 /*is empty*/
#define DOM_CHAR_OK 2 /*char accepted*/
#define DOM_COMPLETED 4 /*parse completed*/
#define DOM_CHAR_ND 6 /*found new subdomain*/
#define DOM_CHAR_NOK -2 /*char rejected*/
#define DOM_TOO_LONG -4 /*exceeds maximum length*/
#define DOM_IC_OUT -6 /*exceeds invalid char tolerance*/

/*syntactic analysis result*/
#define SYNTAX_CORRECTED 0 /*email was corrected*/
#define SYNTAX_OK 1 /*email was correct*/
#define INCORRECT_DOMAIN -1 /*the domain is incorrect*/

/*semantic analysis result*/
#define EMAIL_OK 1 /*email accepted*/
#define EMAIL_WARN 0 /*email accepted with warning*/
#define INVALID_EMAIL -1 /*email not accepted*/

/************************************************************************/
/*parser default configuration (used in case of error loading conf file)*/

/*localpart and domain invalid characters tolerance*/
#define LP_IC_TOLERANCE 0
#define DOM_IC_TOLERANCE 0

/*email average and warning length threshold*/
#define EMAIL_AVG_LEN 30
#define EMAIL_WARN_LEN 45

/*localpart average and warning char type counter threshold*/
#define LP_AVG_LEN 20
#define LP_WARN_LEN 30
#define LP_AVG_NUM 4
#define LP_WARN_NUM 5
#define LP_AVG_P 2
#define LP_WARN_P 3
#define LP_AVG_HYP 1
#define LP_WARN_HYP 2
#define LP_AVG_UND 1
#define LP_WARN_UND 2

/*domain average and warning char type counter threshold*/
#define DOM_AVG_LEN 16
#define DOM_WARN_LEN 30
#define DOM_AVG_NUM 2
#define DOM_WARN_NUM 4
#define DOM_AVG_HYP 1
#define DOM_WARN_HYP 2
#define DOM_AVG_UND 1
#define DOM_WARN_UND 1

/***********************************************************/


/*contains a information used by every parse step*/
typedef struct
{
	email_t *email; /*struct containing email*/
	unsigned char ec; /*last parsed char*/
	/*
	 *Indicates which part of email is being parsed: 0 for localpart;
	 *1 for last level domain; 2 for second-last and so on. Thus at the 
	 *end of parsing it indicates how many domain level the email contains
	 */
	int email_part;
	int first_dom_index; /*first char index of first level domain in email->domain*/
	int lp_ic; /*localpart invalid character counter*/
	int dom_ic; /*domain invalid character counter*/
	int lp_alpha; /*localpart alphabetics counter */
	int lp_num; /*localpart numbers counter*/
	int lp_p; /*localpart points counter*/
	int lp_hyp; /*localpart hyphens counter*/
	int lp_und; /*localpart underscores counter*/
	int dom_alpha; /*domain alphabetics counter*/
	int dom_num; /*domain numbers counter*/
	int dom_hyp; /*domain hyphens counter*/
	int dom_und; /*domain underscores counter*/
} parsing_data_t;

typedef struct
{
	unsigned int total_emails;
	unsigned int warned_emails;
	unsigned int correct_emails;
	unsigned int dup_emails;
} parsing_stat_t;


int load_domains();
void free_domain_table();
parsing_data_t *create_parsing_data();
void init_parsing_data(parsing_data_t *_p_data);
int email_lexical_analysis(parsing_data_t *_p_data);
int localpart_lexical_analysis(parsing_data_t *_p_data);
int domain_lexical_analysis(parsing_data_t *_p_data);
int email_syntactic_analysis(parsing_data_t *_p_data);
int email_semantic_analysis(parsing_data_t *_p_data);
parsing_stat_t parse(mailing_list_t *_mailing_list, FILE *_file, int _save_warnings);

#endif
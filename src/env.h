#ifndef ENV_H
#define ENV_H

#define PATH_MAX_LEN 256

#define ERROR 0
#define SUCCESS 1

typedef struct
{
	char work_dir[PATH_MAX_LEN]; /*if cwd != /bin*/
	char dbpath[PATH_MAX_LEN];
	
	int lp_ic_tolerance, dom_ic_tolerance;
	int email_avg_len, email_warn_len;

	int lp_avg_len, lp_warn_len;
	int lp_avg_num, lp_warn_num;
	int lp_avg_p, lp_warn_p;
	int lp_avg_hyp, lp_warn_hyp;
	int lp_avg_und, lp_warn_und;

	int dom_avg_len, dom_warn_len;
	int dom_avg_num, dom_warn_num;
	int dom_avg_hyp, dom_warn_hyp;
	int dom_avg_und, dom_warn_und;
} env_var_t; 

/*node of the collision list of domain hash table*/
typedef struct domain_node_t
{
	char *domain_name; /*first level domain name*/
	struct domain_node_t *next_domain; /*pointer to next node*/
} domain_node_t;

extern env_var_t env_var;
extern domain_node_t **domains_hash_table;

#endif
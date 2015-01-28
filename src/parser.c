#include "parser.h"

mailing_list_t *mailing_list;

int load_domains()
{
	/*
	 *This function reads a list of first level domains from a file and load them into 
	 *an hash table which is an array of pointers to a collision list.
	 *The hash is an integer calculated according to the lexicographical order of the first 
	 *three letters of the domain (for two letters domain the third char is 0).
	 */
	
	FILE *domain_list; /*file containg domains*/
	domain_node_t *new_domain; /*domain to add*/
	domain_node_t *aux;
	char read_domain[LINE_MAX_LEN]; /*string read in the file*/
	int hash; /*hash table index*/
	char path[PATH_MAX_LEN];
	int i;
	
	/*create and initialize table*/
	domains_hash_table = malloc(sizeof(domain_node_t *) * DOM_HASH_TABLE_SIZE);
	for(i = 0; i < DOM_HASH_TABLE_SIZE; i++)
		domains_hash_table[i] = NULL;
	
	memset(path, '\0', PATH_MAX_LEN);
	strcat(path, env_var.work_dir);
	strcat(path, "domains");
	if((domain_list = fopen(path, "r")) == NULL)
		return ERROR;

	/*read domains from file and insert them into the table*/
	while(fgets(read_domain, LINE_MAX_LEN, domain_list) != NULL)
	{
		read_domain[strlen(read_domain) - 1] = '\0';
		new_domain = malloc(sizeof(domain_node_t));
		new_domain->domain_name = strdup(read_domain);
		new_domain->next_domain = NULL;
		
		hash = (27 * 27 * (read_domain[0] - 'a' + 1)) + (27 * ((read_domain[1] - 'a' + 1)));
		if(strlen(read_domain) > 2)
			hash += read_domain[2] - 'a' + 1;
		
		/*insert new node top or bottom the list*/
		aux = domains_hash_table[hash];
		if(aux == NULL)
			domains_hash_table[hash] = new_domain;		
		else {
			while(aux->next_domain != NULL)
				aux = aux->next_domain;
			aux->next_domain = new_domain;
		}
	}
	
	fclose(domain_list);

	return SUCCESS;
}

void free_domain_table()
{
	domain_node_t *aux1, *aux2;
	int i;
	
	for(i = 0; i < DOM_HASH_TABLE_SIZE; i++)
	{
		aux1 = domains_hash_table[i];
		while(aux1 != NULL)
		{
			aux2 = aux1;
			aux1 = aux1->next_domain;
			free(aux2->domain_name);
			free(aux2);
		}
	}
}

parsing_data_t *create_parsing_data()
{
	/*istantiate the parsing data struct*/
	
	parsing_data_t *p_data;
	
	p_data = malloc(sizeof(parsing_data_t));
	p_data->email = create_email();
	return p_data;
}

void init_parsing_data(parsing_data_t *_p_data)
{
	/*initialize the parsing data struct*/
	
	init_email(_p_data->email);
	_p_data->ec = '\0';
	_p_data->email_part = 0;
	_p_data->lp_ic = 0;
	_p_data->dom_ic = 0;
	_p_data->lp_alpha = 0;
	_p_data->lp_num = 0;
	_p_data->lp_p = 0;
	_p_data->lp_hyp = 0;
	_p_data->lp_und = 0;
	_p_data->dom_alpha = 0;
	_p_data->dom_num = 0;
	_p_data->dom_hyp = 0;
	_p_data->dom_und = 0;
}

int email_lexical_analysis(parsing_data_t *_p_data)
{
	/*
	 *This function take a char in input and yield the result of its lexical analysis.
	 *It is meant to modify the parsing_data_t struct according to the outcomes of the 
	 *underlying functions localpart_lexical_analysis and domain_lexical_analysis.
	 *Even though this function could simply handle syntactic errors it doesn't, cause it's meant 
	 *only for lexical analysis. That's why it contains a little of not really clean code, in order
	 *to yield email without any lexical valid characther missing.
	 *Anyway it needs to perform a little of syntactical analysis, since this part have to check if
	 *the email is made up of localpart@domain, becasue otherwise it can't perform localpart and 
	 *domain separated invalid characters analysis.
	 */

	int res; /*outcome of localpart or domain analysis*/
	
	/*localpart parsing*/
	if(_p_data->email_part == LOCALPART)
	{
		res = localpart_lexical_analysis(_p_data);
		if(res == LP_CHAR_OK)
			_p_data->email->localpart[_p_data->email->localpart_len++] = _p_data->ec;
		else if(res == LP_CHAR_NOK)
		{
			if(_p_data->email->localpart_len > 0)
				_p_data->lp_ic++;
		}
		else if(res == LP_COMPLETED) /*parsing completed*/
		{
			_p_data->email->localpart[_p_data->email->localpart_len] = '\0';
			_p_data->email_part = LAST_DOMAIN;
		}
		else if(res == LP_IC_OUT || res == LP_TOO_LONG || res == LP_EMPTY)
			init_parsing_data(_p_data);
	}
	/*domain parsing*/
	else {
		res = domain_lexical_analysis(_p_data);
		if(res == DOM_CHAR_OK)
			_p_data->email->domain[_p_data->email->domain_len++] = _p_data->ec;
		else if(res == DOM_CHAR_NOK)
			_p_data->dom_ic++;
		else if(res == DOM_COMPLETED) /*if parsing completed*/
		{
			_p_data->email->domain[_p_data->email->domain_len] = '\0';
			/*this is to correct subdomains count in case of last char(s) of email is (are) '.'*/
			if(_p_data->email->domain[_p_data->email->domain_len - 1] == '.')
				_p_data->email_part--;
		}
		else if(res == DOM_CHAR_ND) /*if a new subdomain is detected*/
		{
			_p_data->email->domain[_p_data->email->domain_len++] = _p_data->ec;
			_p_data->email_part++; /*increment subdomain counter*/
		}
		else if(res == DOM_IC_OUT || res == DOM_TOO_LONG)
			init_parsing_data(_p_data);
	}
	return res;
}

int localpart_lexical_analysis(parsing_data_t *_p_data)
{
	/*
	 *This function doesn't affect the parsing_data_t struct values. Its goal is to 
	 *analyze the character given in input using informations contained in _p_data 
	 *to dertimine which action the above email_lexical_analysis function must perform.
	 */

	int ca_res;
	
	ca_res = char_analysis(_p_data->ec);

	/*if char is @ then localpart is complete (or empty)*/
	if(ca_res == AT_CHAR)
	{
		if( _p_data->email->localpart_len > 0)
			return LP_COMPLETED;
		else
			return LP_EMPTY;
	}
	else if(ca_res != INVALID_CHAR)
	{
		if(_p_data->email->localpart_len == RFC_LP_MAX_LEN)
			return LP_TOO_LONG;
		return LP_CHAR_OK;
	}
	/*if I've alreary reach invalid char threshold*/
	else if(_p_data->lp_ic == env_var.lp_ic_tolerance)
		return LP_IC_OUT;
	
	return LP_CHAR_NOK;
}

int domain_lexical_analysis(parsing_data_t *_p_data)
{
	/*Same of localpart_lexical_analysis*/

	int ca_res;

	ca_res = char_analysis(_p_data->ec);

	/*
	 *If I get an invalid char in a subdomain different from last level domain
	 *I consider it as the end of email without any tolerance.
	 */
	if(ca_res == INVALID_CHAR && _p_data->email_part > LAST_DOMAIN)
		return DOM_COMPLETED;
	/*otherwise if I am in the last level domain I could consider a certain tolerance*/
	else if(ca_res == INVALID_CHAR && _p_data->email_part == LAST_DOMAIN)
	{
		if(_p_data->dom_ic == env_var.dom_ic_tolerance)
			return DOM_IC_OUT;
		return DOM_CHAR_NOK;
	}
	else if(ca_res != INVALID_CHAR)
	{
		if(_p_data->email->domain_len == RFC_DOM_MAX_LEN)
			return DOM_TOO_LONG;
		/*if I get a '.' I have to be able to spot subdomains*/
		if(ca_res == P_CHAR)
		{
			if(_p_data->email->domain_len > 0 && _p_data->email->domain[_p_data->email->domain_len - 1] != '.')
				return DOM_CHAR_ND;
		}
		return DOM_CHAR_OK;
	}
	return DOM_CHAR_NOK;
}

int email_syntactic_analysis(parsing_data_t *_p_data)
{
	/*
	 *This function controls if a lexical valid email checks the email's good formation rules.
	 *If it doesn't it tries to resolve the errors.
	 *It performs the following actions:
	 * 1- It checks if localpart starts or ends with a period, or if there are two successive periods.
	 * 2- It checks if domain starts or ends with a period or an hypens, or if there are two successive 
	 *   periods or hyphens, or if an hyphen is next to a period.
	 * 3- It checks if the first level domain is made up only by alphabetics and at least two char long.
	 */
	
	int lp_real_len; /*localpart length without syntactical incorrect chars*/
	int dom_real_len; /*same for domain*/
	int i, aux1, aux2, ca_res;
	
	/*Check 1: use an index incremented only when I read a syntactical correct in order  
	  to discard invalid chars.*/			
	lp_real_len = 0;
	if(_p_data->email->localpart[0] != '.')
		lp_real_len++;
	for(i = 1; i < _p_data->email->localpart_len; i++)
	{
		/*if char is not a point or is a point which doesn't come after another point*/
		if(_p_data->email->localpart[i] != '.' || 
		  (_p_data->email->localpart[i] == '.' && _p_data->email->localpart[i - 1] != '.'))
			_p_data->email->localpart[lp_real_len++] = _p_data->email->localpart[i];
	}
	/*check last char*/
	if(_p_data->email->localpart[i - 1] == '.')
		lp_real_len--;
	aux1 = _p_data->email->localpart_len;
	/*update localpart length*/
	_p_data->email->localpart_len = lp_real_len;
	_p_data->email->localpart[_p_data->email->localpart_len] = '\0';
	
	/*Check 2: same thing as before but this time I have to consider also hyphen*/
	dom_real_len = 0;
	if(_p_data->email->domain[0] != '.' && _p_data->email->domain[0] != '-')
		dom_real_len++;
	for(i = 1; i < _p_data->email->domain_len; i++)
	{
		/*
		 *if char is not a point, or is a point which doesn't come after a point or an hyphen, 
		 *or is an hyphen which doesn't come after an hyper or a point
		 */
		if((_p_data->email->domain[i] != '.' && _p_data->email->domain[i] != '-') ||
		   (_p_data->email->domain[i] == '.' && _p_data->email->domain[i - 1] != '.' && _p_data->email->domain[i - 1] != '-') ||
		   (_p_data->email->domain[i] == '-' && _p_data->email->domain[i - 1] != '-' && _p_data->email->domain[i - 1] != '.'))
		{
			_p_data->email->domain[dom_real_len++] = _p_data->email->domain[i];
		}
		/*in order to consider cases like federico@gmail-.com*/
		if(_p_data->email->domain[i] == '.' && _p_data->email->domain[i - 1] == '-')
			_p_data->email->domain[dom_real_len - 1] = '.';
	}
	if(_p_data->email->domain[i - 1] == '.' || _p_data->email->domain[i - 1] == '-')
		dom_real_len--;
	aux2 = _p_data->email->domain_len;
	_p_data->email->domain_len = dom_real_len;
	_p_data->email->domain[_p_data->email->domain_len] = '\0';
	
	
	/*Check 3: check domain char type and domain length*/
	i = dom_real_len - 1;	
	while(_p_data->email->domain[i] != '.')
	{
		ca_res = char_analysis(_p_data->email->domain[i]);
		if(ca_res != ALPHA_CHAR)
			return INCORRECT_DOMAIN;
		i--;
	}
	
	if(_p_data->email->domain[_p_data->email->domain_len - 2] == '.')
		return INCORRECT_DOMAIN;
	
	
	/*I store it because I need it in the semantic analysis*/
	_p_data->first_dom_index = i + 1;
	/*I make distinction between email that was already corrected and not*/
	if(lp_real_len == aux1 && dom_real_len == aux2)
		return SYNTAX_OK;
	return SYNTAX_CORRECTED;
}

int email_semantic_analysis(parsing_data_t *_p_data)
{
	/*This function checks if a syntactical correct email could really exist.*/
	
	int warn;
	int hash;
	int i, dom_size, char_type;
	char dom[RFC_DOM_MAX_LEN];
	domain_node_t *aux;
	
	
	/*count every char type in localpart*/
	for(i = 0; i < _p_data->email->localpart_len; i++)
	{
		char_type = char_analysis(_p_data->email->localpart[i]);
		switch(char_type)
		{
			case ALPHA_CHAR : { _p_data->lp_alpha++; break; }
			case NUM_CHAR : { _p_data->lp_num++; break; }
			case P_CHAR : { _p_data->lp_p++; break; }
			case HYP_CHAR : { _p_data->lp_hyp++; break; }
			case UND_CHAR : { _p_data->lp_und++; }				
		}
	}
	
	/*count every char type in domain*/
	for(i = 0; i < _p_data->email->domain_len; i++)
	{
		char_type = char_analysis(_p_data->email->domain[i]);
		switch(char_type)
		{
			case ALPHA_CHAR : { _p_data->dom_alpha++; break; }
			case NUM_CHAR : { _p_data->dom_num++; break; }
			case HYP_CHAR : { _p_data->dom_hyp++; break; }
			case UND_CHAR : { _p_data->dom_und++; }				
		}
	}
	
	/*if one of the counters exceed the warn threshold consider email invalid*/
	if(_p_data->email->localpart_len > env_var.lp_warn_len ||
	   _p_data->email->domain_len > env_var.dom_warn_len ||
	   _p_data->email->localpart_len + _p_data->email->domain_len > env_var.email_warn_len ||
	   _p_data->lp_num > env_var.lp_warn_num ||
	   _p_data->lp_p > env_var.lp_warn_p ||
	   _p_data->lp_hyp > env_var.lp_warn_hyp ||
	   _p_data->lp_und > env_var.lp_warn_und ||
	   _p_data->dom_num > env_var.dom_warn_num ||
	   _p_data->dom_hyp > env_var.dom_warn_hyp ||
	   _p_data->dom_und > env_var.dom_warn_und)
		return INVALID_EMAIL;

	/*if one of the counter exceed the avg thresold consider email valid with warnings*/
	warn = 0;
	if(_p_data->email->localpart_len + _p_data->email->domain_len > env_var.email_avg_len ||
	   _p_data->email->localpart_len > env_var.lp_avg_len ||
	   _p_data->email->domain_len > env_var.dom_avg_len ||
	   _p_data->lp_num > env_var.lp_avg_num ||
	   _p_data->lp_p > env_var.lp_avg_p ||
	   _p_data->lp_hyp > env_var.lp_avg_hyp ||
	   _p_data->lp_und > env_var.lp_avg_und ||
	   _p_data->dom_num > env_var.dom_avg_num ||
	   _p_data->dom_hyp > env_var.dom_avg_hyp ||
	   _p_data->dom_und > env_var.dom_avg_und)
		warn = 1;
		
	/*gets first level domain calcute domain hash and search it in the domain table*/
	dom_size = _p_data->email->domain_len - _p_data->first_dom_index;
	for(i = 0; i < dom_size; i++)
		dom[i] = _p_data->email->domain[_p_data->first_dom_index + i];
	dom[i] = '\0';
	
	hash = (27 * 27 * (dom[0] - 'a' + 1)) + (27 * (dom[1] - 'a' + 1));
	if(dom_size > 2)
		hash += dom[2] - 'a' + 1;

	/*search domain into the table*/
	aux = domains_hash_table[hash];
	if(aux == NULL)
		return INVALID_EMAIL;
	while(aux != NULL)
	{
		if(strcmp(aux->domain_name, dom) == 0)
		{
			if(warn == 0)
				return EMAIL_OK;
			return EMAIL_WARN;
		}
		aux = aux->next_domain;
	}
	return INVALID_EMAIL;
}

parsing_stat_t parse(mailing_list_t *_mailing_list, FILE *_file, int _save_warnings)
{
	/*
	 *This function reads the file pointed by _file and appling different analysis 
	 *steps recognizes emails into the file and save them into the program database 
	 *pointed by _mailing_list. The flag _save_warnings is used to include/exclude 
	 *emails with warning. 
	 */
	
	parsing_data_t *p_data;
	parsing_stat_t p_stat;
	char email_str[RFC_EMAIL_MAX_LEN];
	int read_char;
	int res;
	int ok_email_c, warn_email_c, dup_email_c;
	
	ok_email_c = warn_email_c = dup_email_c = 0;
	p_data = create_parsing_data();
	init_parsing_data(p_data);

	/*
	 *Read file char by char putting them into the parsing_data struct. This struct is 
	 *used to store parsing data and info wich are necessary to the three steps of analysis.
	 */
	while((read_char = fgetc(_file)) != EOF)
	{
		p_data->ec = (unsigned char)tolower(read_char);
		if(email_lexical_analysis(p_data) == DOM_COMPLETED)
		{
			res = email_syntactic_analysis(p_data);
			if( res == SYNTAX_OK || res == SYNTAX_CORRECTED)
			{	
				res = email_semantic_analysis(p_data);
				if(res == EMAIL_OK)	
				{
					if(add_email(_mailing_list, get_email(email_str, p_data->email), NO_WARN_CHAR))
						ok_email_c++;
					else
						dup_email_c++;
				}
				else if(res == EMAIL_WARN)
				{
					if(add_email(_mailing_list, get_email(email_str, p_data->email), WARN_CHAR))
						warn_email_c++;
					else
						dup_email_c++;
				}
			}
			//re-initialize parsing data struct
			init_parsing_data(p_data);
		}
	}
	free(p_data->email->localpart);
	free(p_data->email->domain);
	free(p_data->email);
	free(p_data);
	
	p_stat.total_emails = ok_email_c + warn_email_c;
	p_stat.correct_emails = ok_email_c;
	p_stat.warned_emails = warn_email_c;
	p_stat.dup_emails = dup_email_c;
	
	return p_stat;
}
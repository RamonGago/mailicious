#include "emfs.h"

unsigned char char_value(unsigned char _c)
{
	/*This function yields the character value used to calculate hash*/
	
	int res;
	
	res = char_analysis(_c);
	switch (res)
	{
		case HYP_CHAR : return 1;
		case P_CHAR : return 2;
		case NUM_CHAR : return (_c - '0' + 3);
		case ALPHA_CHAR : return (_c - 'a' + 13);
		case UND_CHAR : return 39;
	}
	
	return 0;
}

unsigned int email_hash(char *_email)
{
	/*This function calculate email hash*/
	
	unsigned int hash;
	
	hash = 40 * 40 * char_value(_email[0]);
	if(_email[1] != '@')
		hash += 40 * char_value(_email[1]);
	if(_email[2] != '@')
		hash += char_value(_email[2]);
	return hash;
}

int create_new_list(char *_list_name)
{
	/*
	 *This function creates and initializes the three files (index, data, free slots) 
	 *that make up mailing list file system.
	 */
	
	FILE *list_file; /*pointer used to create the three files of emfs*/
	char path_name[PATH_MAX_LEN]; /*db file path*/
	char file_name[PATH_MAX_LEN]; /*db file path with extension*/
	int i;
	
	/*get db path*/
	strcpy(path_name, env_var.dbpath);
	strcat(path_name, _list_name);
	
	/*get index file path*/
	strcpy(file_name, path_name);
	strcat(file_name, ".ind");
	
	/*check if list name already exist*/
	if(access(file_name, F_OK) == 0)
		return LIST_EXIST;
	
	/*create and initialize index file*/
	list_file = fopen(file_name, "w");
	for(i = 1; i < INDEX_HASH_TABLE_SIZE; i++)
		fprintf(list_file, "%-10d\n", NO_EMAIL);
	fclose(list_file);
	
	/*get data file path*/
	strcpy(file_name, path_name);
	strcat(file_name, ".dat");
	/*create data file*/
	list_file = fopen(file_name, "w");
	/*The position of a record inside data file is its first byte offset from the file start.
	 *If I use every offset from 0 to unsigned int max I would need a negative number to denote 
	 *the absence of a record. This would reduce addressing space. Thus I reserve the first byte 
	 *(with offset 0) so I can use the 0 as record non-existence value.*/
	fprintf(list_file, "\n");
	fclose(list_file);
	
	/*get free data slot file path*/
	strcpy(file_name, path_name);
	strcat(file_name, ".fds");
	/*create free data slot file path*/
	list_file = fopen(file_name, "w");
	fclose(list_file);
	
	return LIST_CREATED;
}

void load_list(mailing_list_t *_mailing_list, char *_list_name)
{
	/*
	 *This function opens mailing list files of _list_name and loads the index file 
	 *and the free slots file data into the _mailing_list struct.
	 */
	
	char path_name[PATH_MAX_LEN]; /*db file path*/
	char file_name[PATH_MAX_LEN]; /*db file path with extension*/
	char pos[OFFSET_FORMAT_LEN]; /*the offset read in the free slots file*/
	free_slot_t *fs; /*free slot list node*/
	int i;
	
	/*get db path*/
	strcpy(path_name, env_var.dbpath);
	strcat(path_name, _list_name);
	
	/*get index file path*/
	strcpy(file_name, path_name);
	strcat(file_name, ".ind");
	/*open index file and load index table*/
	_mailing_list->f_index = fopen(file_name, "r+");
	
	for(i = 0; i < INDEX_HASH_TABLE_SIZE; i++)
		_mailing_list->index_table[i++] = 0;
	i = 1;
	while(fgets(pos, LINE_MAX_LEN, _mailing_list->f_index) != NULL)
		_mailing_list->index_table[i++] = strtol(pos, NULL, 10);
	
	/*get data file path and open it*/
	strcpy(file_name, path_name);
	strcat(file_name, ".dat");
	_mailing_list->f_email = fopen(file_name, "r+");
	
	/*get free data slot file path and open it*/
	strcpy(file_name, path_name);
	strcat(file_name, ".fds");
	_mailing_list->f_fslot = fopen(file_name, "r+");
	_mailing_list->free_slot = NULL;
	/*load free slots into a simple linked list used as stack*/
	while(fgets(pos, LINE_MAX_LEN, _mailing_list->f_fslot) != NULL)
	{
		fs = malloc(sizeof(free_slot_t));		
		fs->pos = strtol(pos, NULL, 10);
		fs->next = _mailing_list->free_slot;
		_mailing_list->free_slot = fs;
	}
}

void close_list(mailing_list_t *_mailing_list)
{
	/*This function closes mailing_list files and deallocate it*/
	
	free_slot_t *aux;
	
	fclose(_mailing_list->f_index);
	fclose(_mailing_list->f_email);
	fclose(_mailing_list->f_fslot);
	
	while(_mailing_list->free_slot != NULL)
	{
		aux = _mailing_list->free_slot;
		_mailing_list->free_slot = _mailing_list->free_slot->next;
		free(aux);
	}
	
	//free(_mailing_list);
}

void read_email_record(email_node_t *_email_node, mailing_list_t *_mailing_list, unsigned int _pos)
{
	/*
	 *This function reads the email record at the offset _pos of email file of 
	 * _mailing_list and store it into _email_node.
	 */
	
	char aux[LINE_MAX_LEN];
	int i;
	
	/*seek and read email record*/
	fseek(_mailing_list->f_email, _pos, SEEK_SET);
	fgets(_email_node->email, LINE_MAX_LEN, _mailing_list->f_email);
	i = 0;
	/*remove blank spaces*/
	while(_email_node->email[i] != ' ')
		i++;
	_email_node->email[i] = '\0';
	/*read warning field*/
	fgets(aux, LINE_MAX_LEN, _mailing_list->f_email);
	_email_node->warning = aux[0];
	/*read prev field*/
	_email_node->prev_email_pos = strtol(fgets(aux, LINE_MAX_LEN, _mailing_list->f_email), NULL, 10);
	/*read next field*/
	_email_node->next_email_pos = strtol(fgets(aux, LINE_MAX_LEN, _mailing_list->f_email), NULL, 10);
}

unsigned int write_email_record(mailing_list_t *_mailing_list, char *_email, char _warn, unsigned int _prev, unsigned int _next)
{
	/*
	 *This function takes the informations that make up an email record,  
	 *writes them into email data file and yields record position.
	 */
	
	unsigned int pos;
	free_slot_t *aux;
	
	/*if there is no available free slots, write it bottom of file*/
	if(_mailing_list->free_slot == NULL)
	{
		fseek(_mailing_list->f_email, 0L, SEEK_END);
		pos = ftell(_mailing_list->f_email);
		fprintf(_mailing_list->f_email, EMAIL_RECORD_FORMAT, _email, _warn, _prev, _next);
	}
	else {
		/*otherwise pop the first free slot and write on it*/	
		pos = _mailing_list->free_slot->pos;
		fseek(_mailing_list->f_email, pos, SEEK_SET);
		fprintf(_mailing_list->f_email, EMAIL_RECORD_FORMAT, _email, _warn, _prev, _next);
		/*update free slots stack top*/
		aux = _mailing_list->free_slot;
		_mailing_list->free_slot = _mailing_list->free_slot->next;
		free(aux);
		/*delete used slot from free slots file*/
		fseek(_mailing_list->f_fslot, -OFFSET_FORMAT_LEN, SEEK_END);
		ftruncate(fileno(_mailing_list->f_fslot), ftell(_mailing_list->f_fslot));
	}
	
	fflush(NULL);
	
	return pos;
}

int add_email(mailing_list_t *_mailing_list, char *_email, char _warn)
{
	/*This function takes an email and the eventual warning and add it to the db.*/
	
	unsigned int hash;
	int pos, pos_new;
	email_node_t *email_node;
	
	pos = pos_new = hash = 0;
	
	email_node = malloc(sizeof(email_node_t));
	hash = email_hash(_email);
	
	/*if there is no hash collision write the email and update index table and file*/
	if((pos = _mailing_list->index_table[hash]) == NO_EMAIL)
	{
		_mailing_list->index_table[hash] = write_email_record(_mailing_list, _email, _warn, NO_EMAIL, NO_EMAIL);
		fseek(_mailing_list->f_index, (hash - 1) * OFFSET_FORMAT_LEN, SEEK_SET);
		fprintf(_mailing_list->f_index, OFFSET_FORMAT, _mailing_list->index_table[hash]);
	}
	else {	
		/*find (alphabetic) insert position into the collision list*/
		read_email_record(email_node, _mailing_list, pos);
		while(strcmp(_email, email_node->email) > 0  && email_node->next_email_pos != NO_EMAIL)
		{
			pos = email_node->next_email_pos;
			read_email_record(email_node, _mailing_list, pos);
		}	
		/*top insertion or insertion between two nodes*/
		if(strcmp(_email, email_node->email) < 0)
		{
			/*write email and update next node prev (both cases)*/
			pos_new = write_email_record(_mailing_list, _email, _warn, email_node->prev_email_pos, pos);
			fseek(_mailing_list->f_email, pos + PREV_RECORD_POS, SEEK_SET);
			fprintf(_mailing_list->f_email, OFFSET_FORMAT, pos_new);
			
			/*update prev node next (middle insertion)*/
			if(email_node->prev_email_pos != NO_EMAIL)
			{
				fseek(_mailing_list->f_email, email_node->prev_email_pos + NEXT_RECORD_POS, SEEK_SET);
				fprintf(_mailing_list->f_email, OFFSET_FORMAT, pos_new);
			}
			/*update index table entry and file (top insert)*/
			else {
				_mailing_list->index_table[hash] = pos_new;
				fseek(_mailing_list->f_index, (hash - 1) * OFFSET_FORMAT_LEN, SEEK_SET);
				fprintf(_mailing_list->f_index, OFFSET_FORMAT, _mailing_list->index_table[hash]);
			}
		}
		/*bottom insertion*/
		else if(strcmp(_email, email_node->email) > 0)
		{
			pos_new = write_email_record(_mailing_list, _email, _warn, pos, NO_EMAIL);
			fseek(_mailing_list->f_email, pos + NEXT_RECORD_POS, SEEK_SET);			
			fprintf(_mailing_list->f_email, OFFSET_FORMAT, pos_new);
		}
		/*otherwise email exist*/
		else {
			free(email_node);
			return EMAIL_EXIST;
		}
	}
	free(email_node);
	
	fflush(NULL);
	
	return EMAIL_ADDED;
}

void delete_email_record(mailing_list_t *_mailing_list, unsigned int _pos)
{
	/*
	 *This function deletes the email record at the offset _pos. It doesn't really 
	 *cancel the record from the file but only remove it from the list and set its 
	 *position as free slot.
	 */
	
	email_node_t *email_node;
	free_slot_t *new_slot;
	unsigned int hash;

	email_node = malloc(sizeof(email_node_t));
	read_email_record(email_node, _mailing_list, _pos);
	hash = email_hash(email_node->email);
	
	/*if it is not the head node update the next of its previous node*/
	if(email_node->prev_email_pos != NO_EMAIL)
	{
		fseek(_mailing_list->f_email, email_node->prev_email_pos + NEXT_RECORD_POS, SEEK_SET);
		fprintf(_mailing_list->f_email, OFFSET_FORMAT, email_node->next_email_pos);
	}
	else {
		/*else update only index table entry and file*/
		_mailing_list->index_table[hash] = email_node->next_email_pos;
		fseek(_mailing_list->f_index, (hash - 1) * OFFSET_FORMAT_LEN, SEEK_SET);
		fprintf(_mailing_list->f_index, OFFSET_FORMAT, email_node->next_email_pos);
	}
	/*if it is not tail node update the prev of its next node*/
	if(email_node->next_email_pos != NO_EMAIL)
	{
		fseek(_mailing_list->f_email, email_node->next_email_pos + PREV_RECORD_POS, SEEK_SET);
		fprintf(_mailing_list->f_email, OFFSET_FORMAT, email_node->prev_email_pos);
	}
	
	free(email_node);
	
	/*add new free slot to free slots list and file*/
	fseek(_mailing_list->f_fslot, 0L, SEEK_END);
	fprintf(_mailing_list->f_fslot, OFFSET_FORMAT, _pos);
	new_slot = malloc(sizeof(free_slot_t));
	new_slot->pos = _pos;
	new_slot->next = _mailing_list->free_slot;
	_mailing_list->free_slot = new_slot;
	
	fflush(NULL);
}

unsigned int search_email_record(mailing_list_t *_mailing_list, char *_email)
{
	/*
	 *This function takes as input an email and a mailing list and return the position 
	 *of the email into email file or NO_EMAIL otherwise. 
	 */
	
	unsigned int hash;
	unsigned int pos;
	int res;
	email_node_t email_node;

	hash = email_hash(_email);
	pos = _mailing_list->index_table[hash];
	
	/*a simple linear search in an ordered linked list*/	
	while(pos != NO_EMAIL)
	{
		read_email_record(&email_node, _mailing_list, pos);
		res = strcmp(_email, email_node.email);
		if(res == 0)
			return pos;
		else if(res < 0)
			return NO_EMAIL;
		else
			pos = email_node.next_email_pos;
	}
	return NO_EMAIL;
}

int print_list(mailing_list_t *_mailing_list, char *_dir_path, int _list_size, int _no_warn, char *_separator_token)
{
	/*
	 *This function export all emails in  _mailing_list. Each email is put into a file according to 
	 *it starting letter (or number). If there are more than _list_size emails that begin with a 
	 *letter more file named progressively will be created, that is: a0, a1, a2 and so on.
	 *_dir_path specifies the directory, that will be created, where to put the files.
	 * _no_warn if true doesn't get warned emails printed.
	 *_separator_token is the string used to separate single emails into a file.
	 */
	
	email_node_t *en;
	int i, aux;
	int file_index;
	int email_count;
	FILE *out;
	char out_name[PATH_MAX_LEN];
	char out_path[PATH_MAX_LEN];
	int cont;
	
	en = malloc(sizeof(email_node_t));
	email_count = 0;
	file_index = 0;
	out = NULL;
	out_name[0] = '\0';
	
	if(_list_size > 0 && !opendir(_dir_path))
		mkdir(_dir_path, S_IRWXU | S_IRWXG | S_IRWXO);
	else if(_list_size == 0)
		out = fopen(_dir_path, "w");
	
	cont = 0;
	/*pass through each list pointed in the index table entries*/
	for(i = 0; i < INDEX_HASH_TABLE_SIZE; i++)
	{
		if(_mailing_list->index_table[i] != NO_EMAIL)
		{
			aux = _mailing_list->index_table[i];
			while(aux != NO_EMAIL)
			{
				read_email_record(en, _mailing_list, aux);
				if(en->warning == WARN_CHAR && _no_warn)
					continue;
				if(_list_size > 0)
				{
					if(out_name[0] != en->email[0])
					{
						file_index = 0;
						email_count = 0;
						sprintf(out_name, "%c%d", en->email[0], file_index);
						sprintf(out_path, "%s/%s", _dir_path, out_name);
						if(out != NULL)
							fclose(out);
						out = fopen(out_path, "w");
					}
					else if(email_count == _list_size)
					{
						fclose(out);
						file_index++;
						email_count = 0;
						sprintf(out_name, "%c%d", en->email[0], file_index);
						sprintf(out_path, "%s/%s", _dir_path, out_name);
						out = fopen(out_path, "w");
					}
				}
				
				fprintf(out, "%s%s", en->email, _separator_token);
				email_count++;
				cont++;
				aux = en->next_email_pos;
			}
		}
	}
	fclose(out);
	free(en);
	
	return cont;
}

int update_email(mailing_list_t *_mailing_list, char *_old_email, char *_new_email, char _warn)
{
	unsigned int pos;
	
	pos = search_email_record(_mailing_list, _old_email);
	if(pos != NO_EMAIL)
	{
		delete_email_record(_mailing_list, pos);
		add_email(_mailing_list, _new_email, _warn);
		return EMAIL_UPDATED;
	}
	return EMAIL_NOT_EXIST;
}

void delete_emails(mailing_list_t *_mailing_list, char *_delete_list_path)
{
	FILE *delete_list;
	char email[LINE_MAX_LEN];
	unsigned int pos;
	int cont = 0;
	
	delete_list = fopen(_delete_list_path, "r");	
	while(fgets(email, LINE_MAX_LEN, delete_list) != NULL)
	{
		email[strlen(email) - 1] = '\0';
		pos = search_email_record(_mailing_list, email);
		if(pos != NO_EMAIL)
		{
			delete_email_record(_mailing_list, pos);
			cont++;
			printf("deleted %s\n", email);
		}
	}
	printf("\n%d emails deleted\n\n", cont);
	fclose(delete_list);
}

float ping_domain(char *domain)
{
	/*Try to ping the domain arguments and if response is positive return ping time*/
	
    FILE *pf;
    char data[DATA_SIZE];
	int data_len;
	char command[MAX];
	char time[10];
	int i, j, c;
    
	/*execute ping through a pipe*/
    sprintf(command, "ping -c 1 -W 1 %s 2> /dev/null", domain);
    pf = popen(command,"r");
    if(!pf)
		return -1;

	/*get ping output*/
	data_len = 0;
	memset(data, '\0', DATA_SIZE);
    while ((c = fgetc(pf)) != EOF)
		data[data_len++] = (char)c;
    data[data_len] = '\0';
	
	/*if there is no output or it ends with \n\n means that domain wasn't found 
	  or it didn't respond*/
	if(data_len == 0)
	{
		pclose(pf);
		return NO_HOST;
	}
	else if(data_len >= 2 && data[data_len - 1] == '\n' && data[data_len - 2] == '\n')
	{
		pclose(pf);
		return NO_RESPONSE;
	}
	/*otherwise parse the output to get ping rtt*/
	else {
		i = data_len - 1;
		j = 0;
		while(i > 0 && j != 2)
		{
			if(data[i] == '/')
				j++;
			i--;
		}
		i += 2;
		memset(time, '\0', 10);
		j = 0;
		while(data[i] != '/')
			time[j++] = data[i++];
		time[j] = '\0';
		
		pclose(pf);
		return atof(time);
	}
	
	pclose(pf);
    return -1;
}

void check_domain(mailing_list_t *_mailing_list, char *_out_name)
{
	/*get all email domain present in _mailing_list and perform ping*/
	
	email_node_t *en;
	unsigned int aux;
	unsigned int record_pos;
	record_list_t *record_node;
	domain_list_t *domain_list;
	domain_list_t *dom, *dom_prev, *dom_curr;
	int i, j, res;
	float response, max_response, min_response, tot_response;
	char fastest_host[RFC_DOM_MAX_LEN];
	char slowest_host[RFC_DOM_MAX_LEN];
	int cont_ok, cont_no_host, cont_no_repsonse;
	int deleted_emails;
	FILE *out;

	domain_list = dom = dom_prev = dom_curr = NULL;
	en = malloc(sizeof(email_node_t));
	/*pass through each list pointed in the index table entries*/
	for(i = 0; i < INDEX_HASH_TABLE_SIZE; i++)
	{
		if(_mailing_list->index_table[i] != NO_EMAIL)
		{
			aux = _mailing_list->index_table[i];
			while(aux != NO_EMAIL)
			{
				read_email_record(en, _mailing_list, aux);
				record_pos = aux;
				aux = en->next_email_pos;
				
				j = 0;
				while(en->email[j] != '@')
					j++;
				
				/*each domain must be considered only one time, so I put domains 
				  in an ordered list to avoid duplicates*/
				
				dom_curr = domain_list;
				dom_prev = NULL;
				res = 1;
				/*check if domain is already present into the list*/
				while(dom_curr != NULL && (res = strcmp(&(en->email[j + 1]), dom_curr->domain)) > 0)
				{
					dom_prev = dom_curr;
					dom_curr = dom_curr->next_domain;
				}
				
				if(res == 0)
				{
					record_node = malloc(sizeof(record_list_t));
					record_node->pos = record_pos;
					record_node->next_pos = dom_curr->record_list;
					
					dom_curr->record_list = record_node;
				}
				/*if not add it*/
				else
				{
					dom = malloc(sizeof(domain_list_t));
					dom->domain = strdup(&(en->email[j + 1]));
					dom->next_domain = NULL;
					dom->record_list = malloc(sizeof(record_list_t));
					dom->record_list->pos = record_pos;
					dom->record_list->next_pos = NULL;
					
					if(dom_curr == domain_list)
					{
						dom->next_domain = domain_list;
						domain_list = dom;
					}
					else
					{
						dom->next_domain = dom_curr;
						dom_prev->next_domain = dom;
					}
				}
			}
		}
	}
	free(en);
	
	/*put domain name and ping response into a csv file, so it
	  can be processed by other programs*/
	out = fopen(_out_name, "w");
	
	response = max_response = min_response = tot_response = cont_ok = cont_no_host = cont_no_repsonse = 0;
	memset(fastest_host, '\0', RFC_DOM_MAX_LEN);
	memset(slowest_host, '\0', RFC_DOM_MAX_LEN);
	deleted_emails = 0;
	/*I get one by one the domains from the list and ping them*/
	while(domain_list)
	{
		dom = domain_list;
		printf("ping: %-50s\t", dom->domain);
		fprintf(out, "%s;", dom->domain);
		response = ping_domain(dom->domain);
		if(response > 0)
		{
			tot_response +=response;
			cont_ok++;
			if(response > max_response)
			{
				max_response = response;
				strcpy(slowest_host, dom->domain);
			}
			if((response < min_response) || min_response == 0)
			{
				min_response = response;
				strcpy(fastest_host, dom->domain);
			}
			printf("%.3f ms\n", response);
			fprintf(out, "%.3f;\n", response);
		}
		else
		{
			if(response == NO_RESPONSE)
			{
				cont_no_repsonse++;
				printf("no response\n");
				fprintf(out, "-;\n");
			}
			else if(response == NO_HOST)
			{
				cont_no_host++;
				printf("unknown host\t");
				fprintf(out, "x;");
			}
		}
		domain_list = domain_list->next_domain;
		
		i = 0;
		while(dom->record_list)
		{
			if(response == NO_HOST)
			{
				delete_email_record(_mailing_list, dom->record_list->pos);
				i++;
			}
			
			record_node = dom->record_list;
			dom->record_list = dom->record_list->next_pos;
			free(record_node);
		}
		free(dom->domain);
		free(dom);
		
		deleted_emails += i;
		
		if(response == NO_HOST)
		{
			printf("deleted %d emails\n", i);
			fprintf(out, "%d;\n", i);
		}
	}
	printf("\n\n");
	printf("Total pings performed: %d (%d with no response and %d to unknown hosts)\n\n", cont_ok + cont_no_repsonse + cont_no_host, cont_no_repsonse, cont_no_host);
	printf("Average response %.3f ms\n\n", tot_response / cont_ok);
	printf("Best response from %s of %.3f ms\n\n", fastest_host, min_response);
	printf("Worst response from %s of %.3f ms\n\n", slowest_host, max_response);
	printf("Deleted %d emails from mailing list\n\n", deleted_emails);
	
	fseek(out, 0, SEEK_SET);
	
	fprintf(out, "total pings;%d;\n", cont_ok + cont_no_repsonse + cont_no_host);
	fprintf(out, "no response pings;%d;\n", cont_no_repsonse);
	fprintf(out, "unknown host pings;%d;\n", cont_no_host);
	fprintf(out, "avg response;%.3f ms;\n", tot_response / cont_ok);
	fprintf(out, "best response;%.3f ms;\n", min_response);
	fprintf(out, "worst response;%.3f ms;\n", max_response);
	fprintf(out, "deleted emails;%d;\n", deleted_emails);
	fprintf(out, ";;\n");
	fprintf(out, "DOMAIN;RTT (ms);DEL_EMAILS;\n");
	
	fclose(out);
}
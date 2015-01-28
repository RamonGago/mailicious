#include "mailicious.h"

/*program global settings*/
env_var_t env_var;
/*used by semantyc analysis (see parser.c)*/
domain_node_t **domains_hash_table;

void load_conf_data()
{	
	FILE *conf;
	char str[LINE_MAX_LEN];
	char **args;
	int argc;
	char path[PATH_MAX_LEN];
	int i;
	
	/*load default in order to prevent conf file loading error or corruption*/
	env_var.lp_ic_tolerance = LP_IC_TOLERANCE;
	env_var.dom_ic_tolerance = DOM_IC_TOLERANCE;
	
	env_var.email_avg_len = EMAIL_AVG_LEN;
	env_var.email_warn_len = EMAIL_WARN_LEN;
	
	env_var.lp_avg_len = LP_AVG_LEN;
	env_var.lp_warn_len = LP_WARN_LEN;
	env_var.lp_avg_num = LP_AVG_NUM;
	env_var.lp_warn_num = LP_WARN_NUM;
	env_var.lp_avg_p = LP_AVG_P;
	env_var.lp_warn_p = LP_WARN_P;
	env_var.lp_avg_hyp = LP_AVG_HYP;
	env_var.lp_warn_hyp = LP_WARN_HYP;
	env_var.lp_avg_und = LP_AVG_UND;
	env_var.lp_warn_und = LP_WARN_UND;
	
	env_var.dom_avg_len = DOM_AVG_LEN;
	env_var.dom_warn_len = DOM_WARN_LEN;
	env_var.dom_avg_num = DOM_AVG_NUM;
	env_var.dom_warn_num = DOM_WARN_NUM;
	env_var.dom_avg_hyp = DOM_AVG_HYP;
	env_var.dom_warn_hyp = DOM_WARN_HYP;
	env_var.dom_avg_und = DOM_AVG_UND;
	env_var.dom_warn_und = DOM_AVG_HYP;
	
	/*try to load global settings from conf file*/
	memset(path, '\0', PATH_MAX_LEN);
	strcat(path, env_var.work_dir);
	strcat(path, "conf");
	if((conf = fopen(path, "r")) == NULL)
	{
		printf("\nError loading configuration file.\n");
		printf("Loaded default option\n\n");
		return;
	}
	
	while(fgets(str, LINE_MAX_LEN, conf))
	{
		strexp(&args, &argc, str, " \t\n");
		if(argc > 2)
		{
			if(strcmp(args[0], "lp_ic_tolerance") == 0)
				env_var.lp_ic_tolerance = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "dom_ic_tolerance") == 0)
				env_var.dom_ic_tolerance = strtol(args[1], NULL, 10);
			
			else if(strcmp(args[0], "email_avg_len") == 0)
				env_var.email_avg_len = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "email_warn_len") == 0)
				env_var.email_warn_len = strtol(args[1], NULL, 10);
			
			else if(strcmp(args[0], "lp_avg_len") == 0)
				env_var.lp_avg_len = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "lp_warn_len") == 0)
				env_var.lp_warn_len = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "lp_avg_num") == 0)
				env_var.lp_avg_num = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "lp_warn_num") == 0)
				env_var.lp_warn_num = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "lp_avg_p") == 0)
				env_var.lp_avg_p = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "lp_warn_p") == 0)
				env_var.lp_warn_p = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "lp_avg_hyp") == 0)
				env_var.lp_avg_hyp = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "lp_warn_hyp") == 0)
				env_var.lp_warn_hyp = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "lp_avg_und") == 0)
				env_var.lp_avg_und = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "lp_warn_und") == 0)
				env_var.lp_warn_und = strtol(args[1], NULL, 10);
			
			else if(strcmp(args[0], "dom_avg_len") == 0)
				env_var.dom_avg_len = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "dom_warn_len") == 0)
				env_var.dom_warn_len = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "dom_avg_num") == 0)
				env_var.dom_avg_num = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "dom_warn_num") == 0)
				env_var.dom_warn_num = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "dom_avg_hyp") == 0)
				env_var.dom_avg_hyp = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "dom_warn_hyp") == 0)
				env_var.dom_warn_hyp = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "dom_avg_und") == 0)
				env_var.dom_avg_und = strtol(args[1], NULL, 10);
			else if(strcmp(args[0], "dom_warn_und") == 0)
				env_var.dom_warn_und = strtol(args[1], NULL, 10);
		}
		
		for(i = 0; i < argc; i++)
			free(args[i]);
		free(args);
	}
	fclose(conf);
}

void init_program_data(char *_arg0)
{
	int i, j;
	
	j = 0;
	i = strlen(_arg0) - 10; /*strlen("mailicious") == 10*/
	memset(env_var.work_dir, '\0', PATH_MAX_LEN);
	while(j < i)
	{
		env_var.work_dir[j] = _arg0[j];
		j++;
	}
	env_var.work_dir[j] = '\0';
	
	strcpy(env_var.dbpath, env_var.work_dir);
	strcat(env_var.dbpath, DB_PATH);
	
	if(!opendir(env_var.dbpath))
		mkdir(env_var.dbpath, S_IRWXU | S_IRWXG | S_IRWXO);
	
	if(load_domains() == ERROR)
	{
		printf("Error loading domain file\nProgram aborted\n\n");
		exit(1);
	}
	load_conf_data();
}

int load_mailing_list(char *_list_name, mailing_list_t *_mailing_list)
{
	/*used by every user command that needs to read/write email db*/
	
	char file_path[PATH_MAX_LEN];
	
	strcpy(file_path, env_var.dbpath);
	strcat(file_path, _list_name);
	strcat(file_path, ".dat");
	
	if(access(file_path, F_OK) != 0)
	{
		printf("Mailing list %s doesn't exist!\n\n", _list_name);
		return 0;
	}
	load_list(_mailing_list, _list_name);
	
	return 1;
}

int list()
{
	/*This function lists all mailing list in the db*/
	
	DIR *d;
	struct dirent *dir;
	struct stat file_info;
	char path[PATH_MAX_LEN];
	char last_char;
	int cont;
	
	d = opendir(env_var.dbpath);
	if (d)
	{
		cont = 0;
		/*at each cycle get a file of the directory d*/
		while ((dir = readdir(d)) != NULL)
		{
			/*get file path*/
			strcpy(path, env_var.dbpath);
			strcat(path, dir->d_name);
			/*check if is a regular file or a directory*/
			stat(path, &file_info);
			/*Since every list has three files I consider only one*/
			last_char = dir->d_name[strlen(dir->d_name) - 1];
			if(S_ISREG(file_info.st_mode) && last_char != 's' && last_char != 't')
			{
				dir->d_name[strlen(dir->d_name) - 4] = '\0'; /*remove extension*/
				printf("%s\n", dir->d_name);
				cont++;
			}
		}
		if(cont == 0)
			printf("Database is empty!\n");
		printf("\n");
		closedir(d);
	}
	
	return CMD_SUCCESSFUL;
}

int import_email(char **_args, int _argc)
{
	/*args sequence: import directory/file target_list [-u] [-s]*/
	
	DIR *d;
	struct dirent *dir;
	struct stat file_info;
	FILE *file_to_parse;
	mailing_list_t mailing_list;
	parsing_stat_t p_stat, p_stat_aux; /*local and global parsing stat*/
	char path[PATH_MAX_LEN];
	int i;
	int update_list = 0;
	int tolerance = ACCEPT_WARNING;
	
	if(_argc < 3)
	{
		printf("Usage: import FILE_OR_DIR_PATH LIST_NAME [-u] [-s]\n\n");
		return CMD_ABORTED;
	}
	
	for(i = 3; i < _argc; i++)
	{
		if(strcmp(_args[i], "-u") == 0)
			update_list = 1;
		if(strcmp(_args[i], "-s") == 0)
			tolerance = NO_WARNING;
	}
	
	/*try to load file infos*/
	if(stat(_args[1], &file_info) == -1)
	{
		printf("Path %s is invalid!\n\n", _args[1]);
		return CMD_ABORTED;
	}

	if(create_new_list(_args[2]) == LIST_EXIST  && !update_list)
	{
		printf("Mailing list %s already exists. Use -u to update a list.\n\n", _args[2]);
		return CMD_ABORTED;
	}
	else
		load_list(&mailing_list, _args[2]);
	
	p_stat.total_emails = p_stat.correct_emails = p_stat.warned_emails = p_stat.dup_emails = 0;
	p_stat_aux.total_emails = p_stat_aux.correct_emails = p_stat_aux.warned_emails = p_stat_aux.dup_emails = 0;
	
	/*if is a regular file open and parse it*/
	if(S_ISREG(file_info.st_mode))
	{
		file_to_parse = fopen(_args[1], "r");
		p_stat = parse(&mailing_list, file_to_parse, tolerance);
		fclose(file_to_parse);
	}

	/*if is a directory I get one by one all its files*/
	if(S_ISDIR(file_info.st_mode))
	{
		d = opendir(_args[1]);
		if (d)
		{
			while ((dir = readdir(d)) != NULL)
			{
				/*make file path*/
				strcpy(path, _args[1]);
				strcat(path, "/");
				strcat(path, dir->d_name);
				/*open and parse it*/
				file_to_parse = fopen(path, "r");
				p_stat_aux = parse(&mailing_list, file_to_parse, tolerance);
				fclose(file_to_parse);
				printf("Imported %d emails from %s (%d with warning)\n", p_stat_aux.total_emails, dir->d_name, p_stat_aux.warned_emails);
				if(p_stat_aux.dup_emails > 0)
					printf("Other %d were already in the database\n", p_stat_aux.dup_emails);
				printf("\n");
				p_stat.total_emails += p_stat_aux.total_emails;
				p_stat.correct_emails += p_stat_aux.correct_emails;
				p_stat.warned_emails += p_stat_aux.warned_emails;
				p_stat.dup_emails += p_stat_aux.dup_emails;
			}
			closedir(d);
		}
	}
	printf("Total imported emails: %d (%d with warning)\n\n", p_stat.total_emails, p_stat.warned_emails);
	if(p_stat.dup_emails > 0)
		printf("Found %d duplicates\n\n", p_stat.dup_emails);
	
	close_list(&mailing_list);
	
	return CMD_SUCCESSFUL;
}

int export_email(char **_args, int _argc)
{
	/*command sequence: export list_name out_path file_size separator_token*/
	
	/*export the mailing list into a directory wich contains a file for each
	  first letter of emails. The file can contains at most file size email. 
	  So I will get a directory named as the mailing_list with files like:
	  a0; a1; b0; c0; and so on.*/
	
	int file_size;
	mailing_list_t mailing_list;
	char *token;
	int i, j;
	
	if(_argc < 4)
	{
		printf("Usage: export LIST_NAME DIR_PATH FILE_SIZE [-w] [TOKEN]\n\n");
		return CMD_ABORTED;
	}
	
	if(!load_mailing_list(_args[1], &mailing_list))
		return CMD_ABORTED;
		
	file_size = strtol(_args[3], NULL, 10);
	
	if(_argc == 4)
		print_list(&mailing_list, _args[2], file_size, 0, "\n");
	else if(_argc == 5 && strcmp(_args[4], "-w") == 0)
		print_list(&mailing_list, _args[2], file_size, 1, "\n");
	else if(_argc == 5 && strcmp(_args[4], "-w") != 0)
	{
		token = strdup(_args[4]);
		for(i = 0, j = 0; i < strlen(token); i++)
		{
			if(token[i] == '\\' && (i + 1) != strlen(token) && token[i + 1] == 'n')
			{
				token[i++] = '\n';
				j++;
			}
			
		}
		token[strlen(token) - j] = '\0';
		print_list(&mailing_list, _args[2], file_size, 0, token);
		free(token);
	}
	else
		print_list(&mailing_list, _args[2], file_size, 1, _args[4]);
	
	close_list(&mailing_list);
	
	return CMD_SUCCESSFUL;
}

int search(char **_args, int _argc)
{
	/*search if an email is present on the mailing_list*/
	
	mailing_list_t mailing_list;
	
	if(_argc < 3)
	{
		printf("Usage: search LIST_NAME EMAIL\n\n");
		return CMD_ABORTED;
	}
	
	if(!load_mailing_list(_args[1], &mailing_list))
		return CMD_ABORTED;
	
	if(search_email_record(&mailing_list, _args[2]))
		printf("\nThe email is in the database\n\n");
	else
		printf("\nThe email is not in the database\n\n");
	
	close_list(&mailing_list);
	
	return CMD_SUCCESSFUL;
}

int update(char **_args, int _argc)
{
	/*modify an existent email*/
	
	mailing_list_t mailing_list;
	
	if(_argc < 4)
	{
		printf("Usage: update LIST_NAME OLD_EMAIL NEW_EMAIL\n\n");
		return CMD_ABORTED;
	}
	
	if(!load_mailing_list(_args[1], &mailing_list))
		return CMD_ABORTED;
	
	if(update_email(&mailing_list, _args[2] , _args[3], NO_WARN_CHAR) == EMAIL_UPDATED)
	{
		printf("\nEmail successfully updated\n\n");
		close_list(&mailing_list);
		return CMD_SUCCESSFUL;
	}
	else {
		printf("\nEmail %s doesn't exist in %s\n\n", _args[2], _args[1]);
		close_list(&mailing_list);
		return CMD_ABORTED;
	}
}

int delete_email(char **_args, int _argc)
{
	/*delete a single email or a list of emails taken by an input file*/
	
	mailing_list_t mailing_list;
	unsigned int pos = 0;
	
	if(_argc < 3)
	{
		printf("Usage: delete LIST_NAME [-f] EMAIL/UNSUBSCRIBE_FILE\n\n");
		return CMD_ABORTED;
	}
	
	if(!load_mailing_list(_args[1], &mailing_list))
		return CMD_ABORTED;
	
	if(strcmp(_args[2], "-f") == 0)
		delete_emails(&mailing_list, _args[3]);
	else {
		if((pos = search_email_record(&mailing_list, _args[2])) == NO_EMAIL)
			printf("\nThe email is not in the database\n\n");
		else {
			delete_email_record(&mailing_list, pos);
			printf("\nEmail deleted\n\n");
		}
	}
	
	close_list(&mailing_list);
	
	return CMD_SUCCESSFUL;
}

int remove_mailing_list(char **_args, int _argc)
{
	char path_name[PATH_MAX_LEN]; /*db file path*/
	char file_name[PATH_MAX_LEN]; /*db file path with extension*/
	
	if(_argc < 2)
	{
		printf("Usage: remove LIST_NAME\n\n");
		return CMD_ABORTED;
	}
	
	/*get db path*/
	strcpy(path_name, env_var.dbpath);
	strcat(path_name, _args[1]);
	
	/*get index file path*/
	strcpy(file_name, path_name);
	strcat(file_name, ".ind");
	/*check if list name already exist*/
	if(access(file_name, F_OK) == 0)
		remove(file_name);
	else {
		printf("Can't find db index file. Command aborted\n\n");
		return CMD_ABORTED;
	}
	
	strcpy(file_name, path_name);
	strcat(file_name, ".dat");
	if(access(file_name, F_OK) == 0)
		remove(file_name);
	else {
		printf("Can't find db data file. Command aborted\n\n");
		return CMD_ABORTED;
	}
	
	strcpy(file_name, path_name);
	strcat(file_name, ".fds");
	if(access(file_name, F_OK) == 0)
		remove(file_name);
	else {
		printf("Can't find db free slot file. Command aborted\n\n");
		return CMD_ABORTED;
	}
	
	printf("Mailing list successfully removed\n\n");
	return CMD_SUCCESSFUL;
}

int ping(char **_args, int _argc)
{
	/*test email domains by pinging them*/
	
	mailing_list_t mailing_list;
	
	if(_argc < 3)
	{
		printf("Usage: ping LIST_NAME OUTPUT_FILE\n\n");
		return CMD_ABORTED;
	}
	
	if(!load_mailing_list(_args[1], &mailing_list))
		return CMD_ABORTED;
	
	check_domain(&mailing_list, _args[2]);
	
	close_list(&mailing_list);
	
	return CMD_SUCCESSFUL;
}

int help()
{
	printf("\n\n");
	printf("  list\t-lists the names of all mailing lists occuring into the db.\n\n\n");
	printf("  import INPUT LIST_NAME [-u] [-s]\t-create a new mailing list.\n\n");
	printf("\tINPUT: file or directory path from which take emails.\n");
	printf("\tLIST_NAME: mailing list name (must be different from existing mailing list).\n");
	printf("\t-u: update an exisisting mailing list by adding (eventaully found) new emails.\n");
	printf("\t-s: doesn't add warned emails to the db.\n\n\n");
	printf("  export LIST_NAME DIR_NAME FILE_SIZE [-w] [TOKEN]\t-export a mailing list.\n\n");
	printf("\tLIST_NAME: mailing list name (must be an existing mailing list).\n");
	printf("\tDIR_PATH: directory where to put mailing_list file (better a new one.\n");
	printf("\tFILE_SIZE: number of emails for each alphabetic named file.\n");
	printf("\t-w: doesn't export warned emails.\n\n\n");
	printf("\tTOKEN: separator chars after each email (default is \\n)\n\n\n");
	printf("  search LIST_NAME EMAIL\t-check email existence into a mailing list\n\n");
	printf("\tLIST_NAME: mailing list where to search.\n");
	printf("\tEMAIL: email to search.\n\n\n");
	printf("  update LIST_NAME OLD_EMAIL NEW_EMAIL\t-update an email\n\n");
	printf("\tLIST_NAME: mailing list where email to modify is located\n");
	printf("\tOLD_EMAIL: email to modify.\n");
	printf("\tNEW_EMAIL: email modified.\n\n\n");
	printf("  delete LIST_NAME [-f] DEL_LIST\t-delete emails from a mailing list\n\n");
	printf("\tLIST_NAME: mailing list where delete\n");
	printf("\t-f: if used it speicifies that DEL_LIST is a file containg emails.\n");
	printf("\tDEL_LIST: unless of -f it is a single email.\n\n\n");
	printf("  remove LIST_NAME \t-delete a mailing list\n\n");
	printf("\tLIST_NAME: mailing list to delete.\n\n\n");
	printf("  ping LIST_NAME OUT_FILE\t-ping mailing list domains\n\n");
	printf("\tLIST_NAME: mailing list where to take domains.\n");
	printf("\tOUT_FILE: path where to save ping results.\n\n\n");
	printf("  exit\t -exit program\n\n\n");
	
	return CMD_SUCCESSFUL;
}

int execute_command(int _argc, char **_args)
{
	if(strcmp(_args[0], "help") == 0)
		return help();
	else if(strcmp(_args[0], "list") == 0)
		return list();	
	else if(strcmp(_args[0], "import") == 0)
		return import_email(_args, _argc);
	else if(strcmp(_args[0], "export") == 0)
		return export_email(_args, _argc);
	else if(strcmp(_args[0], "search") == 0)
		return search(_args, _argc);
	else if(strcmp(_args[0], "delete") == 0)
		return delete_email(_args, _argc);
	else if(strcmp(_args[0], "remove") == 0)
		return remove_mailing_list(_args, _argc);
	else if(strcmp(_args[0], "update") == 0)
		return update(_args, _argc);
	else if(strcmp(_args[0], "ping") == 0)
		return ping(_args, _argc);
	return CMD_ABORTED;
}

int main(int argc, char **argv)
{
	char cmd[CMD_MAX_LEN];
	char **args;
	int arg_num;
	int i;

	init_program_data(argv[0]);
	printf("\n");
	if(argc > 1)
	{
		arg_num = argc - 1;
		args = &argv[1];
		return execute_command(arg_num, args);
	}
	else {
		printf("M@ILICIOUS v1.0\nby Federico Vindigni\n\nType help for commands list\n\n\n");
		while(1)
		{
			printf("@>");
			fgets(cmd, CMD_MAX_LEN, stdin);
			strexp(&args, &arg_num, cmd, " \t\n");
			printf("\n");

			if(argc > 0)
			{
				if(strcmp(args[0], "exit") == 0)
					return 0;
				else
					execute_command(arg_num, args);
			}
			
			for(i = 0; i < arg_num; i++)
				free(args[i]);
			free(args);
		}
	}
	free_domain_table();
	
	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"


email_t *create_email()
{
	email_t *email;
	
	email = malloc(sizeof(email_t));
	email->localpart = malloc(sizeof(char) * RFC_LP_MAX_LEN);
	email->domain = malloc(sizeof(char) * RFC_DOM_MAX_LEN);
	return email;
}

void init_email(email_t *_email)
{
	_email->localpart[0] = '\0';
	_email->domain[0] = '\0';
	_email->localpart_len = 0;
	_email->domain_len = 0;
}

char *get_email(char *_email_str, email_t *_email)
{
	strcpy(_email_str, _email->localpart);
	strcat(_email_str, "@");
	strcat(_email_str, _email->domain);
	
	return _email_str;
}

int char_analysis(char _email_char)
{
	//for every class of char return specific value
	
	if(_email_char == '@')
		return AT_CHAR;
	if((_email_char >= 'a' && _email_char <= 'z') ||
	   (_email_char >= 'A' && _email_char <= 'Z'))
		return ALPHA_CHAR;
	if(_email_char >= '0' && _email_char <= '9')
		return NUM_CHAR;
	if(_email_char == '.')
		return P_CHAR;
	if(_email_char == '-')
		return HYP_CHAR;
	if(_email_char == '_')
		return UND_CHAR;
	return INVALID_CHAR;
}

char *strtrim(char *_str)
{
	/*This function removes separation chars at start and end of a string*/
	
	int i, j, k, l;
	int len;
	
	len = strlen(_str);
	i = 0;
	
	/*find string start*/
	while((_str[i] == ' ' || _str[i] == '\t' || _str[i] == '\n') && i < len)
		i++;
	j = len - 1;
	/*find string end*/
	while((_str[j] == ' ' || _str[j] == '\t' || _str[j] == '\n') && i < len)
		j--;
	/*shift string*/
	for(k = i, l = 0; k <= j; k++, l++)
		_str[l] = _str[k];
	_str[l] = '\0';
	return _str;
}

int strexp(char ***_args, int *_argc, char *_str, char *_delim)
{
	/*
	 *This function takes a string _str as input and puts into _args each substring 
	 *found into _str delimited by separation chars defined in _delim. It puts substring 
	 *number into argc and return it.
	 */
	
	int len, dlen;
	int first_string_char; /*true if char read is first substring char, false otherwise*/
	int start; /*starting index of a substring*/
	int delimiter_found; /*ture if char read is a delimiter*/
	int args_counter;
	int current_arg;
	int current_arg_len;
	int i, j, k;
	
	len = strlen(_str);
	dlen = strlen(_delim);
	
	/*
	 *Since I want that string array size fits arguments number and each string len fits its 
	 *argument len, without use realloc, I have to do two steps: count arguments number; 
	 *count each argument len and then copy it into the arguments array.
	 */
	
	args_counter = 0;
	first_string_char = 1;
	for(i = 0; i < len; i++)
	{
		/*
		 *I increment substring counter each time a non separator char is read, 
		 *so I need to consider only first char of each substring. In order to 
		 *do so I use first_string_char as a flag.
		 */
		
		/*check if char is in delimiter string*/
		j = delimiter_found = 0;
		while(j < dlen && !delimiter_found)
			if(_str[i] == _delim[j++])
				delimiter_found = 1;

		if(!delimiter_found && first_string_char)
		{
			args_counter++;
			first_string_char = 0;
		}
		else {
		if(delimiter_found)
			first_string_char = 1;
		}
	}
	
	*_args = malloc(sizeof(char *) * (args_counter + 1));
	
	delimiter_found = start = current_arg = 0;
	first_string_char = 1;
	for(i = 0; i < len; i++)
	{
		/*Same as before but this time instead of count args I get arg start index*/
		
		j = 0;
		while(j < dlen && delimiter_found == 0)
			if(_str[i] == _delim[j++])
				delimiter_found = 1;

		if(!delimiter_found && first_string_char)
		{
			start = i;
			first_string_char = 0;
		}
		/*second condition is in case that last char is not a delimiter*/
		if(delimiter_found || i == len - 1)
		{
			/*copy arg*/
			if(!first_string_char)
			{
				current_arg_len = i - start;
				/*if last char is not a delimiter*/
				if(!delimiter_found)
					current_arg_len++;
				(*_args)[current_arg] = malloc(sizeof(char) * current_arg_len + 1);
				for(k = 0; k < current_arg_len; k++)
					(*_args)[current_arg][k] = _str[start + k];
				(*_args)[current_arg][current_arg_len] = '\0';
				current_arg++;
			}
			first_string_char = 1;
			delimiter_found = 0;
		}
	}
	
	*_argc = args_counter;
	
	return *_argc;
}
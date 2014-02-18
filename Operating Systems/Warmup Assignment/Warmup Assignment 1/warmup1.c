#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "cs402.h"

#include "my402list.h"

#include "my402data.h"

// int debug = 1;

void print_error(char * error)
{
	fprintf(stderr, "Error: %s, program abort.\n", error);
	exit(1);
}

int commandline(int argc, char * argv[])
{
	int command;
	if(argc < 2)
		print_error("miss command");
	else if(strcmp(argv[1], "sort"))
	    print_error("undefined command");
	else if(argc == 3)
	{
		if(argv[2][0] == '-')
			print_error("malformed command");
		command = 1;
	}
	else if(argc == 2)
		command = 0;
	else
		print_error("number of parameters error");
	return command;
}

int process_type(char * str, int i, char * type)
{
	if(str[i] != '-' && str[i] != '+')
		print_error("undefined transcation type");
	else
	{
		*type = str[i++];
		if(str[i] != '\t')
			print_error("wrong format of file");
		else
			i++;
	}
	return i;
}

int process_timestamp(char * str, int i, char * timestamp)
{
	int timestamp_i = 0;
	while(str[i] != '\t')
	{
		if(timestamp_i >= MAX_TIMESTAMP_LENGTH)
			print_error("timestamp is too large");
		if(str[i] < '0' || str[i] > '9')
			print_error("timestamp must be number");
		timestamp[timestamp_i++] = str[i++];
	}
	timestamp[timestamp_i] = '\0';
	i++;
	return i;
}

int exponentiation(int base, int power)
{
	int result = 1;
	while(power)
	{
		result *= base;
		power--;
	}
	return result;
}

int process_amount(char * str, int i, int * amount_value)
{
	int amount_i = 0, amount_t = 0, digit = 0;
	char amount[MAX_AMOUNT_LENGTH + 1];
	while(str[i] != '\t')
	{
		if(amount_i >= MAX_AMOUNT_LENGTH)
			print_error("amount is too large");
		if(str[i] >= '0' && str[i] <= '9')
			amount[amount_i++] = str[i++];
		else if(str[i] == '.')
		{
			if(++amount_t > 1)
				print_error("wrong format of amount");
			else // skip the '.' symbol
			{
				i++;
				digit = i;
			}
		}
		else
			print_error("wrong format of amount");
	}
	amount[amount_i] = '\0';
	digit = i - digit;
	if(digit != 2)
		print_error("wrong format of amount");
	*amount_value = atoi(amount);
	if(*amount_value <= 0)
		print_error("amount must be a positive value");
	i++;
	return i;
}

int process_description(char * str, int i, char * description)
{
	int description_i = 0;
	while(str[i] != '\0')
	{
		if(i >= MAX_LINE_LENGTH)
			print_error("one line is too long");
		if(str[i] == '\t')
			print_error("wrong format of file");
		if(str[i] == '\n')
			i++;
		description[description_i++] = str[i++];
	}
	description[description_i] = '\0';
	return i;
}

void check_time(int time_value)
{
	unsigned int current = time(NULL);
	if(current < time_value)
		print_error("the timestamp is exceed the current time");
}

My402TransData * parse_file(char * str)
{
	// Data structure to store one line of file
	My402TransData * data;

	if(str[MAX_LINE_LENGTH] != '\0')
		print_error("one line in the file is too long");
	else
	{
		char type, timestamp[MAX_TIMESTAMP_LENGTH + 1];
		int amount;
		// status indicate which kind data currently read, 0:type; 1:timestamp; 2:amount; 3:description
		int i = 0;
		enum readStatus status = 0;

		while(str[i] != '\0')
		{
			switch(status)
			{
				case Type: // read type
					data = (My402TransData *)malloc(sizeof(My402TransData));
					if(!data)
						print_error("malloc space failed");
					i = process_type(str, i, &type);
					status++;
					data -> type = type;
					break;
				case Timestamp: // read timestamp
					i = process_timestamp(str, i, timestamp);
					status++;
					unsigned int time_value = atoi(timestamp);
					check_time(time_value);
					data -> timestamp = time_value;
					break;
				case Amount: // read amount
					i = process_amount(str, i, &amount);
					status++;
					data -> amount = amount;
					break;
				case Description: // read description
					i = process_description(str, i, data -> description);
					status++;
					break;
				default:
					print_error("parse file error");
					break;
			}
		}
	}
	return data;
}

void insert_list(My402List * list, My402TransData * data)
{
	if(My402ListEmpty(list))
		(void)My402ListAppend(list, (void*)data);
	else
	{
		My402ListElem * elem = My402ListFirst(list);
		while(elem)
		{
			My402TransData * temp = (My402TransData *)(elem -> obj);
			if(temp -> timestamp < data -> timestamp)
			{
				elem = My402ListNext(list, elem);
				if(!elem)
					(void)My402ListAppend(list, (void*)data);
			}
			else if(temp -> timestamp > data -> timestamp)
			{
				(void)My402ListInsertBefore(list, (void*)data, elem);
				break;
			}
			else
				print_error("two identical timestamp");
		}
	}
}

void print_line()
{
	fprintf(stdout, "+-----------------+--------------------------+----------------+----------------+\n");
}

void print_title()
{
	print_line();
	fprintf(stdout, "|       Date      | Description              |         Amount |        Balance |\n");
	print_line();
}

int cal_amount(int amount, char type)
{
	if(type == '-')
		amount = 0 - amount;
	return amount;
}

int cal_balance(int balance, int amount, char type)
{
	if(type == '+')
		balance += amount;
	else
		balance -= amount;
	return balance;
}

void format_time(char * to, char * from)
{
	int i = 0, j = 0;
	while(from[j] != '\n')
	{
		// skip the time
		if(j == 11)
			j = 20;
		to[i++] = from[j++];
	}
	to[i] = '\0';
}

void format_description(char * to, char * from)
{
	int i = 0;
	while(from[i] != '\0' && i < 24)
	{
		to[i] = from[i];
		i++;
	}
	to[i] = '\0';
}

void format_money(char * to, int money)
{
	int neg = 0;
	if(money < 0)
	{
		neg = 1;
		money = 0 - money;
	}
	snprintf(to, 15, "              ");
	int i = 12, j = 1;
	while((money > 0 || j <= 3) && j < MAX_AMOUNT_LENGTH + 1)
	{
		int rem = money % 10;
		money = money / 10;

		if(j == 3)
			to[i--] = '.';
		else if((j - 3) % 3 == 0)
			to[i--] = ',';
		to[i--] = '0' + rem;
		j++;
	}
	if(j > MAX_AMOUNT_LENGTH)
		snprintf(to, 15, " ?,???,???.?? ");
	else
		while(i > 0)
			to[i--] = ' ';
	if(neg)
	{
		to[0] = '(';
		to[13] = ')';
	}
}

void print_data(char * date, char * description, char * amount, char * balance)
{
	char buffer[LINE_LENGTH + 1];
	snprintf(buffer, LINE_LENGTH + 1, "| %15s | %-24s | %14s | %14s |", date, description, amount, balance);
	fprintf(stdout, "%s\n", buffer);
}

void print_list(My402List *list)
{
	print_title();

    My402ListElem * elem = NULL;
    long long balance_value = 0;
    int amount_value = 0;
    for (elem = My402ListFirst(list); elem != NULL; elem = My402ListNext(list, elem))
    {
        My402TransData * data = (My402TransData *)(elem -> obj);
        char buffer[26], date[16], description[25], amount[15], balance[15];
        time_t timestamp;
        timestamp = data -> timestamp;
        strncpy(buffer, ctime(&timestamp), sizeof(buffer));
        format_time(date, buffer);
        format_description(description, data -> description);
        amount_value = cal_amount(data -> amount, data -> type);
        format_money(amount, amount_value);
        balance_value = cal_balance(balance_value, data -> amount, data -> type);
        format_money(balance, balance_value);
        print_data(date, description, amount, balance);
    }
    print_line();
}

void process(char * file_name)
{
	struct stat info;
	stat(file_name, &info);
	if(S_ISDIR(info.st_mode))
		print_error("the path is a directory");


    char buffer[MAX_LINE_LENGTH + 2];
    buffer[MAX_LINE_LENGTH] = '\0';

	My402List list;
	memset(&list, 0, sizeof(My402List));
	(void)My402ListInit(&list);

	FILE * file;
	if(file_name)
	{
	    if((file = fopen(file_name, "r")) == NULL)
			print_error("cannot open file");
	}
	else
		file = stdin;

	while(!feof(file))
	{
	   	if(fgets(buffer, MAX_LINE_LENGTH + 2, file))
	   		insert_list(&list, parse_file(buffer));
	}
	fclose(file);
   	print_list(&list);

}

int main(int argc, char * argv[])
{
	if(commandline(argc, argv))
		process(argv[2]);
	else
		process(NULL);

    return(0);
}





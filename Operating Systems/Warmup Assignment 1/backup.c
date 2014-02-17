/* This read file function read file character by character */
void read_file(char * file_name)
{
	FILE *fp;
	// MAX_LINE_LENGTH = 1024; MAX_TIMESTAMP_LENGTH = 10; MAX_AMOUNT_LENGTH = 10
	char ch, type, timestamp[MAX_TIMESTAMP_LENGTH + 1], amount[MAX_AMOUNT_LENGTH + 1], description[MAX_LINE_LENGTH];
	// status indicate which kind data currently read, 0:type; 1:timestamp; 2:amount; 3:description
	int status = 0, timestamp_i, amount_i, amount_t, description_i, description_max;
	// Doubly-linked circular list
	My402List list;
	memset(&list, 0, sizeof(My402List));
	(void)My402ListInit(&list);

	if((fp = fopen(file_name,"r")) == NULL)
		print_error("cannot open file");

	while((ch = fgetc(fp)) != EOF)
	{
		switch(status)
		{
			case 0: // read type
				if(ch != '-' && ch != '+')
					print_error("undefined transcation type");
				else
				{
					type = ch;
					ch = fgetc(fp);
					if(ch != '\t' && ch != EOF)
						print_error("wrong format of file");
					else
						status++;
				}
				break;
			case 1: // read timestamp
				timestamp_i = 0;
				while(ch != '\t' && ch != EOF)
				{
					if(timestamp_i >= MAX_TIMESTAMP_LENGTH)
						print_error("timestamp is too large");
					if(ch < '0' || ch > '9')
						print_error("timestamp must be number");
					timestamp[timestamp_i++] = ch;
					ch = fgetc(fp);
				}
				timestamp[timestamp_i] = '\0';
				status++;
				break;
			case 2: // read amount
				amount_i = 0;
				amount_t = 0;
				while(ch != '\t' && ch != EOF)
				{
					if(amount_i >= MAX_AMOUNT_LENGTH)
						print_error("amount is too large");
					if(ch >= '0' && ch <= '9')
					{
						amount[amount_i++] = ch;
						ch = fgetc(fp);
					}
					else if(ch == '.')
					{
						if(++amount_t > 1)
							print_error("wrong format of amount");
						else
						{
							// skip the '.' symbol
							// amount[amount_i++] = ch;
							ch = fgetc(fp);
						}
					}
					else
						print_error("wrong format of amount");
				}
				amount[amount_i] = '\0';
				status++;
				break;
			case 3: // read description
				description_i = 0;
				description_max = MAX_LINE_LENGTH - amount_i - timestamp_i;
				while(ch != '\n' && ch != EOF)
				{
					if(description_i >= description_max)
						print_error("description is too long");
					if(ch == '\t')
						print_error("wrong format of file");
					description[description_i++] = ch;
					ch = fgetc(fp);
				}
				description[description_i] = '\0';

				encapsulate_data(type, timestamp, amount, description);

				status = 0;
				break;
			default:
				print_error("parse file error");
				break;
		}
	}
	fclose(fp);
}

/* format the line by using array not snprintf */
void format_line(char * line)
{
	int i;
	for(i = 0; i < LINE_LENGTH; i++)
	{
		if(i == 0 || i == 18 || i == 45 || i == 62 || i == 79)
			line[i] = '|';
		else
			line[i] = ' ';
	}
	line[i] = '\0';
}

void format_description(char * line, char * description, int i)
{
	int j = 0;
	line[i++] = ' ';
	while(description[j] != '\0' && i < 45)
		line[i++] = description[j++];
	while(i < 45)
		line[i++] = ' ';
}

void format_time(char * line, char * buffer, int i)
{
	int j = 0;
	while(buffer[j] != '\n')
	{
		// skip the time
		if(j == 11)
			j = 20;
		line[i++] = buffer[j++];
	}
	line[i] = ' ';
}

char int_to_char(int i)
{
	switch(i)
	{
		case 0:
			return '0';
		case 1:
			return '1';
		case 2:
			return '2';
		case 3:
			return '3';
		case 4:
			return '4';
		case 5:
			return '5';
		case 6:
			return '6';
		case 7:
			return '7';
		case 8:
			return '8';
		case 9:
			return '9';
		default:
			return 'N';
	}
}

void format_money(char * line, int money, int start, int end)
{
	line[start] = ' ';
	line[end] = ' ';
	int neg = 0;
	if(money < 0)
	{
		neg = 1;
		money = 0 - money;
	}
	int i = end - 2, j = 1;
	while(money > 0 && j < MAX_AMOUNT_LENGTH + 1)
	{
		int rem = money % 10;
		money = money / 10;
		char ch = int_to_char(rem);
		
		if(j == 3)
			line[i--] = '.';
		else if((j - 3) % 3 == 0)
			line[i--] = ',';
		line[i--] = ch;
		j++;
	}
	if(j > MAX_AMOUNT_LENGTH)
	{
		i = end - 2;
		j = 1;
		//(?,???,???.??)
		while(i > (start + 1))
		{
			if(j == 3)
				line[i--] = '.';
			else if((j - 3) % 3 == 0)
				line[i--] = ',';
			line[i--] = '?';
			j++;
		}
	}
	else
		while(i > (start + 1))
			line[i--] = ' ';
	if(neg)
	{
		line[start + 1] = '(';
		line[end - 1] = ')';
	}
	else
	{
		line[start + 1] = ' ';
		line[end - 1] = ' ';
	}
}

void print_list(My402List *list)
{
	print_title();

    My402ListElem * elem = NULL;
    int balance_value = 0, amount_value = 0;
    for (elem = My402ListFirst(list); elem != NULL; elem = My402ListNext(list, elem))
    {
        My402TransData * data = (My402TransData *)(elem -> obj);
        char line[LINE_LENGTH + 1], buffer[26];
        time_t timestamp;
        timestamp = data -> timestamp;
        strncpy(buffer, ctime(&timestamp), sizeof(buffer));
        format_line(line);
        format_time(line, buffer, 2);
        format_description(line, data -> description, 19);
        amount_value = cal_amount(data -> amount, data -> type);
        format_money(line, amount_value, 46, 61);
        balance_value = cal_balance(balance_value, data -> amount, data -> type);
        format_money(line, balance_value, 63, 78);
        fprintf(stdout, "%s\n", line);
    }
    print_line();
}
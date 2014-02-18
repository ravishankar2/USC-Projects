#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <sys/time.h>

#include "cs402.h"
#include "my402list.h"
#include "warmup2.h"

// one thread for packet arrival
// one thread for token arrival
// one thread for server
// must not use one thread for each packet.
// In addition, use at least one mutex to protect Q1, Q2, and the token bucket.
// Finally, Q1 and Q2 must have infinite capacity

/*---------------------------------Global---------------------------------*/

pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t Queue_Not_Empty_Cond = PTHREAD_COND_INITIALIZER;
long int Queue_Amount_Guard = 0;

pthread_t packet_id, token_id, server_id, int_id, extra_packet_id;

double Lambda, R, Mu;

long int Num, Left_Num = 0, B, P, Bucket = 0;

int Mode, Debug = 0;

My402List Q1, Q2, Packet_List, Collect_List;

struct timeval Start_Time;

sigset_t Signal_Mask;

int Server_Stop = 0, Extra_Packet_Stop = 0;

char * Command_Parameter[DEFAULT_COMMAND_PARAMETER_NUMBER];

long int Total_Token = 0, Discard_Token = 0;
long int Total_Packet = 0, Discard_Packet = 0;

void * prepare_extra_packet();

/*---------------------------------Utility---------------------------------*/

void error(char * message)
{
	fprintf(stderr, "Error: %s, program abort!\n", message);
	exit(1);
}

double time_use(struct timeval End_Time)
{
	// double timeu  = SEC_TO_MSEC * (End_Time.tv_sec - Start_Time.tv_sec) + End_Time.tv_usec - Start_Time.tv_usec;
	// timeu /= SEC_TO_MISEC;
	double timeu = SEC_TO_MISEC * (End_Time.tv_sec - Start_Time.tv_sec) + ((double)(End_Time.tv_usec - Start_Time.tv_usec)) / SEC_TO_MISEC;
	return timeu;
}

void debug(char * message)
{
	fprintf(stdout, "Debug: %s.\n", message);
}

int command_index(char * _command)
{
	if(!strcmp(_command, "-lambda"))
		return LAMBDA_PARAMETER;
	if(!strcmp(_command, "-mu"))
		return MU_PARAMETER;
	if(!strcmp(_command, "-r"))
		return R_PARAMETER;
	if(!strcmp(_command, "-B"))
		return B_PARAMETER;
	if(!strcmp(_command, "-P"))
		return P_PARAMETER;
	if(!strcmp(_command, "-n"))
		return NUM_PARAMETER;
	if(!strcmp(_command, "-t"))
		return TSFILE_PARAMETER;
	return ERROR;
}

void command_input(int argc, char * argv[])
{
	int i, index;
	for(i = 0; i < DEFAULT_COMMAND_PARAMETER_NUMBER; i++)
		Command_Parameter[i] = 0;
	for(i = 1; i < argc; i++)
	{
		if((index = command_index(argv[i])) == ERROR)
			error("undefined command");
		if(++i >= argc)
			error("command value missing");
		Command_Parameter[index] = argv[i];
	}
}

double real_parse(char * str)
{
	int i = 0, flag = 0;
	while(str[i] != '\0' && str[i] != '\n')
	{
		if(str[i] < '0' || str[i] > '9')
		{
			if(str[i] == '.' && flag == 0)
				flag++;
			else
				error("command value error");
		}
		i++;
	}
	return atof(str);
}

long int int_parse(char * str)
{
	int i = 0;
	long long int value = 0;
	while(str[i] != '\0' && str[i] != '\n')
	{
		if(str[i] >= '0' && str[i] <= '9')
		{
			value += (str[i++] - '0');
			value *= 10;
		}
		else
			error("command value error");
	}
	value /= 10;
	if(Debug)
		fprintf(stdout, "Debug: in int_parse value %lld\n", value);
	if(value > MAX_B_P_NUM)
		error("poitive integers must not large than 2147483647");
	return value;
}

double max_limit(double interval)
{
	if(interval <= 0 || interval >= (10 * SEC_TO_MSEC))
		interval = 10 * SEC_TO_MSEC;
	return interval;
}

/*---------------------------------Initial---------------------------------*/

void command_parse()
{
	if(Command_Parameter[TSFILE_PARAMETER] != 0)
	{
		// run in trace-driven mode
		Mode = TRACE_DRIVEN;
		if(Debug)
			debug("run in trace-driven mode");
	}
	else
	{
		// run in deterministic mode
		Mode = DETERMINISTIC;
		if(Debug)
			debug("run in deterministic mode");
		if(Command_Parameter[LAMBDA_PARAMETER] != 0)
			Lambda = real_parse(Command_Parameter[LAMBDA_PARAMETER]);
		else
			Lambda = DEFAULT_PACKET_ARRIVE_RATE;

		if(Command_Parameter[MU_PARAMETER] != 0)
			Mu = real_parse(Command_Parameter[MU_PARAMETER]);
		else
			Mu = DEFAULT_PACKET_SERVE_RATE;
		if(Command_Parameter[P_PARAMETER] != 0)
			P = int_parse(Command_Parameter[P_PARAMETER]);
		else
			P = DEFAULT_TOKEN_REQUIRE_AMOUNT;
		if(Command_Parameter[NUM_PARAMETER] != 0)
		{
			if(Debug)
				fprintf(stdout, "Debug: -n %s\n", Command_Parameter[NUM_PARAMETER]);
			Num = int_parse(Command_Parameter[NUM_PARAMETER]);
			if(Debug)
				fprintf(stdout, "Debug: -n long int %ld\n", Num);
		}
		else
			Num = DEFAULT_TOTAL_PACKET_NUMBER;
	}

	if(Command_Parameter[R_PARAMETER] != 0)
		R = real_parse(Command_Parameter[R_PARAMETER]);
	else
		R = DEFAULT_TOKEN_ARRIVE_RATE;
	if(Command_Parameter[B_PARAMETER] != 0)
		B = int_parse(Command_Parameter[B_PARAMETER]);
	else
		B = DEFAULT_TOKEN_BUCKET_CAPACITY;
}

void prepare_packet()
{
	memset(&Q1, 0, sizeof(My402List));
	(void)My402ListInit(&Q1);
	memset(&Q2, 0, sizeof(My402List));
	(void)My402ListInit(&Q2);
	memset(&Packet_List, 0, sizeof(My402List));
	(void)My402ListInit(&Packet_List);
	memset(&Collect_List, 0, sizeof(My402List));
	(void)My402ListInit(&Collect_List);

	if(Mode == DETERMINISTIC)
	{
		int i;
		double interval;
		My402Packet * packet;
		int num;
		if(Num > MAX_PROCESS_NUM)
		{
			num = MAX_PROCESS_NUM;
			Left_Num = Num - num;
		}
		else
		{
			num = Num;
		}
		for(i = 0; i < num; i++)
		{
			packet = (My402Packet *)malloc(sizeof(My402Packet));
			interval = (1 / Lambda) * SEC_TO_MSEC;
			interval = max_limit(interval);
			packet -> inter_arrival_time = interval;
			packet -> tokens_need = P;
			interval = (1 / Mu) * SEC_TO_MSEC;
			interval = max_limit(interval);
			packet -> service_time = interval;
			(void)My402ListAppend(&Packet_List, (void *)packet);
		}
		if(Left_Num > 0)
		{
			// need to create a new thread to handle large amount of packet
			if(pthread_create(&extra_packet_id, 0, prepare_extra_packet, 0))
				error("create extra packet prepare thread failed");
		}
	}

	// read from file
	if(Mode == TRACE_DRIVEN)
	{
		FILE * file;
		char buffer[MAX_LINE_LENGTH + 2];
		buffer[MAX_LINE_LENGTH] = '\0';
		My402Packet * packet;
		if((file = fopen(Command_Parameter[TSFILE_PARAMETER], "r")) == NULL)
			error("cannot open file");
		// read the number
		if(fgets(buffer, MAX_LINE_LENGTH + 2, file))
	   	{
	   		if(buffer[MAX_LINE_LENGTH] != 0)
	   			error("one line in the file is too long");
	   		Num = int_parse(buffer);
	   	}
	   	int i, index = -1;
	   	char temp_num[3][MAX_LINE_LENGTH];
	   	// index 0:packet arrival time 1:token need 2:server time
		while(!feof(file))
		{
			i = 0, index = -1;
			// read one line
	   		if(fgets(buffer, MAX_LINE_LENGTH + 2, file))
	   		{
	   			// parse the line
	   			while(buffer[i] != '\0' && buffer[i] != '\n')
	   			{
	   				if(buffer[i] == ' ' || buffer[i] == '\t')
	   				{
	   					i++;
	   					continue;
	   				}
	   				index++;
	   				if(index >= 3)
	   					break;
	   				int k = 0;
	   				while(buffer[i] != ' ' && buffer[i] != '\t' && buffer[i] != '\0')
	   				{
	   					temp_num[index][k++] = buffer[i];
	   					i++;
	   				}
	   				temp_num[index][k] = '\0';
	   			}
	   			packet = (My402Packet *)malloc(sizeof(My402Packet));
				packet -> inter_arrival_time = real_parse(temp_num[0]) * SEC_TO_MISEC;
				packet -> tokens_need = int_parse(temp_num[1]);
				packet -> service_time = real_parse(temp_num[2]) * SEC_TO_MISEC;
				(void)My402ListAppend(&Packet_List, (void *)packet);
	   		}
	   	}
	   	fclose(file);
	}
}

void show_parameters()
{
	fprintf(stdout, "Emulation Parameters:\n");
	fprintf(stdout, "\tlambda = %g\t\t\t(if -t is not specified)\n", Command_Parameter[LAMBDA_PARAMETER] == 0 ? DEFAULT_PACKET_ARRIVE_RATE : Lambda);
	fprintf(stdout, "\tmu = %g\t\t\t(if -t is not specified)\n", Command_Parameter[MU_PARAMETER] == 0 ? DEFAULT_PACKET_SERVE_RATE : Mu);
	fprintf(stdout, "\tr = %g\n", R);
	fprintf(stdout, "\tB = %ld\n", B);
	fprintf(stdout, "\tP = %ld\t\t\t\t(if -t is not specified)\n", P);
	fprintf(stdout, "\tnumber to arrive = %ld\t\t(if -t is not specified)\n", Num);
	fprintf(stdout, "\ttsfile = %s\t\t\t(if -t is specified)\n\n", Command_Parameter[TSFILE_PARAMETER] == 0 ? "NULL" : Command_Parameter[TSFILE_PARAMETER]);
}

void initial(int argc, char * argv[])
{
	sigemptyset(&Signal_Mask);
	sigaddset (&Signal_Mask, SIGINT);
	sigaddset (&Signal_Mask, SIGUSR1);
	sigaddset (&Signal_Mask, SIGUSR2);
	if(pthread_sigmask(SIG_BLOCK, &Signal_Mask, 0))
		error("set main thread signal mask failed");
	if(Debug)
		debug("initializing...");
	command_input(argc, argv);
	command_parse();
	show_parameters();
	prepare_packet();
	if(Debug)
		debug("initialize finished");
}

/*---------------------------------Extra Packet Prepare Thread---------------------------------*/

void * prepare_extra_packet()
{
	int num = 0, i;
	double interval;
	My402Packet * packet;
	if(Debug)
		debug("handle extra packet");
	while(Left_Num > 0 && Extra_Packet_Stop == 0)
	{
		if(Debug)
			fprintf(stdout, "Debug: left num = %ld\n", Left_Num);
		if(Left_Num > MAX_PROCESS_NUM)
		{
			num = MAX_PROCESS_NUM;
			Left_Num -= MAX_PROCESS_NUM;
		}
		else
		{
			num = Left_Num;
			Left_Num = 0;
		}
		pthread_mutex_lock(&Mutex);
		for(i = 0; i < num; i++)
		{
			packet = (My402Packet *)malloc(sizeof(My402Packet));
			interval = (1 / Lambda) * SEC_TO_MSEC;
			interval = max_limit(interval);
			packet -> inter_arrival_time = interval;
			packet -> tokens_need = P;
			interval = (1 / Mu) * SEC_TO_MSEC;
			interval = max_limit(interval);
			packet -> service_time = interval;
			(void)My402ListAppend(&Packet_List, (void *)packet);
		}
		pthread_mutex_unlock(&Mutex);
		usleep(SEC_TO_MSEC);
	}
	if(Debug)
		debug("handle extra packet stop");
	return(0);
}

/*---------------------------------Token Thread---------------------------------*/

void token_cleanup(int sig)
{
    pthread_mutex_trylock(&Mutex);
    My402ListUnlinkAll(&Q2);
    Queue_Amount_Guard = 0;
    pthread_mutex_unlock(&Mutex);
	if(Debug)
		debug("token arrival thread stop");
    pthread_exit(0);
}

// the token arrival thread sits in a loop
// sleeps for an interval, trying to match a given interarrival time for tokens
// wakes up, locks mutex, try to increment token count
// check if it can move a packet from Q1 to Q2
// if packet is added to Q2 and Q2 was empty before, signal or broadcast a queue-not-empty condition
// unlocks mutex
// goes back to sleep for the "right" amount
void * token_arrival(void * arg)
{
	double interval = 1 / R * SEC_TO_MSEC;
	interval = max_limit(interval);

	struct timeval end_time;
	double token_arrival_time, token_last_end_time, token_last_start_time = 0;
	double sleep_time;

	// unblock the SIGUSR2 signal
	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset (&signal_mask, SIGUSR2);
	if(pthread_sigmask(SIG_UNBLOCK, &signal_mask, 0))
		error("set packet arrival thread signal mask failed");
	// bind the signal handler
	// signal(SIGUSR2, token_cleanup);
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = token_cleanup;
	sigaction(SIGUSR2, &act, NULL);

	while(TRUE)
	{
		gettimeofday(&end_time, 0);
		token_last_end_time = time_use(end_time);

		sleep_time = interval - token_last_end_time + token_last_start_time;
		if(sleep_time < 0)
			sleep_time = 1;

		usleep(sleep_time);

		Total_Token++;
		pthread_mutex_lock(&Mutex);

		if(My402ListEmpty(&Packet_List) && My402ListEmpty(&Q1))
		{
			if(My402ListEmpty(&Q2))
				pthread_cond_broadcast(&Queue_Not_Empty_Cond);

			pthread_mutex_unlock(&Mutex);
			break;
		}

		gettimeofday(&end_time, 0);
		token_arrival_time = time_use(end_time);

		if(Bucket < B)
		{
			if(Debug)
				debug("Bucket++");

			Bucket++;
			fprintf(stdout, "%012.3fms: token t%ld arrives, token bucket now has %ld token\n", token_arrival_time, Total_Token, Bucket);
		}
		else
		{
			if(Debug)
				debug("Token discard");

			Discard_Token++;
			fprintf(stdout, "%012.3fms: token t%ld arrives, dropped\n", token_arrival_time, Total_Token);
		}

		token_last_start_time = token_arrival_time;

		if(!My402ListEmpty(&Q1))
		{
			// see if there is enough tokens to make the packet at the head of Q1 be eligiable for transmissions
			My402ListElem * elem = My402ListFirst(&Q1);
			My402Packet * packet = (My402Packet *)(elem -> obj);
			if(Bucket >= packet -> tokens_need)
			{
				// If it does, it will remove the corresponding number of tokens from the token bucket
				Bucket -= packet -> tokens_need;
				// remove that packet from Q1 and move it into Q2
				(void)My402ListUnlink(&Q1, elem);
				gettimeofday(&end_time, 0);
				packet -> time_end_from_Q1 = time_use(end_time);

				fprintf(stdout, "%012.3fms: p%ld leaves Q1, time in Q1 = %gms, token bucket now has %ld token\n",
					packet -> time_end_from_Q1, packet -> packet_num, packet -> time_end_from_Q1 - packet -> time_start_in_Q1, Bucket);


				int empty_before = My402ListEmpty(&Q2);
				(void)My402ListAppend(&Q2, (void *)packet);
				gettimeofday(&end_time, 0);

				packet -> time_start_in_Q2 = time_use(end_time);

				fprintf(stdout, "%012.3fms: p%ld enters Q2\n", packet -> time_start_in_Q2, packet -> packet_num);


				if(Debug)
					debug("move packet from Q1 to Q2, Queue_Amount_Guard++");
				Queue_Amount_Guard++;
				// wake up the server (by broadcasting the corresponding condition)
				if(empty_before)
					pthread_cond_broadcast(&Queue_Not_Empty_Cond);
				if(Debug)
					debug("wake up the server");
			}
		}
		pthread_mutex_unlock(&Mutex);
	}
	if(Debug)
		debug("token arrival thread stop");
	return(0);
}

/*---------------------------------Packet Thread---------------------------------*/

void packet_cleanup(int sig)
{
    pthread_mutex_trylock(&Mutex);
    My402ListUnlinkAll(&Packet_List);
    My402ListUnlinkAll(&Q1);
    pthread_mutex_unlock(&Mutex);
	if(Debug)
		debug("packet arrival thread stop");
    pthread_exit(0);
}

// the arrival thread sits in a loop
// sleeps for an interval, trying to match a given interarrival time (from trace or deterministic)
// wakes up, creates a packet object, locks mutex enqueues the packet to Q1 or Q2
// if Q2 was empty before, need to signal or broadcast a queue-not-empty condition
// unlocks mutex
// goes back to sleep for the "right" amount
void * packet_arrival(void * arg)
{
	My402ListElem * elem;
	My402Packet * packet;
	struct timeval end_time;
	double packet_last_end_time, packet_last_start_time = 0;
	double sleep_time;

	// unblock the SIGUSR1 signal
	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset (&signal_mask, SIGUSR1);
	if(pthread_sigmask(SIG_UNBLOCK, &signal_mask, 0))
		error("set packet arrival thread signal mask failed");
	// bind the signal handler
	// signal(SIGUSR1, packet_cleanup);
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = packet_cleanup;
	sigaction(SIGUSR1, &act, NULL);

	while(TRUE)
	{
		pthread_mutex_lock(&Mutex);

		// if no more packet will arrive, exit the thread
		if(My402ListEmpty(&Packet_List))
		{
			pthread_mutex_unlock(&Mutex);
				break;
		}

		elem = My402ListFirst(&Packet_List);

		packet = (My402Packet *)(elem -> obj);

		Total_Packet++;
		pthread_mutex_unlock(&Mutex);

		gettimeofday(&end_time, 0);
		packet_last_end_time = time_use(end_time);

		// sleep
		sleep_time = packet -> inter_arrival_time - packet_last_end_time + packet_last_start_time;
		if(sleep_time < 0)
			sleep_time = 1;
		usleep(sleep_time);

		pthread_mutex_lock(&Mutex);

		gettimeofday(&end_time, 0);
		packet -> time_packet_arrival = time_use(end_time);

		if(packet -> tokens_need > B)
		{
			(void)My402ListUnlink(&Packet_List, elem);
			fprintf(stdout, "%012.3fms: packet p%ld arrives, needs %ld tokens, dropped\n", packet -> time_packet_arrival, Total_Packet, packet -> tokens_need);
			Discard_Packet++;
			pthread_mutex_unlock(&Mutex);
			free(packet);
			continue;
		}

		packet -> packet_num = Total_Packet;
		if(Debug)
			debug("packet arrival");
		
		// gettimeofday(&end_time, 0);
		// packet -> time_packet_arrival = time_use(end_time);

		fprintf(stdout, "%012.3fms: p%ld arrives, needs %ld tokens, inter-arrival time = %.3fms\n",
			packet -> time_packet_arrival, Total_Packet, packet -> tokens_need, packet -> time_packet_arrival - packet_last_start_time);

		packet_last_start_time = packet -> time_packet_arrival;

		if(Debug)
			debug("enqueues the packet to Q1");
		// enqueues the packet to Q1
		(void)My402ListAppend(&Q1, (void *)packet);
		gettimeofday(&end_time, 0);
		packet -> time_start_in_Q1 = time_use(end_time);

		fprintf(stdout, "%012.3fms: p%ld enters Q1\n", packet -> time_start_in_Q1, packet -> packet_num);


		// if token's number is enough, and no packet waiting in the Q1
		if(Bucket >= packet -> tokens_need && (My402ListLength(&Q1) == 1))
		{
			// remove that packet from Q1 and move it into Q2
			My402ListElem * elem_temp = My402ListFirst(&Q1);
			(void)My402ListUnlink(&Q1, elem_temp);
			gettimeofday(&end_time, 0);
			packet -> time_end_from_Q1 = time_use(end_time);

			fprintf(stdout, "%012.3fms: p%ld leaves Q1, time in Q1 = %gms, token bucket now has %ld token\n",
				packet -> time_end_from_Q1, packet -> packet_num, packet -> time_end_from_Q1 - packet -> time_start_in_Q1, Bucket);

			if(Debug)
				debug("enqueues the packet to Q2, Queue_Amount_Guard++");
			int empty_before = My402ListEmpty(&Q2);
			// enqueues the packet to Q2
			Bucket -= packet -> tokens_need;
			(void)My402ListAppend(&Q2, (void *)packet);
			Queue_Amount_Guard++;
			gettimeofday(&end_time, 0);
			packet -> time_start_in_Q2 = time_use(end_time);

			fprintf(stdout, "%012.3fms: p%ld enters Q2\n", packet -> time_start_in_Q2, packet -> packet_num);

			if(empty_before)
			{
				pthread_cond_broadcast(&Queue_Not_Empty_Cond);
				if(Debug)
					debug("wake up the server");
			}
		}
		(void)My402ListUnlink(&Packet_List, elem);

		pthread_mutex_unlock(&Mutex);
	}
	if(Debug)
		debug("packet arrival thread stop");
	return(0);
}

/*---------------------------------Statistics---------------------------------*/

void packet_statistics()
{
	if(My402ListEmpty(&Q2))
	{
		My402ListUnlinkAll(&Q2);
 		Queue_Amount_Guard = 0;
	}

	pthread_mutex_unlock(&Mutex);

	My402ListElem * elem;
	My402Packet * packet;
	struct timeval end_time;
	double total_emulation_time = 0, all_time_spent_in_Q1 = 0, all_time_spent_in_Q2 = 0, all_time_spent_in_S = 0, 
	all_arrival_time = 0, packet_last_arrival_time = 0, all_server_time = 0, all_system_time = 0, deviation = 0;
	long int served_packet_num = 0;

	gettimeofday(&end_time, 0);
	total_emulation_time = time_use(end_time);
	elem = My402ListFirst(&Collect_List);
	while(elem)
	{
		packet = (My402Packet *)(elem -> obj);

		all_arrival_time += (packet -> time_packet_arrival - packet_last_arrival_time);
		packet_last_arrival_time = packet -> time_packet_arrival;
		all_server_time += (packet -> time_end_from_S - packet -> time_start_in_S);

		all_time_spent_in_Q1 += (packet -> time_end_from_Q1 - packet -> time_start_in_Q1);
		all_time_spent_in_Q2 += (packet -> time_end_from_Q2 - packet -> time_start_in_Q2);
		all_time_spent_in_S += (packet -> time_end_from_S - packet -> time_start_in_S);

		all_system_time += (packet -> time_end_from_S - packet -> time_packet_arrival);

		served_packet_num++;
		
		elem = My402ListNext(&Collect_List, elem);
	}
	// if no packet, avoid error
	if(served_packet_num == 0)
		served_packet_num++;

	double average_time_in_system = all_system_time / served_packet_num / SEC_TO_MISEC;

	while(!My402ListEmpty(&Collect_List))
	{
		elem = My402ListFirst(&Collect_List);
		packet = (My402Packet *)(elem -> obj);
		(void)My402ListUnlink(&Collect_List, elem);

		double system_time = (packet -> time_end_from_S - packet -> time_packet_arrival) / SEC_TO_MISEC;
		double difference = system_time - average_time_in_system;
		deviation = deviation + difference * difference;

		free(packet);
	}

	deviation /= served_packet_num;

	fprintf(stdout, "\nStatistics:\n");

	fprintf(stdout, "\n\taverage packet inter-arrival time = %.6gs\n", all_arrival_time / served_packet_num / SEC_TO_MISEC);
	fprintf(stdout, "\n\taverage packet service time = %.6gs\n", all_server_time / served_packet_num / SEC_TO_MISEC);

	fprintf(stdout, "\n\taverage number of packets in Q1 = %.6g\n", all_time_spent_in_Q1 / total_emulation_time);
	fprintf(stdout, "\taverage number of packets in Q2 = %.6g\n", all_time_spent_in_Q2 / total_emulation_time);
	fprintf(stdout, "\taverage number of packets in S = %.6g\n", all_time_spent_in_S / total_emulation_time);

	fprintf(stdout, "\n\taverage time a packet spent in system = %.6gs\n", average_time_in_system);
	fprintf(stdout, "\tstandard deviation for time spent in system = %.6g\n", sqrt(deviation));

	fprintf(stdout, "\n\ttoken drop probability = %.6g\n", (double)Discard_Token / Total_Token);
	fprintf(stdout, "\tpacket drop probability = %.6g\n", (double)Discard_Packet / Total_Packet);

	pthread_exit(0);
}

/*---------------------------------Server Thread---------------------------------*/

// the server thread
// initially blocked, waiting for the queue-not-empty condition to be signaled
// when unblocked, mutex is locked
// if Q2 is not empty, dequeues a packet and unlock mutex
// 		sleeps for an interval matching the service time of the packet, eject the packet from the system
// 		lock mutex, check if Q2 is empty, etc.
// if Q2 is empty, go wait for the queue-not-empty condition to be signaled
void * server(void * arg)
{
	struct timeval end_time;

	while(TRUE)
	{
		pthread_mutex_lock(&Mutex);
		while(!(Queue_Amount_Guard > 0))
		{
			if(Debug)
				debug("server blocked");
			// handle the stop singal when Q2 has no packet
			if((Server_Stop) || (My402ListEmpty(&Packet_List) && My402ListEmpty(&Q1) && My402ListEmpty(&Q2)))
			{
				if(Debug)
					debug("server thread stop");
				packet_statistics();
			}
			pthread_cond_wait(&Queue_Not_Empty_Cond, &Mutex);
		}
		// handle the stop singal when Q2 has packet
		if(Server_Stop)
		{
			if(Debug)
					debug("server thread stop");
			packet_statistics();
		}
		My402ListElem * elem = My402ListFirst(&Q2);
		My402Packet * packet = (My402Packet *)(elem -> obj);
		(void)My402ListUnlink(&Q2, elem);

		if(Debug)
			debug("start service, Queue_Amount_Guard--");
		Queue_Amount_Guard--;

		pthread_mutex_unlock(&Mutex);

		gettimeofday(&end_time, 0);
		packet -> time_start_in_S = time_use(end_time);
		packet -> time_end_from_Q2 = packet -> time_start_in_S;

		fprintf(stdout, "%012.3fms: p%ld begin service as S, time in Q2 = %.3fms\n",
		packet -> time_start_in_S, packet -> packet_num, packet -> time_end_from_Q2 - packet -> time_start_in_Q2);

		usleep(packet -> service_time);
		if(Debug)
			debug("end service, eject the packet");

		gettimeofday(&end_time, 0);
		packet -> time_end_from_S = time_use(end_time);

		fprintf(stdout, "%012.3fms: p%ld departs from S, service time = %.3fms, time in system = %.3fms\n",
			packet -> time_end_from_S, packet -> packet_num, packet -> time_end_from_S - packet -> time_start_in_S,
			packet -> time_end_from_S - packet -> time_packet_arrival);
		(void)My402ListAppend(&Collect_List, (void *)packet);
	}
	if(Debug)
		debug("server thread stop");
	return(0);
}

/*---------------------------------<Cntrl+C>---------------------------------*/

// <Cntrl+C>
// arrival thread will stop generating packets and terminate
// 	the arrival thread needs to stop the token thread
// 	the arrival thread needs to clear out Q1 and Q2
// server threads must finish serving its current packet must print statistics for all packet seen
// 	need to make sure that packets deleted this way do not participate in certain statistics calculation
// ￼	you need to decide which ones and justify them
void * sigint_handler(void * arg)
{
	int sig;
	if(sigwait(&Signal_Mask, &sig))
		error("handler error");

	if(Debug)
		debug("catch <Ctrl+C> in interrupt");

	pthread_mutex_lock(&Mutex);
	fprintf(stdout, "\n");
	Extra_Packet_Stop = 1;
	pthread_kill(packet_id, SIGUSR1);
		if(Debug)
		debug("send SIGUSR1");
	pthread_kill(token_id, SIGUSR2);
	if(Debug)
		debug("send SIGUSR2");
	Server_Stop = 1;
	if(My402ListEmpty(&Q2))
		pthread_cond_broadcast(&Queue_Not_Empty_Cond);
	pthread_mutex_unlock(&Mutex);

	return(0);
}

/*---------------------------------Process---------------------------------*/

void process()
{
	gettimeofday(&Start_Time, 0);

	fprintf(stdout, "%012.3fms: emulation begins\n", time_use(Start_Time));

	// create the packet arrival thread
	if(pthread_create(&packet_id, 0, packet_arrival, 0))
		error("create packet arrival thread failed");
	// create the token depositing thread
	if(pthread_create(&token_id, 0, token_arrival, 0))
		error("create token depositing thread failed");
	// create the server thread
	if(pthread_create(&server_id, 0, server, 0))
		error("create server thread failed");
	// create the interrupt handler
	if(pthread_create(&int_id, 0, sigint_handler, 0))
		error("create interrupt SIGINT thread failed");
	// waiting for the child thread
	pthread_join(packet_id, 0);
	pthread_join(token_id, 0);
	pthread_join(server_id, 0);
}

/*---------------------------------Main---------------------------------*/

int main(int argc, char * argv[])
{
	// initial the arguments and packets
	initial(argc, argv);
	// create the child threads
	process();

	return(0);
}





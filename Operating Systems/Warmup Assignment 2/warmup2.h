#ifndef _WARMUP2_H_
#define _WARMUP2_H_

#ifndef SEC_TO_MSEC
#define SEC_TO_MSEC 1000000
#endif

#ifndef SEC_TO_MISEC
#define SEC_TO_MISEC 1000
#endif

#ifndef DEFAULT_PACKET_ARRIVE_RATE
#define DEFAULT_PACKET_ARRIVE_RATE 0.5
#endif

#ifndef DEFAULT_PACKET_SERVE_RATE
#define DEFAULT_PACKET_SERVE_RATE 0.35
#endif

#ifndef DEFAULT_TOKEN_ARRIVE_RATE
#define DEFAULT_TOKEN_ARRIVE_RATE 1.5
#endif

#ifndef DEFAULT_TOKEN_REQUIRE_AMOUNT
#define DEFAULT_TOKEN_REQUIRE_AMOUNT 3
#endif

#ifndef DEFAULT_TOKEN_BUCKET_CAPACITY
#define DEFAULT_TOKEN_BUCKET_CAPACITY 10
#endif

#ifndef DEFAULT_TOTAL_PACKET_NUMBER
#define DEFAULT_TOTAL_PACKET_NUMBER 20
#endif

#ifndef MAX_B_P_NUM 
#define MAX_B_P_NUM 2147483647
#endif

#ifndef MAX_PROCESS_NUM
#define MAX_PROCESS_NUM 10000
#endif

#ifndef MAX_LINE_LENGTH 
#define MAX_LINE_LENGTH 1024
#endif

#ifndef DEFAULT_COMMAND_PARAMETER_NUMBER
#define DEFAULT_COMMAND_PARAMETER_NUMBER 7
#endif

enum commandParameter
{
	LAMBDA_PARAMETER,
	MU_PARAMETER,
	R_PARAMETER,
	B_PARAMETER,
	P_PARAMETER,
	NUM_PARAMETER,
	TSFILE_PARAMETER,
	ERROR
};

enum runMode
{
	DETERMINISTIC,
	TRACE_DRIVEN
};

typedef struct tagMy402Packet {
	long int packet_num;
	// microsecond
	double time_packet_arrival;
	double time_start_in_Q1;
	double time_end_from_Q1;
	double time_start_in_Q2;
	double time_end_from_Q2;
	double time_start_in_S;
	double time_end_from_S;

    double inter_arrival_time;
    double service_time;
    long int tokens_need;
} My402Packet;

#endif







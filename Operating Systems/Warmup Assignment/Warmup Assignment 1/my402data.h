#ifndef MAX_TIMESTAMP_LENGTH
#define MAX_TIMESTAMP_LENGTH 10
#endif

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 1024
#endif

#ifndef MAX_AMOUNT_LENGTH
#define MAX_AMOUNT_LENGTH 10
#endif

#ifndef LINE_LENGTH
#define LINE_LENGTH 80
#endif

#ifndef _MY402DATA_H_
#define _MY402DATA_H_

enum readStatus{Type, Timestamp, Amount, Description};

// Data structure to store one line of file
typedef struct tagMy402TransData {
    char type;
    unsigned int timestamp;
    int amount;
    char description[MAX_LINE_LENGTH];
} My402TransData;

#endif /*_MY402DATA_H_*/




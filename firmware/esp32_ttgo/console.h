#include <unistd.h>
#include <stdint.h>

#define MAX_INPUT_LENGTH 64
#define MAX_ARGS_PER_LINE 8
#define MAX_ARG_LENGTH 8

class consoleClass {
public:
    consoleClass();
    int interface();
    int init();
private:
    void lr(char cmd[MAX_ARGS_PER_LINE][MAX_ARG_LENGTH]);
    void parse(char input[MAX_INPUT_LENGTH], int length);

    int _sessionConnected;
    long _lastConnection;
    char _input[MAX_INPUT_LENGTH];
    int _inputLength;    
};
extern consoleClass console;

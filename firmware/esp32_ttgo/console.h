#include <unistd.h>
#include <stdint.h>

#define MAX_INPUT_LENGTH 128
#define MAX_ARGS_PER_LINE 8
#define MAX_ARG_LENGTH 16

class consoleClass {
public:
    consoleClass();
    int interface();
    int init();
private:
    int lr(int argc, char *argv[]);
    int tx(int argc, char *argv[]);
    void parse(char input[MAX_INPUT_LENGTH], int length);

    int _sessionConnected;
    long _lastConnection;
    char _input[MAX_INPUT_LENGTH];
    int _inputLength;    
};
extern consoleClass console;

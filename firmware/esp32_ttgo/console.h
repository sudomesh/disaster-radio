#include <unistd.h>
#include <stdint.h>

#define MAX_INPUT_LENGTH 64

class consoleClass {
public:
    consoleClass();
    int interface();
    int init();
private:
    void parse(char input[MAX_INPUT_LENGTH], int length);

    int _sessionConnected;
    long _lastConnection;
    char _input[MAX_INPUT_LENGTH];
    int _inputLength;    
};
extern consoleClass console;

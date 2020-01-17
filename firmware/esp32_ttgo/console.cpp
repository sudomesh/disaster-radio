#include <Layer1.h>
#include <LoRaLayer2.h>
#include <console.h>

consoleClass::consoleClass() :

    _sessionConnected(0),
    _lastConnection(),
    _input(),
    _inputLength()
    {}

int consoleClass::lr(int argc, char *argv[]){
    int index;
    int opt;
    optind = 1;
    opterr = 0;
    while ((opt = getopt(argc, argv, "ar")) != -1){
        switch (opt){
            case 'a':
                Serial.printf("1: lora1:\r\n");
                Serial.printf("    address: ");
                LL2.printAddress(Layer1.localAddress());
                Serial.printf("\r\n");
                break;
            case 'r':
                LL2.printRoutingTable();
                break;
            case '?':
                Serial.printf("Usage: lr [ OPTIONS ]\r\n");
                Serial.printf("where  OPTIONS := { -a | -r }\r\n");
                break;
            default:
                Serial.printf("Something is broken\r\n");
                return 1;
        }
    }
    for (index = optind; index < argc; index++){
        Serial.printf("Non-option argument %s\n", argv[index]);
        return 1;
    }
    return 0;
}

int consoleClass::tx(int argc, char *argv[]){
    int index;
    int opt;
    optind = 1;
    opterr = 0;
    uint8_t message[240];
    uint8_t length = 0;
    uint8_t destination[ADDR_LENGTH] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    uint8_t type = 'c';
    uint8_t *hexOptArg;
    while ((opt = getopt(argc, argv, "m:d:t:")) != -1){
        switch (opt){
            case 'm':
                length = strlen(optarg);
                memcpy(&message, optarg, length);
                break;
            case 'd':
                hexOptArg = Layer1.charToHex(optarg);
                memcpy(&destination, hexOptArg, ADDR_LENGTH);
                break;
            case 't':
                type = optarg[0];
                break;
            case '?':
                Serial.printf("Usage: tx [-m <message>] [-d <destination> ] [-t <type> ]\r\n");
                Serial.printf("Where <message> is a string with no spaces!\r\n");
                Serial.printf("      <destination> is the destination mac address formatted as a1b2c3d4e5f6\r\n");
                Serial.printf("          if no destination is given, it will default to the broadcast address, ffffffffffff\r\n");
                Serial.printf("      <type> is a single character signfiying message type, e.g. `c` or `m`\r\n");
                Serial.printf("          if no type is given, will default to the chat type, 'c'\r\n");
            default:
                Serial.printf("Something is broken\r\n");
                return 1;
        }
    }
    for (index = optind; index < argc; index++){
        Serial.printf("Non-option argument %s\n", argv[index]);
        return 1;
    }
    Serial.printf("TXing: ");
    for(int i = 0 ; i < length ; i++){
        Serial.printf("%c", message[i]);
    }
    Serial.printf(" to ");
    for(int i = 0 ; i < ADDR_LENGTH ; i++){
        Serial.printf("%02x", destination[i]);
    }
    Serial.printf(" as type ");
    Serial.printf("'%c'", type);
    Serial.printf("\r\n");
    LL2.sendToLayer2(destination, type, message, length);
    return 0;
}

void consoleClass::parse(char input[MAX_INPUT_LENGTH], int length){
    char cmd[MAX_ARGS_PER_LINE][MAX_ARG_LENGTH];
    char *argv[8]; //= (char*) malloc(8 * sizeof(char));
    char *delim = " ";
    char *token = strtok(input, delim);
    int argc = 0;
    while((argc < MAX_ARGS_PER_LINE) && (token != NULL)){
        memcpy(&cmd[argc], token, MAX_ARG_LENGTH);
	token = strtok(NULL, delim);
        argv[argc] = &cmd[argc][0];
        argc++;
    }
    argv[argc] = NULL;
    if(strcmp(cmd[0], "lr") == 0){
        lr(argc, argv);
    }
    else if(strcmp(cmd[0], "tx") == 0){
        tx(argc, argv);
    }
    return;
}

int consoleClass::interface(){
    if(_sessionConnected == 0) {
        if(Layer1.getTime() - _lastConnection > 2000) {
            if(Serial.available() > 0) {
                char incoming = Serial.read();
                Serial.printf("     ___              __                            ___    \r\n");
                Serial.printf(" ___/ (_)__ ___ ____ / /____ ____      _______ ____/ (_)__ \r\n");
                Serial.printf("/ _  / (_-</ _ `(_-</ __/ -_) __/ _   / __/ _ `/ _  / / _ \\\r\n");
                Serial.printf("\\_,_/_/___/\\_,_/___/\\__/\\__/_/   (_) /_/  \\_,_/\\_,_/_/\\___/\r\n");
                Serial.printf("v0.1.1\r\n");
                if(Layer1.loraInitialized()){
                    Serial.printf("LoRa tranceiver connected");
                }else{
                    Serial.printf("WARNING: LoRa tranceiver not found!");
                }
                Serial.printf("\r\n");
                Serial.printf("Local address of your node is ");
                LL2.printAddress(Layer1.localAddress());
                Serial.printf("\r\n");
                /*
                if(fileSystem.sdInitialized()){
                    Serial.printf("SD card connected! Using SD as default file system");
                }else{
                    Serial.printf("WARNING: SD card not found! Using SPIFFs as default file system");
                }
                */
                Serial.printf("\r\n");
                _sessionConnected = 1;
            }
            _lastConnection == Layer1.getTime();
        }
    }else{
        if(Serial.available() > 0) {
            char incoming = Serial.read();
            Serial.printf("%c", incoming);
            if(incoming == 0x7f){ // backspace or delete
                if(_inputLength > 0){
                    _input[_inputLength] = 0; 
                    _inputLength--;
                    Serial.printf("\b \b");
                }
            }
            else if(incoming == '\r'){ // enter key
                Serial.printf("\n");
                parse(_input, _inputLength);
                memset(_input, 0, MAX_INPUT_LENGTH);
                _inputLength = 0;
                Serial.printf("/# ");
            }else{
                _input[_inputLength] = incoming; 
                _inputLength++;
            }
        }
    }
    return _sessionConnected;
}

int consoleClass::init(){
    _lastConnection == Layer1.getTime();
    return 0;
}

consoleClass console;

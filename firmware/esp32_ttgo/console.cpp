#include <Layer1.h>
#include <LoRaLayer2.h>
#include <console.h>
extern "C" char _binary_data_txt_start;
extern "C" char _binary_data_txt_end;

consoleClass::consoleClass() :

    _sessionConnected(0),
    _lastConnection(),
    _input(),
    _inputLength()
    {}

void consoleClass::parse(char input[MAX_INPUT_LENGTH], int length){
    
    char cmd[10][MAX_INPUT_LENGTH]; 
    int init_size = strlen(input);
    char delim[] = " ";
    char *ptr = strtok(input, delim);
    while(ptr != NULL){
        Serial.printf("'%s'\r\n", ptr);
	ptr = strtok(NULL, delim);
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
                Serial.printf("v0.1.0\r\n");
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
            if(incoming == 0x7f){
                if(_inputLength > 0){
                    _input[_inputLength] = 0; 
                    _inputLength--;
                    Serial.printf("\b \b");
                }
            }
            else if(incoming == '\r'){
                Serial.printf("\n");
                Serial.printf("echo: ");
                for(int i=0 ; i < _inputLength ; i++){
                    Serial.printf("%c", _input[i]);
                }
                Serial.printf("\r\n");
                parse(_input, _inputLength);
                memset(_input, 0, MAX_INPUT_LENGTH);
                _inputLength = 0;
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


-include $(dir $(SKETCH))settings.mk

SKETCH = ./firmware/main.ino
ESP_ROOT = ./arduino-esp32
FS_DIR = ./web/static
LIBS = ./libs/AsyncTCP \
	./libs/ESPAsyncWebServer \
	./libs/arduino-LoRa \
	./libs/LoRaLayer2 \
	$(ESP_LIBS)/WiFi \
	$(ESP_LIBS)/SPI \
	$(ESP_LIBS)/SD \
	$(ESP_LIBS)/ESPmDNS \
	$(ESP_LIBS)/FS \
	$(ESP_LIBS)/SPIFFS

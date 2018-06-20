#====================================================================================
# makeESPArduino
#
# A makefile for ESP8286 and ESP32 Arduino projects.
# Edit the contents of this file to suit your project
# or just include it and override the applicable macros.
#
# License: LGPL 2.1
# General and full license information is available at:
#    https://github.com/plerup/makeEspArduino
#
# Copyright (c) 2016-2017 Peter Lerup. All rights reserved.
#
#====================================================================================

#====================================================================================
# Project specfic values
#====================================================================================

# Include possible project makefile. This can be used to override the defaults below
-include $(firstword $(PROJ_CONF) $(dir $(SKETCH))config.mk)

#=== Default values not available in the Arduino configuration files

CHIP ?= esp8266

# Set chip specific default board unless specified
BOARD ?= $(if $(filter $(CHIP), esp32),esp32,generic)

# Serial flashing parameters
UPLOAD_PORT ?= $(shell ls -1tr /dev/ttyUSB* 2>/dev/null | tail -1)
UPLOAD_PORT := $(if $(UPLOAD_PORT),$(UPLOAD_PORT),/dev/ttyS0)
UPLOAD_VERB ?= -v

# OTA parameters
ESP_ADDR ?= ESP_123456
ESP_PORT ?= 8266
ESP_PWD ?= 123

# HTTP update parameters
HTTP_ADDR ?= ESP_123456
HTTP_URI ?= /update
HTTP_PWD ?= user
HTTP_USR ?= password

# Output directory
BUILD_ROOT ?= /tmp/mkESP
BUILD_DIR ?= $(BUILD_ROOT)/$(MAIN_NAME)_$(BOARD)

# File system source directory
FS_DIR ?= $(dir $(SKETCH))data
FS_REST_DIR ?= $(BUILD_DIR)/file_system

# Bootloader
BOOT_LOADER ?= $(ESP_ROOT)/bootloaders/eboot/eboot.elf

#====================================================================================
# Standard build logic and values
#====================================================================================

START_TIME := $(shell perl -e "print time();")
OS ?= $(shell uname -s)

# Utility functions
git_description = $(shell git -C  $(1) describe --tags --always --dirty 2>/dev/null || echo Unknown)
time_string = $(shell date +$(1))
ifeq ($(OS), Darwin)
  find_files = $(shell find -E $2 -regex ".*\.($1)")
else
  find_files = $(shell find $2 -regextype posix-egrep -regex ".*\.($1)")
endif

# ESP Arduino directories
ifndef ESP_ROOT
  # Location not defined, find and use possible version in the Arduino IDE installation
  ifeq ($(OS), Windows_NT)
    ARDUINO_ROOT = $(shell cygpath -m $(LOCALAPPDATA)/Arduino15)
  else ifeq ($(OS), Darwin)
    ARDUINO_ROOT = $(HOME)/Library/Arduino15
  else
    ARDUINO_ROOT = $(HOME)/.arduino15
  endif
  ARDUINO_ESP_ROOT = $(ARDUINO_ROOT)/packages/$(CHIP)
  ESP_ROOT := $(lastword $(wildcard $(ARDUINO_ESP_ROOT)/hardware/$(CHIP)/*))
  ifeq ($(ESP_ROOT),)
    $(error No installed version of $(CHIP) Arduino found)
  endif
  ARDUINO_LIBS = $(shell grep -o "sketchbook.path=.*" $(ARDUINO_ROOT)/preferences.txt 2>/dev/null | cut -f2- -d=)/libraries
  ESP_ARDUINO_VERSION := $(notdir $(ESP_ROOT))
  # Find used version of compiler and tools
  COMP_PATH := $(lastword $(wildcard $(ARDUINO_ESP_ROOT)/tools/xtensa-lx106-elf-gcc/*))
  ESPTOOL_PATH := $(lastword $(wildcard $(ARDUINO_ESP_ROOT)/tools/esptool/*))
  MKSPIFFS_PATH := $(lastword $(wildcard $(ARDUINO_ESP_ROOT)/tools/mkspiffs/*))
else
  # Location defined, assume it is a git clone
  ESP_ARDUINO_VERSION = $(call git_description,$(ESP_ROOT))
  MKSPIFFS_PATH := $(lastword $(wildcard $(ESP_ROOT)/tools/mkspiffs/*))
endif
ESP_LIBS = $(ESP_ROOT)/libraries
SDK_ROOT = $(ESP_ROOT)/tools/sdk
TOOLS_ROOT = $(ESP_ROOT)/tools

ifeq ($(wildcard $(ESP_ROOT)/cores/$(CHIP)),)
  $(error $(ESP_ROOT) is not a vaild directory for $(CHIP))
endif

ESPTOOL ?= $(shell which esptool.py 2>/dev/null || which esptool 2>/dev/null)
ifneq ($(ESPTOOL),)
  # esptool exists in path, overide defaults and use it for esp8266 flash operations
  ifeq ($(CHIP),esp8266)
    ESPTOOL_COM = $(ESPTOOL)
    UPLOAD_COM = $(ESPTOOL_PATTERN) -a soft_reset write_flash 0x00000 $(BUILD_DIR)/$(MAIN_NAME).bin
    FS_UPLOAD_COM = $(ESPTOOL_PATTERN) -a soft_reset write_flash $(SPIFFS_START) $(FS_IMAGE)
  endif
endif
ESPTOOL_PATTERN = echo Using: $(UPLOAD_PORT) @ $(UPLOAD_SPEED) && "$(ESPTOOL_COM)" --baud=$(UPLOAD_SPEED) --port $(UPLOAD_PORT) --chip $(CHIP)

ifeq ($(MAKECMDGOALS),help)
  DEMO=1
endif
ifdef DEMO
  SKETCH := $(if $(filter $(CHIP), esp32),$(ESP_LIBS)/WiFi/examples/WiFiScan/WiFiScan.ino,$(ESP_LIBS)/ESP8266WebServer/examples/HelloServer/HelloServer.ino)
endif
SKETCH ?= $(wildcard *.ino)
ifeq ($(SKETCH),)
  $(error No sketch specified or found. Use "DEMO=1" for testing)
endif
ifeq ($(wildcard $(SKETCH)),)
  $(error Sketch $(SKETCH) not found)
endif
SRC_GIT_VERSION := $(call git_description,$(dir $(SKETCH)))

# Main output definitions
MAIN_NAME := $(basename $(notdir $(SKETCH)))
MAIN_EXE = $(BUILD_DIR)/$(MAIN_NAME).bin
FS_IMAGE = $(BUILD_DIR)/FS.spiffs

ifeq ($(OS), Windows_NT)
  # Adjust some paths for cygwin
  BUILD_DIR := $(shell cygpath -m $(BUILD_DIR))
  SKETCH := $(shell cygpath -m $(SKETCH))
  ifdef ARDUINO_LIBS
    ARDUINO_LIBS := $(shell cygpath -m $(ARDUINO_LIBS))
  endif
endif

# Build file extensions
OBJ_EXT = .o
DEP_EXT = .d

# Auto generated makefile with Arduino definitions
ARDUINO_MK = $(BUILD_DIR)/arduino.mk

# Special tool definitions
OTA_TOOL ?= $(TOOLS_ROOT)/espota.py
HTTP_TOOL ?= curl

# Core source files
CORE_DIR = $(ESP_ROOT)/cores/$(CHIP)
CORE_SRC := $(call find_files,S|c|cpp,$(CORE_DIR))
CORE_OBJ := $(patsubst %,$(BUILD_DIR)/%$(OBJ_EXT),$(notdir $(CORE_SRC)))
CORE_LIB = $(BUILD_DIR)/arduino.ar

SKETCH_DIR = $(dir $(SKETCH))
# User defined compilation units and directories
ifeq ($(LIBS),)
  # Automatically find directories with header files used by the sketch
  LIBS := $(shell perl -e 'use File::Find;@d = split(" ", shift);while (<>) {$$f{"$$1"} = 1 if /^\s*\#include\s+[<"]([^>"]+)/;}find(sub {if ($$f{$$_}){print $$File::Find::dir," ";$$f{$$_}=0;}}, @d);' \
	                        "$(CUSTOM_LIBS) $(ESP_LIBS) $(ARDUINO_LIBS)" $(SKETCH) $(call find_files,S|c|cpp,$(SKETCH_DIR)))
endif

IGNORE_PATTERN := $(foreach dir,$(EXCLUDE_DIRS),$(dir)/%)
USER_INC := $(filter-out $(IGNORE_PATTERN),$(call find_files,h,$(SKETCH_DIR) $(dir $(LIBS))))
USER_SRC := $(SKETCH) $(filter-out $(IGNORE_PATTERN),$(call find_files,S|c|cpp,$(SKETCH_DIR) $(LIBS)))
# Object file suffix seems to be significant for the linker...
USER_OBJ := $(subst .ino,_.cpp,$(patsubst %,$(BUILD_DIR)/%$(OBJ_EXT),$(notdir $(USER_SRC))))
USER_DIRS := $(sort $(dir $(USER_SRC)))
USER_INC_DIRS := $(sort $(dir $(USER_INC)))

# Use first flash definition for the board as default
FLASH_DEF_MATCH = $(if $(filter $(CHIP), esp32),build\.flash_size=(\S+),menu\.FlashSize\.([^\.]+)=(.+))
FLASH_DEF ?= $(shell cat $(ESP_ROOT)/boards.txt | perl -e 'while (<>) {if (/^$(BOARD)\.$(FLASH_DEF_MATCH)/){ print "$$1"; exit;}}')
# Same method for LwIPVariant
LWIP_VARIANT ?= $(shell cat $(ESP_ROOT)/boards.txt | perl -e 'while (<>) {if (/^$(BOARD)\.menu\.LwIPVariant\.([^\.=]+)=/){ print "$$1"; exit;}}')

# Handle possible changed state i.e. make command line parameters or changed git versions
ifeq ($(OS), Darwin)
  CMD_LINE := $(shell ps $$PPID -o command | tail -1)
else
  CMD_LINE := $(shell tr "\0" " " </proc/$$PPID/cmdline)
endif
IGNORE_STATE ?= $(if $(filter $(MAKECMDGOALS), help clean dump_flash restore_flash list_boards),1,)
ifeq ($(IGNORE_STATE),)
  STATE_LOG := $(BUILD_DIR)/state.txt
  STATE_INF := $(strip $(foreach par,$(CMD_LINE),$(if $(findstring =,$(par)),$(par),))) \
               $(SRC_GIT_VERSION) $(ESP_ARDUINO_VERSION)
  PREV_STATE_INF := $(if $(wildcard $(STATE_LOG)),$(shell cat $(STATE_LOG)),$(STATE_INF))
  ifneq ($(PREV_STATE_INF),$(STATE_INF))
    $(info * Build state has changed, doing a full rebuild *)
    $(shell rm -rf $(BUILD_DIR)/*)
  endif
  STATE_SAVE := $(shell mkdir -p $(BUILD_DIR) ; echo '$(STATE_INF)' >$(STATE_LOG))
endif

# The actual build commands are to be extracted from the Arduino description files
ARDUINO_DESC := $(shell find $(ESP_ROOT) -maxdepth 1 -name "*.txt" | sort)
$(ARDUINO_MK): $(ARDUINO_DESC) $(MAKEFILE_LIST) | $(BUILD_DIR)
	perl -e "$$PARSE_ARDUINO" $(BOARD) '$(FLASH_DEF)' '$(OS)' '$(LWIP_VARIANT)' $(ARDUINO_EXTRA_DESC) $(ARDUINO_DESC) >$(ARDUINO_MK)

-include $(ARDUINO_MK)

# Compilation directories and path
INCLUDE_DIRS += $(CORE_DIR) $(ESP_ROOT)/variants/$(INCLUDE_VARIANT) $(BUILD_DIR)
C_INCLUDES := $(foreach dir,$(INCLUDE_DIRS) $(USER_INC_DIRS),-I$(dir))
VPATH += $(shell find $(CORE_DIR) -type d) $(USER_DIRS)

# Automatically generated build information data
# Makes the build date and git descriptions at the actual build event available as string constants in the program
BUILD_INFO_H = $(BUILD_DIR)/buildinfo.h
BUILD_INFO_CPP = $(BUILD_DIR)/buildinfo.c++
BUILD_INFO_OBJ = $(BUILD_INFO_CPP)$(OBJ_EXT)

$(BUILD_INFO_H): | $(BUILD_DIR)
	echo "typedef struct { const char *date, *time, *src_version, *env_version;} _tBuildInfo; extern _tBuildInfo _BuildInfo;" >$@

# Build rules for the different source file types
$(BUILD_DIR)/%.cpp$(OBJ_EXT): %.cpp $(BUILD_INFO_H) $(ARDUINO_MK)
	echo  $(<F)
	$(CPP_COM) $(CPP_EXTRA) $< -o $@

$(BUILD_DIR)/%_.cpp$(OBJ_EXT): %.ino $(BUILD_INFO_H) $(ARDUINO_MK)
	echo  $(<F)
	$(CPP_COM) $(CPP_EXTRA) -x c++ -include $(CORE_DIR)/Arduino.h $< -o $@

$(BUILD_DIR)/%.c$(OBJ_EXT): %.c $(ARDUINO_MK)
	echo  $(<F)
	$(C_COM) $(C_EXTRA) $< -o $@

$(BUILD_DIR)/%.S$(OBJ_EXT): %.S $(ARDUINO_MK)
	echo  $(<F)
	$(S_COM) $(S_EXTRA) $< -o $@

$(CORE_LIB): $(CORE_OBJ)
	echo  Creating core archive
	rm -f $@
	$(AR_COM) $^

BUILD_DATE = $(call time_string,"%Y-%m-%d")
BUILD_TIME = $(call time_string,"%H:%M:%S")

$(MAIN_EXE): $(CORE_LIB) $(USER_OBJ)
	echo Linking $(MAIN_EXE)
	$(LINK_PREBUILD)
	echo "  Versions: $(SRC_GIT_VERSION), $(ESP_ARDUINO_VERSION)"
	echo 	'#include <buildinfo.h>' >$(BUILD_INFO_CPP)
	echo '_tBuildInfo _BuildInfo = {"$(BUILD_DATE)","$(BUILD_TIME)","$(SRC_GIT_VERSION)","$(ESP_ARDUINO_VERSION)"};' >>$(BUILD_INFO_CPP)
	$(CPP_COM) $(BUILD_INFO_CPP) -o $(BUILD_INFO_OBJ)
	$(LD_COM) $(LD_EXTRA)
	$(GEN_PART_COM)
	$(ELF2BIN_COM)
	$(SIZE_COM) | perl -e "$$MEM_USAGE" "$(MEM_FLASH)" "$(MEM_RAM)"
ifneq ($(LWIP_INFO),)
	printf "LwIPVariant: $(LWIP_INFO)\n"
endif
ifneq ($(FLASH_INFO),)
	printf "Flash size: $(FLASH_INFO)\n\n"
endif
	perl -e 'print "Build complete. Elapsed time: ", time()-$(START_TIME),  " seconds\n\n"'

upload flash: all
	$(UPLOAD_COM)

ota: all
	$(OTA_TOOL) -r -i $(ESP_ADDR) -p $(ESP_PORT) -a $(ESP_PWD) -f $(MAIN_EXE)

http: all
	$(HTTP_TOOL) --verbose -F image=@$(MAIN_EXE) --user $(HTTP_USR):$(HTTP_PWD) http://$(HTTP_ADDR)$(HTTP_URI)
	echo "\n"

$(FS_IMAGE): $(ARDUINO_MK) $(wildcard $(FS_DIR)/*)
	echo Generating filesystem image: $(FS_IMAGE)
	$(MKSPIFFS_COM)

fs: $(FS_IMAGE)

upload_fs flash_fs: $(FS_IMAGE)
	$(FS_UPLOAD_COM)

ota_fs: $(FS_IMAGE)
	$(OTA_TOOL) -r -i $(ESP_ADDR) -p $(ESP_PORT) -a $(ESP_PWD) -s -f $(FS_IMAGE)


FLASH_FILE ?= $(BUILD_DIR)/esp_flash.bin
dump_flash:
	echo Dumping flash memory to file: $(FLASH_FILE)
	$(ESPTOOL_PATTERN) read_flash 0 $(shell perl -e 'shift =~ /(\d+)([MK])/ || die "Invalid memory size\n";$$mem_size=$$1*1024;$$mem_size*=1024 if $$2 eq "M";print $$mem_size;' $(FLASH_DEF)) $(FLASH_FILE)

dump_fs:
	echo Dumping flash file system to directory: $(FS_REST_DIR)
	-$(ESPTOOL_PATTERN) read_flash $(SPIFFS_START) $(SPIFFS_SIZE) $(FS_IMAGE)
	mkdir -p $(FS_REST_DIR)
	echo
	echo == Files ==
	$(RESTSPIFFS_COM)

restore_flash:
	echo Restoring flash memory from file: $(FLASH_FILE)
	$(ESPTOOL_PATTERN) -a soft_reset write_flash 0 $(FLASH_FILE)

erase_flash:
	$(ESPTOOL_PATTERN) erase_flash

clean:
	echo Removing all build files
	rm  -rf $(BUILD_DIR)/*

list_boards:
	echo === Available boards ===
	cat $(ESP_ROOT)/boards.txt | perl -e 'while (<>) { if (/^(\w+)\.name=(.+)/){ print sprintf("%-20s %s\n", $$1,$$2);} }'

list_lib:
	echo === User specific libraries ===
	perl -e 'foreach (@ARGV) {print "$$_\n"}' "* Include directories:" $(USER_INC_DIRS)  "* Library source files:" $(USER_SRC)

list_flash_defs:
	echo === Memory configurations for board: $(BOARD) ===
	cat $(ESP_ROOT)/boards.txt | perl -e 'while (<>) { if (/^$(BOARD)\.$(FLASH_DEF_MATCH)/){ print sprintf("%-10s %s\n", $$1,$$2);} }'

help:
	echo
	echo "Generic makefile for building Arduino esp8266 and esp32 projects"
	echo "This file can either be used directly or included from another makefile"
	echo ""
	echo "The following targets are available:"
	echo "  all                  (default) Build the project application"
	echo "  clean                Remove all intermediate build files"
	echo "  flash                Build and and flash the project application"
	echo "  flash_fs             Build and and flash file system (when applicable)"
	echo "  ota                  Build and and flash via OTA"
	echo "                         Params: ESP_ADDR, ESP_PORT and ESP_PWD"
	echo "  ota_fs               Build and and flash file system via OTA"
	echo "  http                 Build and and flash via http (curl)"
	echo "                         Params: HTTP_ADDR, HTTP_URI, HTTP_PWD and HTTP_USR"
	echo "  dump_flash           Dump the whole board flash memory to a file"
	echo "  restore_flash        Restore flash memory from a previously dumped file"
	echo "  dump_fs              Extract all files from the flash file system"
	echo "                         Params: FS_DUMP_DIR"
	echo "  erase_flash          Erase the whole flash"
	echo "  list_lib             Show a list of used library files and include paths"
	echo "Configurable parameters:"
	echo "  SKETCH               Main source file"
	echo "                         If not specified the first sketch in current"
	echo "                         directory will be used."
	echo "  LIBS                 Includes in the sketch file of libraries from within"
	echo "                         the ESP Arduino directories are automatically"
	echo "                         detected. If this is not enough, define this"
	echo "                         variable with all libraries or directories needed."
	echo "  CHIP                 Set to esp8266 or esp32. Default: '$(CHIP)'"
	echo "  BOARD                Name of the target board. Default: '$(BOARD)'"
	echo "                         Use 'list_boards' to get list of available ones"
	echo "  FLASH_DEF            Flash partitioning info. Default '$(FLASH_DEF)'"
	echo "                         Use 'list_flash_defs' to get list of available ones"
	echo "  BUILD_DIR            Directory for intermediate build files."
	echo "                         Default '$(BUILD_DIR)'"
	echo "  BUILD_EXTRA_FLAGS    Additional parameters for the compilation commands"
	echo "  FS_DIR               File system root directory"
	echo "  UPLOAD_PORT          Serial flashing port name. Default: '$(UPLOAD_PORT)'"
	echo "  UPLOAD_SPEED         Serial flashing baud rate. Default: '$(UPLOAD_SPEED)'"
	echo "  FLASH_FILE           File name for dump and restore flash operations"
	echo "                          Default: '$(FLASH_FILE)'"
	echo "  LWIP_VARIANT         Use specified variant of the lwip library when applicable"
	echo "                         Default: $(LWIP_VARIANT) ($(LWIP_INFO))"
	echo "  VERBOSE              Set to 1 to get full printout of the build"
	echo "  SINGLE_THREAD        Use only one build thread"
	echo

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: all
all: $(BUILD_DIR) $(ARDUINO_MK) $(BUILD_INFO_H) prebuild $(MAIN_EXE)

prebuild:
ifdef USE_PREBUILD
	$(CORE_PREBUILD)
endif
	$(SKETCH_PREBUILD)

# Include all available dependencies
-include $(wildcard $(BUILD_DIR)/*$(DEP_EXT))

.DEFAULT_GOAL = all

ifndef SINGLE_THREAD
  # Use multithreaded builds by default
  MAKEFLAGS += -j
endif

ifndef VERBOSE
  # Set silent mode as default
  MAKEFLAGS += --silent
endif

# Inline Perl scripts

# Parse Arduino definitions and build commands from the descriptions
define PARSE_ARDUINO
my $$board = shift;
my $$flashSize = shift;
my $$os = (shift =~ /Windows_NT/) ? "windows" : "linux";
my $$lwipvariant = shift;
my %v;

sub def_var {
   my ($$name, $$var) = @_;
   print "$$var ?= $$v{$$name}\n";
   $$v{$$name} = "\$$($$var)";
}

$$v{'runtime.platform.path'} = '$$(ESP_ROOT)';
$$v{'includes'} = '$$(C_INCLUDES)';
$$v{'runtime.ide.version'} = '10605';
$$v{'build.arch'} = uc('$(CHIP)');
$$v{'build.project_name'} = '$$(MAIN_NAME)';
$$v{'build.path'} = '$$(BUILD_DIR)';
$$v{'object_files'} = '$$^ $$(BUILD_INFO_OBJ)';

foreach my $$fn (@ARGV) {
   open($$f, $$fn) || die "Failed to open: $$fn\n";
   while (<$$f>) {
      s/\s+$$//;
      next unless /^(\w[\w\-\.]+)=(.*)/;
      my ($$key, $$val) =($$1, $$2);
      $$board_defined = 1 if $$key eq "$$board.name";
      $$key =~ s/$$board\.menu\.FlashSize\.$$flashSize\.//;
      $$key =~ s/$$board\.menu\.CpuFrequency\.[^\.]+\.//;
      $$key =~ s/$$board\.menu\.FlashFreq\.[^\.]+\.//;
      $$key =~ s/$$board\.menu\.UploadSpeed\.[^\.]+\.//;
      $$key =~ s/$$board\.menu\.ResetMethod\.[^\.]+\.//;
      $$key =~ s/$$board\.menu\.FlashMode\.[^\.]+\.//;
			$$key =~ s/$$board\.menu\.LwIPVariant\.$$lwipvariant\.//;
      $$key =~ s/^$$board\.//;
      $$v{$$key} ||= $$val;
      $$v{$$1} = $$v{$$key} if $$key =~ /(.+)\.$$os$$/;
   }
   close($$f);
}
$$v{'runtime.tools.xtensa-lx106-elf-gcc.path'} ||= '$$(COMP_PATH)';
$$v{'runtime.tools.esptool.path'} ||= '$$(ESPTOOL_PATH)';

die "* Unkown board $$board\n" unless $$board_defined;
print "# Board definitions\n";
def_var('build.f_cpu', 'F_CPU');
def_var('build.flash_mode', 'FLASH_MODE');
def_var('build.flash_freq', 'FLASH_SPEED');
def_var('upload.resetmethod', 'UPLOAD_RESET');
def_var('upload.speed', 'UPLOAD_SPEED');
def_var('compiler.warning_flags', 'COMP_WARNINGS');
$$v{'upload.verbose'} = '$$(UPLOAD_VERB)';
$$v{'serial.port'} = '$$(UPLOAD_PORT)';
$$v{'recipe.objcopy.hex.pattern'} =~ s/[^"]+\/bootloaders\/eboot\/eboot.elf/\$$(BOOT_LOADER)/;
$$v{'recipe.hooks.linking.prelink.1.pattern'} =~ s/\{build.vtable_flags\}/\$$(VTABLE_FLAGS)/;
$$v{'tools.esptool.upload.pattern'} =~ s/\{(cmd|path)\}/\{tools.esptool.$$1\}/g;
$$v{'compiler.cpreprocessor.flags'} .= " \$$(C_PRE_PROC_FLAGS)";
$$v{'build.extra_flags'} .= " \$$(BUILD_EXTRA_FLAGS)";

foreach my $$key (sort keys %v) {
   while ($$v{$$key} =~/\{/) {
      $$v{$$key} =~ s/\{([\w\-\.]+)\}/$$v{$$1}/;
      $$v{$$key} =~ s/""//;
   }
   $$v{$$key} =~ s/ -o\s+$$//;
   $$v{$$key} =~ s/(-D\w+=)"([^"]+)"/$$1\\"$$2\\"/g;
}

print "INCLUDE_VARIANT = $$v{'build.variant'}\n";
print "# Commands\n";
print "C_COM=$$v{'recipe.c.o.pattern'}\n";
print "CPP_COM=$$v{'recipe.cpp.o.pattern'}\n";
print "S_COM=$$v{'recipe.S.o.pattern'}\n";
print "AR_COM=$$v{'recipe.ar.pattern'}\n";
print "LD_COM=$$v{'recipe.c.combine.pattern'}\n";
print "PART_FILE?=$$1\n" if $$v{'recipe.objcopy.eep.pattern'} =~ /\"([^\"]+\.csv)\"/;
$$v{'recipe.objcopy.eep.pattern'} =~ s/\"([^\"]+\.csv)\"/\$$(PART_FILE)/;
print "GEN_PART_COM=$$v{'recipe.objcopy.eep.pattern'}\n";
print "ELF2BIN_COM=$$v{'recipe.objcopy.hex.pattern'}\n";
print "SIZE_COM=$$v{'recipe.size.pattern'}\n";
print "ESPTOOL_COM?=$$v{'tools.esptool.path'}/$$v{'tools.esptool.cmd'}\n";
print "UPLOAD_COM?=$$v{'tools.esptool.upload.pattern'}\n";

if ($$v{'build.spiffs_start'}) {
  print "SPIFFS_START?=$$v{'build.spiffs_start'}\n";
	my $$spiffs_size = sprintf("0x%X", hex($$v{'build.spiffs_end'})-hex($$v{'build.spiffs_start'}));
  print "SPIFFS_SIZE?=$$spiffs_size\n";
} elsif ($$v{'build.partitions'}) {
  print "COMMA=,\n";
  print "SPIFFS_SPEC:=\$$(subst \$$(COMMA), ,\$$(shell grep spiffs \$$(PART_FILE)))\n";
  print "SPIFFS_START:=\$$(word 4,\$$(SPIFFS_SPEC))\n";
  print "SPIFFS_SIZE:=\$$(word 5,\$$(SPIFFS_SPEC))\n";
}
$$v{'build.spiffs_blocksize'} ||= "4096";
print "SPIFFS_BLOCK_SIZE?=$$v{'build.spiffs_blocksize'}\n";
print "MKSPIFFS_COM?=\"\$$(MKSPIFFS_PATH)\" -b \$$(SPIFFS_BLOCK_SIZE) -s \$$(SPIFFS_SIZE) -c \$$(FS_DIR) \$$(FS_IMAGE)\n";
print "RESTSPIFFS_COM?=\"\$$(MKSPIFFS_PATH)\" -b \$$(SPIFFS_BLOCK_SIZE) -s \$$(SPIFFS_SIZE) -u \$$(FS_REST_DIR) \$$(FS_IMAGE)\n";

my $$fs_upload_com = $$v{'tools.esptool.upload.pattern'};
$$fs_upload_com =~ s/(.+ -ca) .+/$$1 \$$(SPIFFS_START) -cf \$$(FS_IMAGE)/;
$$fs_upload_com =~ s/(.+ --flash_size detect) .+/$$1 \$$(SPIFFS_START) \$$(FS_IMAGE)/;
print "FS_UPLOAD_COM?=$$fs_upload_com\n";
my $$val = $$v{'recipe.hooks.core.prebuild.1.pattern'};
$$val =~ s/bash -c "(.+)"/$$1/;
$$val =~ s/(#define .+0x)(\`)/"\\$$1\"$$2/;
$$val =~ s/(\\)//;
print "CORE_PREBUILD=$$val\n";
print "SKETCH_PREBUILD=$$v{'recipe.hooks.sketch.prebuild.1.pattern'}\n";
print "VTABLE_FLAGS?=$$v{'build.vtable_flags'}\n";
print "LINK_PREBUILD=$$v{'recipe.hooks.linking.prelink.1.pattern'}\n";
print "MEM_FLASH=$$v{'recipe.size.regex'}\n";
print "MEM_RAM=$$v{'recipe.size.regex.data'}\n";
print "FLASH_INFO=$$v{'menu.FlashSize.' . $$flashSize}\n";
print "LWIP_INFO=$$v{'menu.LwIPVariant.' . $$lwipvariant}\n";
endef
export PARSE_ARDUINO

# Convert memory information
define MEM_USAGE
$$fp = shift;
$$rp = shift;
while (<>) {
  $$r += $$1 if /$$rp/;
  $$f += $$1 if /$$fp/;
}
print "\nMemory usage\n";
print sprintf("  %-6s %6d bytes\n" x 2 ."\n", "Ram:", $$r, "Flash:", $$f);
endef
export MEM_USAGE

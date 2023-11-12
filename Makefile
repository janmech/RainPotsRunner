CC = g++
# CC = gcc
CFLAGGS = -g -pthread -lstdc++ -ljsoncpp -lcurl -rdynamic
CFLAGGS_OPTIMIZED = -pthread -lstdc++ -ljsoncpp -lcurl -rdynamic

#€-lwiringPi
#-Werror

BUILD_PATH= build/

SOURCE_PATH= src src/*  src/*/* src/*/*  tinyosc
TARGET = main
OUTNAME = rainpots
# TARGET = serial-test
# TARGET = json_test
# TARGET = stacktraceteest
# OUTNAME = $(TARGET)

SRCS = $(wildcard $(foreach fd, $(SOURCE_PATH), $(fd)/*.c))
SRCS_CPP = $(wildcard $(foreach fd, $(SOURCE_PATH), $(fd)/*.cpp))

# $(NULL) is empty string
NULL  :=
# $(SPACE) is one space
SPACE := $(NULL) $(NULL)
# $(\n) is new line
ENDLN = "\n"

SRCS_FORMATTED = $(subst $(SPACE),$(ENDLN),$(SRCS))
SRCS_CPP_FORMATTED = $(subst $(SPACE),$(ENDLN),$(SRCS_CPP))


.PHONY: clean

# $@ = target name
# $^ =  dependency list

all: clean $(TARGET)

clean:
	rm -rf $(BUILD_PATH)*.o 
	rm -rf $(BUILD_PATH)$(TARGET)
	rm -rf $(BUILD_PATH)$(OUTNAME)

$(TARGET): $(SRCS) $(SRCS_CPP)
	@echo "\n***************************\n"
	@echo $(SRCS_FORMATTED)
	@echo $(SRCS_CPP_FORMATTED)
	@echo "\n***************************\n"
# $(CC) $^ $(TARGET).cpp $(CFLAGGS) -o $(BUILD_PATH)$@ 
# $(CC) $^ $(TARGET).cpp $(CFLAGGS) -o $(BUILD_PATH)$(OUTNAME)
	$(CC) $^ $(TARGET).cpp $(CFLAGGS_OPTIMIZED) -o $(BUILD_PATH)$(OUTNAME)
	
	@echo "\n"

install:
	sudo cp ./$(BUILD_PATH)rainpots /usr/bin




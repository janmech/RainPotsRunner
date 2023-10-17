CC = g++
CFLAGGS = -g -pthread -lwiringPi -lstdc++ -ljsoncpp -lcurl

#-Werror

BUILD_PATH= build/
SOURCE_PATH= src src/*  src/*/* src/*/*  tinyosc

TARGET = main

# TARGET = json_test


SRCS = $(wildcard $(foreach fd, $(SOURCE_PATH), $(fd)/*.c))
SRCS_CPP = $(wildcard $(foreach fd, $(SOURCE_PATH), $(fd)/*.cpp))


.PHONY: clean

# $@ = target name
# $^ =  dependency list

all: clean $(TARGET)

clean:
	rm -rf $(BUILD_PATH)*.o 
	rm -rf $(TARGET)

$(TARGET): $(SRCS) $(SRCS_CPP)
	@echo "\n***************************\n"
	@echo $(SRCS)
	@echo $(SRCS_CPP)
	@echo "\n***************************\n"
	$(CC) $^ $(TARGET).cpp $(CFLAGGS) -o $(BUILD_PATH)$@ 




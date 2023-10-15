CC = gcc
CFLAGGS = -g -Wall -pthread -lwiringPi -lstdc++

BUILD_PATH= build/
SOURCE_PATH= src src/*  src/*/* src/*/*  tinyosc

# TARGET = hello

TARGET = hello


SRCS = $(wildcard $(foreach fd, $(SOURCE_PATH), $(fd)/*.c))

.PHONY: clean

# $@ = target name
# $^ =  dependency list

all: clean $(TARGET)

clean:
	rm -rf $(BUILD_PATH)*.o 
	rm -rf hello

$(TARGET): $(SRCS)
	echo "\n***************************\n"
	echo $(SRCS)
	echo "\n***************************\n"
	$(CC) $(CFLAGGS) $^ $(TARGET).cpp -o $(BUILD_PATH)$@ 


foo:
	echo $(SRCS)




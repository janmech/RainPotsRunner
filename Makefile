CC = gcc
CFLAGGS = -g -Wall -pthread -lwiringPi -lstdc++

BUILD_PATH= build/
SOURCE_PATH= src src/*  src/*/* src/*/*  tinyosc

# TARGET = hello

TARGET = hello


SRCS = $(wildcard $(foreach fd, $(SOURCE_PATH), $(fd)/*.c))
SRCS_CPP = $(wildcard $(foreach fd, $(SOURCE_PATH), $(fd)/*.cpp))

.PHONY: clean

# $@ = target name
# $^ =  dependency list

all: clean $(TARGET)

clean:
	rm -rf $(BUILD_PATH)*.o 
	rm -rf hello

$(TARGET): $(SRCS) $(SRCS_CPP)
	@echo "\n***************************\n"
	@echo $(SRCS)
	@echo $(SRCS_CPP)
	@echo "\n***************************\n"
	$(CC) $(CFLAGGS) $^ $(TARGET).cpp -o $(BUILD_PATH)$@ 


foo:
	echo $(SRCS)




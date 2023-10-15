CC = gcc
CFLAGGS = -g -Wall

BUILD_PATH= build/
SOURCE_PATH= src src/*  src/*/* 

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
	echo $(SRCS)
	$(CC) $(CFLAGGS) $^ $(TARGET).c -o $(BUILD_PATH)$@ 


foo:
	echo $(SRCS)




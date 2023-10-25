CC = g++
# CC = gcc
CFLAGGS = -g -pthread -lstdc++ -ljsoncpp -lcurl

#€-lwiringPi
#-Werror

BUILD_PATH= build/

SOURCE_PATH= src src/*  src/*/* src/*/*  tinyosc

TARGET = main
# TARGET = serial-test

# TARGET = json_test


SRCS = $(wildcard $(foreach fd, $(SOURCE_PATH), $(fd)/*.c))
SRCS_CPP = $(wildcard $(foreach fd, $(SOURCE_PATH), $(fd)/*.cpp))


.PHONY: clean

# $@ = target name
# $^ =  dependency list

all: clean $(TARGET)

clean:
	rm -rf $(BUILD_PATH)*.o 
	rm -rf $(BUILD_PATH)$(TARGET)
	rm -rf $(BUILD_PATH)rainpots

$(TARGET): $(SRCS) $(SRCS_CPP)
	@echo "\n***************************\n"
	@echo $(SRCS)
	@echo $(SRCS_CPP)
	@echo "\n***************************\n"
	# $(CC) $^ $(TARGET).cpp $(CFLAGGS) -o $(BUILD_PATH)$@ 
	$(CC) $^ $(TARGET).cpp $(CFLAGGS) -o $(BUILD_PATH)rainpots
	@echo "\n"

install:
	sudo cp ./$(BUILD_PATH)rainpots /usr/bin




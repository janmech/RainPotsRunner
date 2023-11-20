CC = g++

# USE THIS FOR DEV
# CFLAGGS = -g -Wall -pthread -lstdc++ -ljsoncpp -lcurl -rdynamic

# USE THIS FOR PRODUCTION
CFLAGGS = -Wall -pthread -lstdc++ -ljsoncpp -lcurl -rdynamic  -O1

BUILD_PATH= build/

SOURCE_PATH= src src/*  src/*/* src/*/*  tinyosc
TARGET = main
OUTNAME = rainpots
MAIN_FILE = main

SRCS = $(wildcard $(foreach fd, $(SOURCE_PATH), $(fd)/*.c))
SRCS_CPP = $(wildcard $(foreach fd, $(SOURCE_PATH), $(fd)/*.cpp))

OBJ_DEPENDENCIES =  TSQueue.o DataHandler.o OscListener.o OscSender.o SerialConnector.o SerialSender.o tinyosc.o

.PHONY: clean rptest

# $@ = target name
# $^ =  dependency list

all: rainpots

clean:
	rm -rf $(BUILD_PATH)*

# TheadClass.o: src/TheadClass/TheadClass.cpp src/TheadClass/TheadClass.hpp
# 	$(CC) $(CFLAGGS) -c $(filter %.cpp,$^) -o $(addprefix $(BUILD_PATH), $@)

TSQueue.o: src/data/TSQueue.cpp src/data/TSQueue.hpp 
	$(CC) $(CFLAGGS) -c $(filter %.cpp,$^) -o $(addprefix $(BUILD_PATH), $@)

DataHandler.o: src/data/DataHandler.cpp src/data/DataHandler.hpp
	$(CC) $(CFLAGGS) -c $(filter %.cpp,$^) -o $(addprefix $(BUILD_PATH), $@)
	
OscListener.o: src/osc/OscListener.cpp src/osc/OscListener.hpp
	$(CC) $(CFLAGGS) -c $(filter %.cpp,$^) -o $(addprefix $(BUILD_PATH), $@)

OscSender.o: src/osc/OscSender.cpp src/osc/OscSender.hpp
	$(CC) $(CFLAGGS) -c $(filter %.cpp,$^) -o $(addprefix $(BUILD_PATH), $@)

SerialConnector.o: src/serial/SerialConnector.cpp src/serial/SerialConnector.hpp
	$(CC) $(CFLAGGS) -c $(filter %.cpp,$^) -o $(addprefix $(BUILD_PATH), $@)

SerialSender.o: src/serial/SerialSender.cpp src/serial/SerialSender.hpp
	$(CC) $(CFLAGGS) -c $(filter %.cpp,$^) -o $(addprefix $(BUILD_PATH), $@)

tinyosc.o: src/osc/tinyosc/tinyosc.c src/osc/tinyosc/tinyosc.h
	$(CC) $(CFLAGGS) -c $(filter %.c,$^) -o $(addprefix $(BUILD_PATH), $@)


rainpots: $(OBJ_DEPENDENCIES)
	$(CC) $(addprefix $(BUILD_PATH), $^) main.cpp $(CFLAGGS) -o $(BUILD_PATH)$@

install:
	@if [ ! -d "/home/pi/Documents/rainpots" ]; then mkdir /home/pi/Documents/rainpots; fi
	@if [ ! -d "/home/pi/Documents/rainpots/log" ];then mkdir /home/pi/Documents/rainpots/log; fi
	sudo cp ./$(BUILD_PATH)rainpots /usr/bin



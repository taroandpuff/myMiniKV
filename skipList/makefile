CC = g++  
HEADS = ./include/
OBJ = ./*.cpp
TARGET = ./bin/skipdb

skipdb : $(OBJ)
	$(CC) $(OBJ) -I $(HEADS) -o $(TARGET) --std=c++11 -lpthread 
clean: 
	rm -f ./bin/*

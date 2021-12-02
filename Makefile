all: dosdetector

SRC_PATH = src/
OBJ_PATH = obj/

dosdetector: $(OBJ_PATH)/message.o $(OBJ_PATH)/timer.o $(OBJ_PATH)/sniffer.o $(OBJ_PATH)/statistics.o $(OBJ_PATH)/cgui.o $(OBJ_PATH)/dosdetector.o
	g++ -o dosdetector $(OBJ_PATH)/*.o -pthread -lpcap -lncurses

$(OBJ_PATH)/message.o: $(SRC_PATH)/message.cpp
	g++ -c $(SRC_PATH)/message.cpp -o $(OBJ_PATH)/message.o

$(OBJ_PATH)/timer.o: $(SRC_PATH)/timer.cpp
	g++ -c $(SRC_PATH)/timer.cpp -o $(OBJ_PATH)/timer.o

$(OBJ_PATH)/sniffer.o: $(SRC_PATH)/sniffer.cpp
	g++ -c $(SRC_PATH)/sniffer.cpp -o $(OBJ_PATH)/sniffer.o

$(OBJ_PATH)/statistics.o: $(SRC_PATH)/statistics.cpp
	g++ -c $(SRC_PATH)/statistics.cpp -o $(OBJ_PATH)/statistics.o

$(OBJ_PATH)/cgui.o: $(SRC_PATH)/cgui.cpp
	g++ -c $(SRC_PATH)/cgui.cpp -o $(OBJ_PATH)/cgui.o

$(OBJ_PATH)/dosdetector.o: $(SRC_PATH)/dosdetector.cpp
	g++ -c $(SRC_PATH)/dosdetector.cpp -o $(OBJ_PATH)/dosdetector.o

clean:
	rm dosdetector obj/*
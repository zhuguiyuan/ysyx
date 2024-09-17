CFLAGS = -Wall -g

TARGET = ex18

all: $(TARGET)

check: $(TARGET)
	valgrind ./$(TARGET) 4 1 7 3 2 0 8

clean:
	rm -f $(TARGET)

.PHONY: all clean

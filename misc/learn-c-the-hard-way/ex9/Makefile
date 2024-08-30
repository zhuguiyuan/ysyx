CFLAGS = -Wall -g

TARGET = ex9

all: $(TARGET)

check: $(TARGET)
	valgrind ./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean

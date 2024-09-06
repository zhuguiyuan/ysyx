CFLAGS = -Wall -g

TARGET = ex11

all: $(TARGET)

check: $(TARGET)
	valgrind ./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean

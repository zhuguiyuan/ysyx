CFLAGS = -Wall -g

TARGET = ex15

all: $(TARGET)

check: $(TARGET)
	valgrind ./$(TARGET) $(PARGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean

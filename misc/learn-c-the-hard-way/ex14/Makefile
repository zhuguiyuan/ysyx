CFLAGS = -Wall -g

TARGET = ex14

all: $(TARGET)

check: $(TARGET)
	valgrind ./$(TARGET) $(PARGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean

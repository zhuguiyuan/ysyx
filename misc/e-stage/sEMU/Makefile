CPPFLAGS += $(shell pkg-config --cflags glib-2.0)
LDLIBS   += $(shell pkg-config --libs glib-2.0)

all: main

dev: CFLAGS += -g
dev: all


clean:
	rm -f main

ide:
	bear -- $(MAKE)

.PHONY: all clean ide

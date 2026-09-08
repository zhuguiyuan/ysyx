CPPFLAGS += $(shell pkg-config --cflags glib-2.0)
LDLIBS   += $(shell pkg-config --libs glib-2.0)
CFLAGS   += -MMD -MP

OBJS = main.o inst.o state.o
all: main
main: $(OBJS)

DEPS = $(OBJS:.o=.d)
-include $(DEPS)

dev: CFLAGS += -g
dev: all

clean:
	rm -f main *.o *.d

ide:
	bear -- $(MAKE)

.PHONY: all clean ide

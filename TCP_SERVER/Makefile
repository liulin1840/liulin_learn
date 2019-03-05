CC=gcc
LD=gcc
CFLAGS= -Wall
LIBS= -lpthread -lm -ldl -llua
OBJS=x-core.o x-http.o x-poll.o x-memory.o x-session.o x-timer.o x-lua.o
CFLAGS +=  -I./include
VPATH=./src:./include

.PHONY:clean obj_clean
all:clean x-server cjson  socket obj_clean

cjson:
	make -C modules/lua-cjson-2.1.0
socket:
	make -C modules/luasocket-2.0.2
	make -C modules/luasocket-2.0.2 install
x-server:$(OBJS)
	@echo "LD -o $@ $(OBJS) $(CFLAGS) $(LIBS)"
	@$(LD) $(CFLAGS) -o $@ $^ $(LIBS)  

%.o:%.c
	@echo "CC -c $<"
	@$(CC) $(CFLAGS) -c $<
	
all_clean:
	rm -f x-server
	rm -f *.o .*.depend
clean:
	rm -f x-server
	rm -f *.o .*.depend
	make -C modules/lua-cjson-2.1.0 clean
	make -C modules/luasocket-2.0.2
obj_clean:
	rm -f *.o .*.depend

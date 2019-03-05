CC=gcc
LD=gcc
CFLAGS= -Wall -g
LIBS= -lpthread -lm
OBJS=main.o thread.o timer.o db.o mem.o allocator.o worker.o public.o auto_data_save.o hash.o cJSON.o
LIBS += 
CFLAGS += -I./
VPATH=./

.PHONY:all 

all:clean analyzer

analyzer:$(OBJS)
	@echo "LD -o $@ $(OBJS) $(CFLAGS) $(LIBS)"
	@$(LD) $(CFLAGS) -o $@ $^ $(LIBS)  
	rm -f *.o *.depend

%.o:%.c
	@echo "CC -c $<"
	@$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f analyzer
	rm -f *.o .*.depend

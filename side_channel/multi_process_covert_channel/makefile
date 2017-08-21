include ../Rule.mak
CC=gcc
MAIN_SOURCE=sender receiver
RESULT_PATH=./result
ISFOLDER=$(wildcard $(RESULT_PATH))

all: compile1
	
compile1: mkdir
	@for i in $(MAIN_SOURCE);\
		do ($(CC) $(CFLAGS) -o $(RESULT_PATH)/$$i $$i.c $(INCLUDE_SOURCE));\
	done
mkdir:
ifeq ($(strip $(ISFOLDER)),)
	mkdir $(RESULT_PATH)
else
	echo "$(RESULT_PATH) exists.";
endif

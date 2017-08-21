include ../Rule.mak
CP=g++
MAIN_SOURCE=clflushtest.c
RESULT_PATH=./result
ISFOLDER=$(wildcard $(RESULT_PATH))

all: clflushtest
	
clflushtest: mkdir
	$(CP) $(CFLAGS) -o $(RESULT_PATH)/$@ $(MAIN_SOURCE) $(INCLUDE_SOURCE)

mkdir:
ifeq ($(strip $(ISFOLDER)),)
	mkdir $(RESULT_PATH)
else 
	echo "$(RESULT_PATH) exists."
endif

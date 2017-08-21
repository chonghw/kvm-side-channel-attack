include ./Rule.mak

DIR= one_process_covert_channel multi_process_covert_channel

all:
	@for i in $(DIR);\
		do (echo"";echo "====>./$$i/make all <===="; cd $$i; make all; echo "";);\
		done

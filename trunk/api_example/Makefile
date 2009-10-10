#
# Assume you have opt and debug directory for optimized and debugging compilations respectively
#
CC=gcc
PATH_OPT=../opt
PATH_DEBUG=../debug
PATH_SRC=../src

all: example1_debug example2_debug

example1_debug: example1_debug.o $(PATH_DEBUG)/src/.libs/libopensmt.a
	${CC} -g  -o example1_debug example1_debug.o -L$(PATH_DEBUG)/src/.libs -lopensmt -L/opt/local/lib -lgmp -lgmpxx

example1_debug.o: example1.c
	${CC} -c -I$(PATH_SRC)/api example1.c -o example1_debug.o

example1_opt: example1_opt.o $(PATH_OPT)/src/.libs/libopensmt.a
	${CC} -O3 -o example1_opt example1_opt.o -L$(PATH_OPT)/src/.libs -lopensmt -L/opt/local/lib -lgmp -lgmpxx

example1_opt.o: example1.c
	${CC} -c -I$(PATH_SRC)/api example1.c -o example1_opt.o
	
example2_debug: example2_debug.o $(PATH_DEBUG)/src/.libs/libopensmt.a
	${CC} -g  -o example2_debug example2_debug.o -L$(PATH_DEBUG)/src/.libs -lopensmt -L/opt/local/lib -lgmp -lgmpxx

example2_debug.o: example2.c
	${CC} -c -I$(PATH_SRC)/api example2.c -o example2_debug.o

example2_opt: example2_opt.o $(PATH_OPT)/src/.libs/libopensmt.a
	${CC} -O3 -o example2_opt example2_opt.o -L$(PATH_OPT)/src/.libs -lopensmt -L/opt/local/lib -lgmp -lgmpxx

example2_opt.o: example2.c
	${CC} -c -I$(PATH_SRC)/api example2.c -o example2_opt.o


clean:
	rm -f example1_opt example1_debug example1_opt.o example1_debug.o
	rm -f example2_opt example2_debug example2_opt.o example2_debug.o

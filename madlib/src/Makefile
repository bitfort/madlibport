
#
# Victor's Main Memory Makefile
#


PORT=ports/mainmem/
MODS=$(PORT)/modules
MADLIB=modules/

CPP=g++ -O0 -g -I$(PORT) -I$(MADLIB) -I. -I/usr/include/eigen3/

all: bin/logreg 
#bin/linreg

bin/logreg:
	$(CPP) -o bin/logreg $(MODS)/logreg.cc

bin/linreg:
	$(CPP) -o bin/linreg $(MODS)/linreg.cc

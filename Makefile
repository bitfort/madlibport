EIGEN=/usr/include/eigen3/

SRC=-I$(EIGEN) -Imadlib/src -Imadlib/src/ports -Imadlib/src/ports/metaport -Isrc -Iudf/

all: bin/linreg_test bin/logreg_test

objs/udf.o:
	g++ -I. udf/udf/udf.cc -c -o objs/udf.o $(SRC)

uda_test: objs/udf.o
	g++ -I. -o uda_test -lgtest udf/udf/uda-test.cc objs/udf.o $(SRC)

udf_test: objs/udf.o
	g++ -I. -o udf_test -lgtest udf/udf/udf-test.cc objs/udf.o $(SRC)

bin/linreg_test: objs/udf.o
	g++ -I. -o bin/linreg_test -lgtest test/test-linreg.cc objs/udf.o -g -O0 $(SRC)

bin/logreg_test: objs/udf.o
	g++ -I. -o bin/logreg_test -lgtest test/test-logreg.cc objs/udf.o -g -O0 $(SRC)

documentation:
	doxygen doc/doxconf

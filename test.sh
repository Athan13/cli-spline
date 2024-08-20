gcc -Wall -g -c src/spline-calc.c -o spline-calc.o
gcc -Wall -g -c test/spline-calc-test.c -o spline-test.o
gcc -Wall -g spline-calc.o spline-test.o -lgsl -lgslcblas -o cli-spline-test.out

rm -f spline-calc.o spline-test.o

./cli-spline-test.out
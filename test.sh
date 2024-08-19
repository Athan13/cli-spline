gcc -Wall -g -c -I/opt/homebrew/include src/spline-calc.c -o spline-calc.o
gcc -Wall -g -c -I/opt/homebrew/include test/spline-calc-test.c -o spline-test.o
gcc -Wall -g spline-calc.o spline-test.o -lgsl -gslcblas -o cli-spline-test.out

rm -f spline-calc.o spline-test.o

./cli-spline-test.out
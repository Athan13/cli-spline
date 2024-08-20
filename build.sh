gcc -Wall -c src/spline-calc.c -o spline-calc.o
gcc -Wall -c src/cli-spline.c -o cli-spline.o
gcc -Wall spline-calc.o cli-spline.o -lgsl -lgslcblas -lncurses -o cli-spline.out

rm -f spline-calc.o cli-spline.o
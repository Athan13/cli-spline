gcc -Wall -c -I/opt/homebrew/include src/spline-calc.c -o spline-calc.o
gcc -Wall -c -I/opt/homebrew/include src/cli-spline.c -o cli-spline.o
gcc -Wall spline-calc.o cli-spline.o -lgsl -gslcblas -lncurses -o cli-spline.out

rm -f spline-calc.o cli-spline.o
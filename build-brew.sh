gcc -Wall -c -I/opt/homebrew/include src/spline-calc.c -o spline-calc.o
gcc -Wall -c -I/opt/homebrew/include src/cli-spline.c -o cli-spline.o
gcc -Wall spline-calc.o cli-spline.o /opt/homebrew/lib/libgsl.a /opt/homebrew/lib/libgslcblas.a -lncurses -o cli-spline

rm -f spline-calc.o cli-spline.o
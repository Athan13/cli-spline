gcc -Wall -I/opt/homebrew/include -c src/cli-spline.c src/spline-calc.c -o cli-spline.o
gcc cli-spline.o -L/opt/homebrew/lib -lgsl -lncurses -o cli-spline

rm -f cli-spline.o
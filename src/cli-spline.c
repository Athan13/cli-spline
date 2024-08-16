#include <stdio.h>
#include <unistd.h>

#include <gsl/gsl_matrix.h>
#include <ncurses.h>

typedef struct Ant{
    uint32_t x;
    uint32_t y;
    uint32_t t;
} Ant;

// Loop for creating the path that the ants follow.
int make_path_loop(WINDOW *instruction_window, WINDOW *game_window) {
    box(instruction_window, 0, 0);

    wrefresh(instruction_window);

    while(1) {
        box(game_window, 0, 0);
        wrefresh(game_window);
        break;
    }

    return 0;
}

// Loop for the main part of the game: seeing the ants follow the path.
int ants_loop() {
    return 0;
}

int main(int argc, char** argv) {

    // initialise ncurses
    initscr();

    noecho();
    cbreak();

    WINDOW *instructions_window = newwin(5, COLS, 0, 0);
    WINDOW *game_window = newwin(LINES - 5, COLS, 5, 0);

    nodelay(instructions_window, true);
    nodelay(game_window, true);

    make_path_loop(instructions_window, game_window);
    sleep(2);

    // Cleanup ncurses
    endwin();
    delwin(instructions_window);
    delwin(game_window);

    return 0;
}
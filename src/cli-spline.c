#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <gsl/gsl_matrix.h>
#include <ncurses.h>

#include "spline-calc.h"

#define INSTRUCT_HEIGHT 5
#define MAX_PEBBLES 50
#define MAX_ANTS 50

#define ANT_CH 'O'
#define PEBBLE_CH 'X'

typedef struct Ant{
    size_t x;
    size_t y;
    size_t t;
} Ant;

// Game loop for creating the path that the ants follow.
// Returns a vector the x-y coordinates of the pebbles.
gsl_vector* make_path_loop(WINDOW *instruction_window, WINDOW *game_window) {
    // Create and draw instruction window
    mvwaddstr(instruction_window, 1, 1, "To move the pebble, use H (left), J (down), K (up), L (right)");
    mvwaddstr(instruction_window, 2, 1, "Press SPACE to drop the pebble, and to get a new one.");
    mvwaddstr(instruction_window, 3, 1, "Press ENTER when you're done dropping pebbles.");

    box(instruction_window, 0, 0);
    wrefresh(instruction_window);

    // Create main window
    struct timespec remaining, rest = {0, 5E7};

    gsl_vector* pebbles_xy = gsl_vector_alloc(MAX_PEBBLES * 2);
    if (pebbles_xy == NULL) return NULL;

    size_t num_pebbles = 0;

    size_t cursor_x = 1, cursor_y = 1;
    int input_ch;

    while(1) {       
        // Draw game window
        werase(game_window);

        for (size_t i = 0; i < num_pebbles; i++) {
            size_t x = (size_t) gsl_vector_get(pebbles_xy, 2 * i);
            size_t y = (size_t) gsl_vector_get(pebbles_xy, 2 * i + 1);
            mvwaddch(game_window, y, x, PEBBLE_CH);
        }

        mvwaddch(game_window, cursor_y, cursor_x, PEBBLE_CH);
        
        box(game_window, 0, 0);

        wrefresh(game_window);
        nanosleep(&rest, &remaining);

        // Get keyboard input
        input_ch = wgetch(game_window);
        switch(input_ch) {
            case 'h':
                cursor_x -= (cursor_x == 1) ? 0 : 1;
                break;
            case 'j':
                cursor_y += (cursor_y == LINES - INSTRUCT_HEIGHT - 2) ? 0 : 1;
                break;
            case 'k':
                cursor_y -= (cursor_y == 1) ? 0 : 1;
                break;
            case 'l':
                cursor_x += (cursor_x == COLS - 2) ? 0 : 1;
                break;
            case ' ':
                if (num_pebbles == MAX_PEBBLES) break;
                gsl_vector_set(pebbles_xy, 2 * num_pebbles, cursor_x);
                gsl_vector_set(pebbles_xy, 2 * num_pebbles + 1, cursor_y);
                num_pebbles++;
                break;
            case '\n':
                if (num_pebbles < 2) break;
                pebbles_xy->size = num_pebbles * 2;
                return pebbles_xy;
            default:
                break;
        } 
    }

    return NULL;
}

// Game loop for the main part of the game: seeing the ants follow the path.
int ants_loop() {
    return 0;
}

int main(int argc, char** argv) {
    // Initialise ncurses
    initscr();
    noecho();
    cbreak();
    curs_set(0);

    WINDOW *instruction_window = newwin(INSTRUCT_HEIGHT, COLS, 0, 0);
    WINDOW *game_window = newwin(LINES - INSTRUCT_HEIGHT, COLS, INSTRUCT_HEIGHT, 0);

    nodelay(instruction_window, true);
    nodelay(game_window, true);

    // Game loop 1
    gsl_vector *pebbles_xy = make_path_loop(instruction_window, game_window);
    if (pebbles_xy == NULL) {
        endwin();
        delwin(instruction_window);
        delwin(game_window);

        curs_set(2); 

        fprintf(stderr, "Something went wrong in game loop 1.");
        return 1;
    }

    // Linear algebra for splines

    // Game loop 2 (main game loop)

    // Cleanup - ncurses
    endwin();
    delwin(instruction_window);
    delwin(game_window);

    curs_set(2);

    // Cleanup - vectors
    gsl_vector_free(pebbles_xy);

    return 0;
}

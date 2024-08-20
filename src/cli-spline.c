#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <gsl/gsl_matrix.h>
#include <ncurses.h>

#include "spline-calc.h"

#define INSTRUCT_HEIGHT 6
#define MAX_PEBBLES 100
#define MAX_ANTS 100

#define ANT_CH 'O'
#define PEBBLE_CH 'X'

// Game loop for creating the path that the ants follow.
// Returns a vector the x-y coordinates of the pebbles.
gsl_vector* make_path_loop(WINDOW* instruction_window, WINDOW* game_window, double clock_ns) {
    // Create and draw instruction window
    mvwaddstr(instruction_window, 1, 1, "To move the pebble, use H (left), J (down), K (up), L (right)");
    mvwaddstr(instruction_window, 2, 1, "Press SPACE to drop the pebble, and to get a new one.");
    mvwaddstr(instruction_window, 3, 1, "Press ENTER when you're done dropping pebbles (there must be at least three pebbles).");
    mvwaddstr(instruction_window, 4, 1, "Press ESC to exit the game.");

    box(instruction_window, 0, 0);
    wrefresh(instruction_window);

    // Create main window
    struct timespec remaining, rest = {0, clock_ns};

    gsl_vector* pebbles_xy = gsl_vector_alloc(MAX_PEBBLES * 2);

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
                if (num_pebbles < 3) break;
                pebbles_xy->size = num_pebbles * 2;
                return pebbles_xy;
            case 27:  // escape
                gsl_vector_free(pebbles_xy);
                return NULL;
        } 
    }

    return NULL;
}

// Game loop for the main part of the game: seeing the ants follow the path.
int ants_loop (WINDOW* instruction_window, WINDOW* game_window, 
                const gsl_vector* pebbles_xy, const gsl_matrix* path_matrix, double clock_ns) {

    // Create and draw instruction window
    werase(instruction_window);
    mvwaddstr(instruction_window, 1, 1, "Press SPACE to send another ant down the path.");
    mvwaddstr(instruction_window, 2, 1, "Press K to increase and J to decrease the speed of the ants.");
    mvwaddstr(instruction_window, 3, 1, "Press ESC to exit the game.");
    mvwaddstr(instruction_window, 4, 1, "Look at those ants go!");

    box(instruction_window, 0, 0);
    wrefresh(instruction_window);

    // The "active" ants are the ants with indeces first_ant_index to first_ant_index + num_ants.
    size_t ants[MAX_ANTS] = {0};
    size_t first_ant_index = 0;
    size_t num_ants = 1;

    size_t num_pebbles = pebbles_xy->size / 2;
    size_t path_length = path_matrix->size1 * path_matrix->size2 / 2;

    gsl_vector_view x_path = gsl_vector_view_array(path_matrix->data, path_length);
    gsl_vector_view y_path = gsl_vector_view_array(path_matrix->data + path_length, path_length);

    int input_ch;

    while(1) {       
        // Draw game window
        werase(game_window);

        size_t x, y;
        for (size_t i = 0; i < num_pebbles; i++) {
            x = (size_t) gsl_vector_get(pebbles_xy, 2 * i);
            y = (size_t) gsl_vector_get(pebbles_xy, 2 * i + 1);
            mvwaddch(game_window, y, x, PEBBLE_CH);
        }

        size_t ant_i, ant_t, ant_x, ant_y;
        for (size_t i = 0; i < num_ants; i++) {
            ant_i = (first_ant_index + i) % MAX_ANTS;
            ant_t = ants[ant_i];
            ant_x = (size_t) gsl_vector_get(&x_path.vector, ant_t);
            ant_y = (size_t) gsl_vector_get(&y_path.vector, ant_t);
            mvwaddch(game_window, ant_y, ant_x, ANT_CH);

            ants[ant_i]++;
            if (ants[ant_i] == path_length) {
                ants[ant_i] = 0;
                first_ant_index++;
                num_ants--;
            }
        }

        box(game_window, 0, 0);

        wrefresh(game_window);
        struct timespec remaining, rest = {0, clock_ns};
        nanosleep(&rest, &remaining);

        // Get keyboard input
        input_ch = wgetch(game_window);
        switch(input_ch) {
            case ' ':
                if (num_ants == MAX_ANTS) break;
                num_ants++;
                break;
            case 27:  // escape key
                return 0;
            case 'k':
                clock_ns /= 1.5;
                break;
            case 'j':
                clock_ns *= 1.5;
                break;
        }
    }

    return 1;
}

int main(int argc, char** argv) {
    // Initialise ncurses
    initscr();
    noecho();
    cbreak();
    curs_set(0);

    WINDOW* instruction_window = newwin(INSTRUCT_HEIGHT, COLS, 0, 0);
    WINDOW* game_window = newwin(LINES - INSTRUCT_HEIGHT, COLS, INSTRUCT_HEIGHT, 0);

    nodelay(instruction_window, true);
    nodelay(game_window, true);

    double clock_ns = 2E7;

    // Game loop 1
    gsl_vector* pebbles_xy = make_path_loop(instruction_window, game_window, clock_ns);
    if (pebbles_xy == NULL) goto cleanup;

    // Linear algebra for splines
    size_t t_sample_resolution = 50;
    gsl_matrix* path_matrix = gsl_matrix_alloc(pebbles_xy->size - 2, t_sample_resolution);
    create_path(path_matrix, pebbles_xy, t_sample_resolution);

    // Game loop 2 (main game loop)
    ants_loop(instruction_window, game_window, pebbles_xy, path_matrix, clock_ns);

    // Cleanup - vectors
    gsl_vector_free(pebbles_xy);
    gsl_matrix_free(path_matrix);

cleanup:
    // Cleanup - ncurses
    endwin();
    delwin(instruction_window);
    delwin(game_window);

    curs_set(2);
    
    printf("Thanks for playing!\n");
    system("stty sane");  // ncurses sometimes messes with alignment of terminal

    return 0;
}

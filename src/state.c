#include "state.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_row(unsigned int cur_row, char c);
static unsigned int get_next_col(unsigned int cur_col, char c);
static void find_head(game_state_t *state, unsigned int snum);
static char next_square(game_state_t *state, unsigned int snum);
static void update_tail(game_state_t *state, unsigned int snum);
static void update_head(game_state_t *state, unsigned int snum);

/* Task 1 */
game_state_t *create_default_state() {
    // TODO: Implement this function.
    game_state_t* gst = malloc(sizeof(game_state_t));
    if (gst == NULL) {
        printf("failed to create game_state_t\n");
        exit(1); 
    }
    gst->num_rows = 18;
    gst->board = malloc(gst->num_rows * sizeof(char*));
    for (int i=0; i<gst->num_rows; i++) {
        char* tmp = malloc(21 * sizeof(char));
        if (tmp == NULL) {
            printf("failed to create char array\n");
            exit(1);
        }
        char* s = i == 0 || i == 17 ? "####################" : "#                  #";
        strcpy(tmp, s);
        gst->board[i] = tmp;
    }
    set_board_at(gst, 2, 9, '*');
    set_board_at(gst, 2, 2, 'd');
    set_board_at(gst, 2, 3, '>');
    set_board_at(gst, 2, 4, 'D');
    gst->num_snakes = 1;
    snake_t* st = malloc(sizeof(snake_t));
    if (st == NULL) {
        printf("failed to create snake_t\n");
        exit(1);
    }
    st->tail_row = 2;
    st->tail_col = 2;
    st->head_row = 2;
    st->head_col = 4;
    st->live = true;
    gst->snakes = st; 
    return gst;
}

/* Task 2 */
void free_state(game_state_t *state) {
    // TODO: Implement this function.
    for (int i=0; i<state->num_rows; i++){
        free(state->board[i]);
    }
    free(state->snakes);
    free(state->board);
    free(state);
    return;
}

/* Task 3 */
void print_board(game_state_t *state, FILE *fp) {
    // TODO: Implement this function.
    for (int i=0; i<state->num_rows; i++) {
        fprintf(fp, "%s\n",state->board[i]);
    }
    return;
}

/*
   Saves the current state into filename. Does not modify the state object.
   (already implemented for you).
 */
void save_board(game_state_t *state, char *filename) {
    FILE *f = fopen(filename, "w");
    print_board(state, f);
    fclose(f);
}

/* Task 4.1 */

/*
   Helper function to get a character from the board
   (already implemented for you).
 */
char get_board_at(game_state_t *state, unsigned int row, unsigned int col) { return state->board[row][col]; }

/*
   Helper function to set a character on the board
   (already implemented for you).
 */
static void set_board_at(game_state_t *state, unsigned int row, unsigned int col, char ch) {
    state->board[row][col] = ch;
}

/*
   Returns true if c is part of the snake's tail.
   The snake consists of these characters: "wasd"
   Returns false otherwise.
 */
static bool is_tail(char c) {
    // TODO: Implement this function.
    if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
        return true;
    }
    return false;
}

/*
   Returns true if c is part of the snake's head.
   The snake consists of these characters: "WASDx"
   Returns false otherwise.
 */
static bool is_head(char c) {
    // TODO: Implement this function.
    if (c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'x') {
        return true;
    }
    return false;
}

/*
   Returns true if c is part of the snake.
   The snake consists of these characters: "wasd^<v>WASDx"
 */
static bool is_snake(char c) {
    // TODO: Implement this function.
    if (is_head(c) || is_tail(c) || c == '^' || c == '<' || c == 'v' || c == '>') {
        return true;
    }
    return false;
}

/*
   Converts a character in the snake's body ("^<v>")
   to the matching character representing the snake's
   tail ("wasd").
 */
static char body_to_tail(char c) {
    // TODO: Implement this function.
    switch(c) {
        case '^': return 'w';
        case '<': return 'a';
        case 'v': return 's';
        case '>': return 'd';
    }
    return '?';
}

/*
   Converts a character in the snake's head ("WASD")
   to the matching character representing the snake's
   body ("^<v>").
 */
static char head_to_body(char c) {
    // TODO: Implement this function.
    switch(c) {
        case 'W': return '^';
        case 'A': return '<';
        case 'S': return 'v';
        case 'D': return '>';
    }
    return '?';
}

/*
   Returns cur_row + 1 if c is 'v' or 's' or 'S'.
   Returns cur_row - 1 if c is '^' or 'w' or 'W'.
   Returns cur_row otherwise.
 */
static unsigned int get_next_row(unsigned int cur_row, char c) {
    // TODO: Implement this function.
    if (c == 'v' || c == 's' || c == 'S') {
        return cur_row + 1;
    }
    else if (c == '^' || c == 'w' || c == 'W') {
        return cur_row - 1;
    }
    return cur_row;
}

/*
   Returns cur_col + 1 if c is '>' or 'd' or 'D'.
   Returns cur_col - 1 if c is '<' or 'a' or 'A'.
   Returns cur_col otherwise.
 */
static unsigned int get_next_col(unsigned int cur_col, char c) {
    // TODO: Implement this function.
    if (c == '>' || c == 'd' || c == 'D') {
        return cur_col + 1;
    }
    else if (c == '<' || c == 'a' || c == 'A') {
        return cur_col - 1;
    }
    return cur_col;
}

/*
   Task 4.2

   Helper function for update_state. Return the character in the cell the snake is moving into.

   This function should not modify anything.
 */
static char next_square(game_state_t *state, unsigned int snum) {
    // TODO: Implement this function.
    unsigned int hr = state->snakes[snum].head_row;
    unsigned int hc = state->snakes[snum].head_col;
    char head = state->board[hr][hc];
    if (head == 'W') {
        return state->board[hr-1][hc];
    }
    if (head == 'A') {
        return state->board[hr][hc-1];
    }
    if (head == 'S') {
        return state->board[hr+1][hc]; 
    }
    if (head == 'D') {
        return state->board[hr][hc+1];
    }
    return '?';
}

/*
   Task 4.3

   Helper function for update_state. Update the head...

   ...on the board: add a character where the snake is moving

   ...in the snake struct: update the row and col of the head

   Note that this function ignores food, walls, and snake bodies when moving the head.
 */
static void update_head(game_state_t *state, unsigned int snum) {
    // TODO: Implement this function.
    unsigned int hr = state->snakes[snum].head_row;
    unsigned int hc = state->snakes[snum].head_col;
    char head = state->board[hr][hc];
    if (head == 'W') {
        state->board[hr-1][hc] = 'W';
        state->board[hr][hc] = '^';
        state->snakes[snum].head_row = hr-1;
    }
    if (head == 'A') {
        state->board[hr][hc-1] = 'A';
        state->board[hr][hc] = '<';
        state->snakes[snum].head_col = hc-1;
    }
    if (head == 'S') {
        state->board[hr+1][hc] = 'S';
        state->board[hr][hc] = 'v';
        state->snakes[snum].head_row = hr+1;
    }
    if (head == 'D') {
        state->board[hr][hc+1] = 'D';
        state->board[hr][hc] = '>';
        state->snakes[snum].head_col = hc+1;
    }
    return;
}

/*
   Task 4.4

   Helper function for update_state. Update the tail...

   ...on the board: blank out the current tail, and change the new
   tail from a body character (^<v>) into a tail character (wasd)

   ...in the snake struct: update the row and col of the tail
 */
static void update_tail(game_state_t *state, unsigned int snum) {
    // TODO: Implement this function.
    unsigned int* hr = &state->snakes[snum].tail_row;
    unsigned int* hc = &state->snakes[snum].tail_col;
    char *tail = &state->board[*hr][*hc];
    if (*tail == 'w') {
        char* next_c = &state->board[(*hr)-1][*hc];
        *next_c = body_to_tail(*next_c); 
        *tail = ' ' ;
        *hr = (*hr)-1;
    }
    if (*tail == 'a') {
        char* next_c = &state->board[(*hr)][(*hc) - 1];
        *next_c = body_to_tail(*next_c); 
        *tail = ' ' ;
        *hc = (*hc) - 1;
    }
    if (*tail == 's') {
        char* next_c = &state->board[(*hr)+1][*hc];
        *next_c = body_to_tail(*next_c); 
        *tail = ' ' ;
        *hr = (*hr)+1;
    }
    if (*tail == 'd') {
        char* next_c = &state->board[*hr][(*hc) + 1];
        *next_c = body_to_tail(*next_c); 
        *tail = ' ' ;
        *hc = (*hc)+1;
    }
    return;
}

/* Task 4.5 */
void update_state(game_state_t *state, int (*add_food)(game_state_t *state)) {
    // TODO: Implement this function.
    // judge the head and then change the snake state
    for (unsigned int i=0; i<state->num_snakes; i++) {
        char next_c = next_square(state, i);
        if (state->snakes[i].live){
            if(is_snake(next_c) || next_c == '#') {
                set_board_at(state, state->snakes[i].head_row, state->snakes[i].head_col, 'x');
                state->snakes[i].live = false;
            }
            else if (next_c != '*') {
                update_head(state, i);
                update_tail(state, i); 
            }
            else {
                update_head(state, i);
                (*add_food)(state);
            }
        }
    }   
    return;
}

/* Task 5.1 */
char *read_line(FILE *fp) {
    // TODO: Implement this function.
    int s_len = 60;
    char *curline = malloc((size_t)s_len * sizeof(char));
    if (fgets(curline, s_len, fp) != NULL) {
        return curline;
    }
    return NULL;
}

/* Task 5.2 */
game_state_t *load_board(FILE *fp) {
    // TODO: Implement this function.
    game_state_t *gst = malloc(sizeof(game_state_t));
    if (gst == NULL) {
        printf("faile to allocate space for game_state_t");
    }
    gst->num_snakes = 0;
    gst->snakes = NULL;
    unsigned int num_rows = 0;
    char *line = NULL;
    while ((line = read_line(fp)) != NULL) {
        size_t ll = strlen(line);
        gst->board[num_rows++] = realloc(line, (ll+1) * sizeof(char));
        free(line);
    }
    gst->num_rows = num_rows; 
    return NULL;
}

/*
   Task 6.1

   Helper function for initialize_snakes.
   Given a snake struct with the tail row and col filled in,
   trace through the board to find the head row and col, and
   fill in the head row and col in the struct.
 */
static void find_head(game_state_t *state, unsigned int snum) {
    // TODO: Implement this function.
    return;
}

/* Task 6.2 */
game_state_t *initialize_snakes(game_state_t *state) {
    // TODO: Implement this function.
    return NULL;
}

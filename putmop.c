#include <ncurses.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

/* Number of cells in player 'memory' (bottom bar cells) */
#define MAX_PLAYER_CELLS 10

enum {
    POS_POS = 0,
    MAX_Y,
    MAX_X,
    NUM_META_DATA
};


/* Cell types with random location
 * Number is used to denote it in position array */
enum {
    MOVE_LEFT_KEY = 0,
    MOVE_RIGHT_KEY,
    MOVE_UP_KEY,
    MOVE_DOWN_KEY,
    PUT_VAL_KEY,
    PUT_CELL_POINTER_KEY,
    UPDATE_CELL_KEY,
    INCREMENT_KEY,
    DECREMENT_KEY,
    TEST_EQ_KEY,
    TEST_L_KEY,
    TEST_G_KEY,
    PUT_CELL_VAL_KEY,
    PUT_FROM_CELL_KEY,
    IF_TEST_KEY,
    ELSE_TEST_KEY,
    IF_END_KEY,
    BOT_PROGRAM_END,
    PLAYER_POS,
    PLAYER_POS_PTR,
    PLAYER_CELLS,
    PLAYER_VALUE,
    WORLD_SIZE,
    MOVE_LENGTH,
    WIN_FLAG,
    RESTART_GAME,
    SEED,
    IS_POINTER,
    POSITIONS,
    PLAYER_SYMBOL,
    BOTS
};

#define INHABITED_CELLS (1 + BOTS + NUMBER_OF_BOTS)

/* Bot specific constants */
#define NUMBER_OF_BOTS 5
#define PROGRAM_INIT_SIZE 5
#define NUMBER_OF_BOT_COMMANDS 16
enum {
    BOT_POS = 0,
    BOT_CELL_VAL,
    BOT_FLAG_VAL,
    BOT_BOOL,
    BOT_PROGRAM_COUNTER,
    BOT_PROGRAM,
    BOT_SIZE
};
/* Bot defaults to userinput 0 for all commands needing an input value */

/* Type for cells in world. Must be large enough to contain a normal C-pointer. */
typedef long long int CELLVALUE;


/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~PROTOTYPES~~~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void init_screen();

void execute_bot_step(int, CELLVALUE *);

void execute_bot_steps(CELLVALUE *);

void eval_input(char, CELLVALUE *);

void make_inits_values(CELLVALUE *, int *);

static int rand_int(int);

static void shuffle(int *, int);

int *get_randomized_cell_types_order();

int make_random_bot_command();

void init_bots(int, int, CELLVALUE *);

void init_test_bots(int, int, CELLVALUE *);

void init_world(int, int, int, CELLVALUE *);

char make_char(CELLVALUE);

void draw_map(WINDOW *, CELLVALUE *, int);

void draw_bottom_bar(WINDOW *, CELLVALUE *);

void clear_area(int, int, int, int);

char *get_input(char);

void skip_bot_if(int, CELLVALUE *);

/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~IMPLEMENTATION~~~~~~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void init_screen()
{
    initscr();
    noecho();
    cbreak();
    refresh();
}

/* Executes one step of bot and updates necessary values */
void execute_bot_step(int i, CELLVALUE *world)
{
    int bpos = world[world[POS_POS]+BOTS+i];
    int bc = world[bpos+BOT_PROGRAM_COUNTER];
    int com = world[bpos+BOT_PROGRAM+bc];

    if (com == world[world[world[POS_POS] + MOVE_RIGHT_KEY]]) {
        world[bpos + BOT_POS] += world[world[world[POS_POS] + MOVE_LENGTH]];
    } else if (com == world[world[world[POS_POS] + MOVE_UP_KEY]]) {
        world[bpos + BOT_POS] -= world[world[world[POS_POS] + MOVE_LENGTH]]*world[MAX_X];
    } else if (com == world[world[world[POS_POS] + MOVE_DOWN_KEY]]) {
        world[bpos + BOT_POS] += world[world[world[POS_POS] + MOVE_LENGTH]]*world[MAX_X];
    } else if (com == world[world[world[POS_POS] + MOVE_LEFT_KEY]]) {
        world[bpos + BOT_POS] -= world[world[world[POS_POS] + MOVE_LENGTH]];
    } else if (com == world[world[world[POS_POS] + PUT_CELL_VAL_KEY]]) {
        world[bpos + BOT_CELL_VAL] =  world[world[bpos + BOT_POS]];
        world[bpos + BOT_FLAG_VAL] = 0;
    } else if (com == world[world[world[POS_POS] + PUT_CELL_POINTER_KEY]]) {
        world[bpos + BOT_CELL_VAL] =  world[world[bpos + BOT_POS]];
        world[bpos + BOT_FLAG_VAL] = 1;
    } else if (com == world[world[world[POS_POS] + PUT_VAL_KEY]]) {
        if (world[bpos + BOT_FLAG_VAL])
            world[world[bpos + BOT_POS]] = world[world[bpos + BOT_CELL_VAL]];
        else
            world[world[bpos + BOT_POS]] = world[bpos + BOT_CELL_VAL];
    } else if (com == world[world[world[POS_POS] + UPDATE_CELL_KEY]]) {
        if (world[bpos + BOT_FLAG_VAL])
            world[world[bpos + BOT_CELL_VAL]] = world[world[bpos + BOT_POS]];
        else
            world[world[bpos + BOT_CELL_VAL]] = world[bpos + BOT_POS];
    } else if (com == world[world[world[POS_POS] + IF_TEST_KEY]]) {
        if (!world[world[bpos + BOT_BOOL]])
            skip_bot_if(bpos, world);
    } else if (com == world[world[world[POS_POS] + ELSE_TEST_KEY]]) {
        if (world[world[bpos + BOT_BOOL]])
            skip_bot_if(bpos, world);
    } else if (com == world[world[world[POS_POS] + BOT_PROGRAM_END]]) {
        world[bpos + BOT_PROGRAM_COUNTER] = -1;
    }

    world[bpos + BOT_PROGRAM_COUNTER]++;
}

/* Execute one step for all bots */
void execute_bot_steps(CELLVALUE *world)
{
    for (int i = 0; i < NUMBER_OF_BOTS; i++)
        execute_bot_step(i, world);
}

/* Skips the program counter of bot until if-block/else-block ends */
void skip_bot_if(int bpos, CELLVALUE *world)
{
    int c = bpos + BOT_PROGRAM + world[bpos+BOT_PROGRAM_COUNTER];

    while (world[c] != BOT_PROGRAM_END &&
           world[c] != ELSE_TEST_KEY && 
           world[c] != IF_END_KEY)
        world[bpos + BOT_PROGRAM_COUNTER]++;
}

void eval_input(char ch, CELLVALUE *world)
{
    if (ch == world[world[world[POS_POS] + MOVE_RIGHT_KEY]]) {
        world[world[world[POS_POS] + PLAYER_POS]] += world[world[world[POS_POS] + MOVE_LENGTH]];
    } else if (ch == world[world[world[POS_POS] + MOVE_UP_KEY]]) {
        world[world[world[POS_POS] + PLAYER_POS]] -= world[world[world[POS_POS] + MOVE_LENGTH]]*world[MAX_X];
    } else if (ch == world[world[world[POS_POS] + MOVE_DOWN_KEY]]) {
        world[world[world[POS_POS] + PLAYER_POS]] += world[world[world[POS_POS] + MOVE_LENGTH]]*world[MAX_X];
    } else if (ch == world[world[world[POS_POS] + MOVE_LEFT_KEY]]) {
        world[world[world[POS_POS] + PLAYER_POS]] -= world[world[world[POS_POS] + MOVE_LENGTH]];
    } else if (ch == world[world[world[POS_POS] + PUT_CELL_VAL_KEY]]) {

        char *strint = get_input(ch);
        CELLVALUE val = strtoll(strint, NULL, 0);
        free(strint);
        world[world[world[POS_POS] + PLAYER_CELLS]+val] =  world[world[world[world[POS_POS] + PLAYER_POS]]];
        world[world[world[POS_POS] + IS_POINTER]+val] = 0;

    } else if (ch == world[world[world[POS_POS] + PUT_CELL_POINTER_KEY]]) {

        char *strint = get_input(ch);
        CELLVALUE val = strtoll(strint, NULL, 0);
        free(strint);
        world[world[world[POS_POS] + PLAYER_CELLS]+val] = world[world[world[POS_POS] + PLAYER_POS]];
        world[world[world[POS_POS] + IS_POINTER]+val] = 1;

    } else if (ch == world[world[world[POS_POS] + PUT_VAL_KEY]]) {

        char *strint = get_input(ch);
        CELLVALUE val = strtoll(strint, NULL, 0);
        free(strint);
        world[world[world[world[POS_POS] + PLAYER_POS]]] = val;

    } else if (ch == world[world[world[POS_POS] + UPDATE_CELL_KEY]]) {

        char *strint = get_input(ch);
        CELLVALUE cell = strtoll(strint, NULL, 0);
        free(strint);

        char *strval = get_input(ch);
        CELLVALUE val = strtoll(strval, NULL, 0);
        free(strval);

        if (world[world[world[POS_POS] + IS_POINTER] + cell])
            world[world[world[world[POS_POS] + PLAYER_CELLS] + cell]] = val;
        else
            world[world[world[POS_POS] + PLAYER_CELLS] + cell] = val;
    }
}

/**
 * Initializes the array of initial values for each cell type
 * and the sizes for each cell type.
 */
void make_inits_values(CELLVALUE *init_values, int *sizes) {

    for (int i = 0; i < INHABITED_CELLS; i++)
        sizes[i] = 1;

    sizes[PLAYER_CELLS] = MAX_PLAYER_CELLS;
    sizes[IS_POINTER] = MAX_PLAYER_CELLS;
    sizes[POSITIONS] = INHABITED_CELLS;

    // Init bots' sizes, which are at the end of sizes
    for (int i = 1; i <= NUMBER_OF_BOTS; i++)
        sizes[INHABITED_CELLS-i] = BOT_SIZE + PROGRAM_INIT_SIZE;

    init_values[MOVE_LENGTH] = 1;
    init_values[MOVE_LEFT_KEY] = 'h';
    init_values[MOVE_RIGHT_KEY] = 'l';
    init_values[MOVE_UP_KEY] = 'k';
    init_values[MOVE_DOWN_KEY] = 'j';
    init_values[PUT_VAL_KEY] = 'p';
    init_values[PUT_CELL_VAL_KEY] = 'c';
    init_values[PUT_CELL_POINTER_KEY] = 'C';
    init_values[UPDATE_CELL_KEY] = 'e';
    init_values[IF_TEST_KEY] = '?';
    init_values[ELSE_TEST_KEY] = ':';
    init_values[WIN_FLAG] = 1;
    init_values[PLAYER_SYMBOL] = '@';
    init_values[BOT_PROGRAM_END] = 0;
}

/* Returns a random positive integer smaller than n */
static int rand_int(int n)
{
    if (n <= 0) return 0;

    int limit = RAND_MAX - RAND_MAX % n;
    int rnd;

    do {
        rnd = rand();
    } while (rnd >= limit);
    return rnd % n;
}

/* Simple Fisher-Yates shuffle of an array */
static void shuffle(int *array, int n)
{
    int j, tmp;

    for (int i = n - 1; i > 0; i--) {
        j = rand_int(i + 1);
        tmp = array[j];
        array[j] = array[i];
        array[i] = tmp;
    }
}

/* Randomizes the order of the habited cells */
int *get_randomized_cell_types_order()
{

    int *arr = malloc(INHABITED_CELLS * sizeof(int));

    for (int i = 0; i < INHABITED_CELLS; i++)
        arr[i] = i;

    shuffle(arr, INHABITED_CELLS);
    return arr;
}

int make_random_bot_command()
{
    return rand_int(NUMBER_OF_BOT_COMMANDS+1);
}

/* Initializes bots with random positions and programs */
void init_bots(int world_size, int pos, CELLVALUE *world)
{
    for (int i = 0; i < NUMBER_OF_BOTS; i++) {

        int bpos = world[pos+BOTS+i];
        world[bpos+BOT_POS] = rand_int(world_size); 
        world[bpos+BOT_PROGRAM_COUNTER] = 0;

        for (int j = 0; j < PROGRAM_INIT_SIZE-2; j++)
            world[bpos+BOT_PROGRAM+j] = world[world[pos+make_random_bot_command()]];
        
        world[bpos+BOT_PROGRAM+PROGRAM_INIT_SIZE-1] = world[world[pos+BOT_PROGRAM_END]];
    }
}

/* Hard-coded bot for testing */
void init_test_bots(int world_size, int pos, CELLVALUE *world)
{
    for (int i = 0; i < NUMBER_OF_BOTS; i++) {

        int bpos = world[pos+BOTS+i];
        world[bpos+BOT_POS] = 50;
        world[bpos+BOT_PROGRAM_COUNTER] = 0;

        CELLVALUE commands[PROGRAM_INIT_SIZE] = {MOVE_LEFT_KEY, MOVE_DOWN_KEY, 0, 0, 0};

        for (int j = 0; j < PROGRAM_INIT_SIZE; j++)
            world[bpos+BOT_PROGRAM+j] = world[world[pos+commands[j]]];
    }
}

/* Inits the world with random placements for each cell type, and
 * fills in the inital values.
 * Returns the position of the position-array in the world.
 */
void init_world(int world_size, int world_win_max_y, int world_win_max_x, CELLVALUE *world)
{
    int *positions = malloc(sizeof(int) * INHABITED_CELLS);
    int *sizes = malloc(sizeof(int) * INHABITED_CELLS);
    int *order = get_randomized_cell_types_order();

    CELLVALUE *init_values = malloc(sizeof(CELLVALUE)*INHABITED_CELLS);

    make_inits_values(init_values, sizes);

    int rest_size = 0;
    for (int j = 0; j < INHABITED_CELLS; j++)
        rest_size += sizes[j];

    int i = 0;
    int pos = NUM_META_DATA;

    while (i < INHABITED_CELLS) {

        pos += rand_int(world_size - (1 + pos + rest_size));
        positions[order[i]] = pos;
        world[pos] = init_values[order[i]];
        pos += sizes[order[i]];
        rest_size -= sizes[order[i]];
        i++;
    }

    for (int k = 0; k < INHABITED_CELLS; k++)
        world[positions[POSITIONS]+k] = positions[k];

    //init_bots(world_size, positions[POSITIONS], world);
    init_test_bots(world_size, positions[POSITIONS], world);
    world[POS_POS] = positions[POSITIONS];
    world[MAX_X] = world_win_max_x;
    world[MAX_Y] = world_win_max_y;
}

/* Simply returns a character for drawing val */
char make_char(CELLVALUE val)
{
    if (val == 0)
        return '.';
    else {
        char c = (char) (33 + abs(val % 92)); // c should now be printable ASCII char

        // c should not be 46='.' nor 64='@'
        if (c < 46)
            return c;
        else if (c < 63)
            return c+1;
        else
            return c+2;
    }
}

void draw_map(WINDOW *win, CELLVALUE *world, int world_size)
{
    int row, col;
    int i = 0;

    getmaxyx(win, row, col);
    (void) row; // Unused variable needed for above macro
    wmove(win, 1, 1);

    while (i < world_size) {
        if (i == world[world[world[0] + PLAYER_POS]])
            waddch(win, world[world[world[0] + PLAYER_SYMBOL]]);
        else
            waddch(win, make_char(world[i]));

        i++;
        if (i % (col-2) == 0)
            wmove(win, (i / (col-2))+1, 1);
    }

    for (i = 0; i < NUMBER_OF_BOTS; i++) {
        int bpos = world[world[world[0]+INHABITED_CELLS-i]+BOT_POS];
        int by = bpos / (col-2);
        int bx = bpos - (by*(col-2));
        mvwaddch(win, 1+by, 1+bx, world[world[world[0]+PLAYER_SYMBOL]]);
    }

    box(win, 0, 0);
    wrefresh(win);
}

void draw_bottom_bar(WINDOW *win, CELLVALUE *world)
{
    int col, row;
    getmaxyx(win, row, col);
    (void) row; // Unused variable needed for above macro
    wmove(win, 0, 0);

    int d = 0;
    int curow = 0;

    CELLVALUE val;

    for (int i = 0; i < MAX_PLAYER_CELLS; i++) {

        if ((d += 22) > col) {
            wmove(win, ++curow, 0);
            d = 0;
        }

        if (world[world[world[0] + IS_POINTER]+i] == 1) 
            val = world[world[world[world[0] + PLAYER_CELLS]+i]]; 
        else
            val = world[world[world[0] + PLAYER_CELLS]+i];

        wprintw(win, "%22d", val);
    }

    int k, j;
    getyx(win, j, k);
    (void) j; // Unused variable needed for above macro

    while (k++ < col-1) waddch(win, ' ');
    wrefresh(win);
}

void clear_area(int fromY, int toY, int fromX, int toX)
{
    move(fromY, fromX);
    int curX = fromX, curY = fromY;

    while (curY <= toY) {
        addch(' ');
        curX++;

        if (curX == toX) {
            curX = 0;
            curY++;
            move(curY, 0);
        }
    }
}

char *get_input(char key)
{
    int row, col;

    echo();
    getmaxyx(stdscr, row, col);
    move(row-1, 0);
    addch(key);

    char *strint = malloc(sizeof(char)*64);
    getstr(strint);
    noecho();
    clear_area(row-1, row, 0, col);

    return strint;
}

int main()
{
    /* Intializes random number generator */
    time_t t;
    srand((unsigned) time(&t));

    init_screen();

    CELLVALUE row, col;
    getmaxyx(stdscr, row, col);

    CELLVALUE world_win_max_y = row-4;
    CELLVALUE world_win_max_x = col;
    CELLVALUE world_max_y = world_win_max_y - 2;
    CELLVALUE world_max_x = world_win_max_x - 2;
    
    int world_size = world_max_y*world_max_x;
    CELLVALUE *world = malloc(sizeof(CELLVALUE) * world_size);;
    init_world(world_size, world_max_y, world_max_x, world);

    WINDOW *map_win = newwin(world_win_max_y, world_win_max_x, 0, 0);
    WINDOW *bottom_bar = newwin(4, col, row-4, 0);
    box(map_win, 0, 0);

    wrefresh(map_win);
    wrefresh(bottom_bar);

    draw_map(map_win, world, world_size);
    draw_bottom_bar(bottom_bar, world);

    while (true) {
        eval_input(getch(), world);
        execute_bot_steps(world);
        world[world[world[POS_POS] + PLAYER_VALUE]] = world[world[world[world[0] + PLAYER_POS]]];
        draw_map(map_win, world, world_size);
        draw_bottom_bar(bottom_bar, world);

        if (!world[world[world[POS_POS] + WIN_FLAG]]) break;
    }

    endwin();
    free(world);
    return 0;
}

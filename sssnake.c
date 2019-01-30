#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define SNAKE_DELAY_MAX     100000
#define SNAKE_DELAY_MIN     40000
#define SNAKE_GETCH_TIMEOUT 0.1
#define SNAKE_MAX_LENGTH    100
#define SNAKE_END_SCORE     10

char *snake_menu[] = {
                        "Play",
                        "Exit",
};

struct snake_coord {
    int x, y;
};

struct snake {
    struct snake_coord body[SNAKE_MAX_LENGTH];
    int len;
};

struct snake_game {
    int food_x, food_y, caught;
    int speed_inc, speed;
    int next_x, next_y;
    int max_x, max_y;
    int score, color;
    int endscore;

    struct snake snake;
};

void snake_exit(const char *message)
{
    int max_x, max_y;

    getmaxyx(stdscr, max_y, max_x);
    clear();
    mvprintw(max_y/2, max_x/2, message);
    refresh();
    sleep(1);
    timeout(100000);
    endwin();
    exit(0);
}

int snake_collision(struct snake *snake)
{
    int head_x = snake->body[0].x;
    int head_y = snake->body[0].y;
    int max_x, max_y;

    /* Check if snake intersects its body */
    for (int i = 1; i < snake->len; i++) {
           if (head_x == snake->body[i].x &&
           head_y == snake->body[i].y)
            return 1;
    }

    /* Check if snake head hits the wall */
    getmaxyx(stdscr, max_y, max_x);

    if (head_x >= max_x || head_y >= max_y || head_x < 0 || head_y < 0)
        return 1;

    return 0;
}

void snake_init(struct snake *snake)
{
    for (int i = 1; i < SNAKE_MAX_LENGTH; i++) {
           snake->body[i].x = -10;
           snake->body[i].y = -10;
    }

    snake->body[0].x = 0;
    snake->body[0].y = 0;

    /* We have to know the coordinates for new body */
    snake->body[1].x = 0;
    snake->body[1].y = 0;
    snake->len = 1;
}

void snake_move(struct snake *snake, int enlarge, int direction_x, int direction_y)
{
    int i;

    if (enlarge) {
        i = snake->len;
        snake->len++;
    } else {
        i = snake->len-1;
    }

    /* move the body */
    for (; i > 0; i--) {
        snake->body[i].x = snake->body[i-1].x;
        snake->body[i].y = snake->body[i-1].y;
    }

    /* move the head */
    snake->body[0].x += direction_x;
    snake->body[0].y += direction_y;

    if (snake_collision(snake))
        snake_exit("Game over!");
}


void snake_print(struct snake *snake)
{
    /* Print the body */
    for (int i = snake->len-1; i > 0; i--) {
        int x = snake->body[i].x;
        int y = snake->body[i].y;
        if (x == -10 && y == -10) break;
        mvprintw(y, x, "x");
    }

    /* Print head of the snake */
    mvprintw(snake->body[0].y, snake->body[0].x, "X");
}

void snake_increase_speed(struct snake_game *game)
{
    if (game->speed > SNAKE_DELAY_MIN)
        game->speed -= game->speed_inc;

    if (game->speed < SNAKE_DELAY_MIN)
        game->speed = SNAKE_DELAY_MIN;

    return;
}

void snake_setup_colors()
{
    /*
     * Simple color assignment, often all we need.  Color pair 0 cannot
     * be redefined.  This example uses the same value for the color
     * pair as for the foreground color, though of course that is not
     * necessary:
     */
    init_pair(1, COLOR_RED,     COLOR_BLACK);
    init_pair(2, COLOR_GREEN,   COLOR_BLACK);
    init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
    init_pair(4, COLOR_BLUE,    COLOR_BLACK);
    init_pair(5, COLOR_CYAN,    COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_WHITE,   COLOR_BLACK);
}

void snake_init_screen()
{
    initscr();                    /* Initialize the window */
    /* noecho();                  // Don't echo any keypresses */
    /* cbreak();                  // Don't wait for new line */
    timeout(SNAKE_GETCH_TIMEOUT); /* getch() timeout */
    keypad(stdscr, TRUE);         /* enable keyboard mapping */
    nonl();                       /* tell curses not to do NL->CR/NL on output */
    cbreak();                     /* take input chars one at a time, no wait for \n */
    echo();                       /* echo input - in color */
    curs_set(FALSE);              /* Don't display a cursor */
}

void snake_game_init(struct snake_game *game)
{
    game->endscore = SNAKE_END_SCORE;
    game->score = 0;
    /* To make the food appear ?? */
    game->caught = 1;
    game->speed = SNAKE_DELAY_MAX;
    /* Make the speed increasing up to 3/4 of the game's end score */
    game->speed_inc = (SNAKE_DELAY_MAX - SNAKE_DELAY_MIN)/(3*game->endscore/4);

    /* Setup snake */
    snake_init(&game->snake);

    /* Setup ncurses stuff */
    snake_init_screen();

    if (has_colors()) {
        start_color();
        snake_setup_colors();
    }
}

/*
 * 258 KEY_DOWN
 * 259 KEY_UP
 * 260 KEY_LEFT
 * 261 KEY_RIGHT
 */
void snake_run(struct snake_game *game)
{
    int direction_x = 1, direction_y = 0;
    struct snake *snake = &game->snake;
    int next_x = 0, next_y = 0;
    int max_x = 0, max_y = 0;
    int *head_x = &snake->body[0].x;
    int *head_y = &snake->body[0].y;

    while(1) {
        int c = getch();

        switch(c) {
            case KEY_LEFT:
                /* Check it's not RIGHT, we can't turn the opposite direction */
                if (direction_x != 1 && direction_y != 0) {
                    direction_x = -1;
                    direction_y = 0;
                }
                break;
            case KEY_RIGHT:
                /* Check it's not LEFT, we can't turn the opposite direction */
                if (direction_x != -1 && direction_y != 0) {
                    direction_x = 1;
                    direction_y = 0;
                }
                break;
            case KEY_UP:
                /* Check it's not DOWN, we can't turn the opposite direction */
                if (direction_x != 0 && direction_y != 1) {
                    direction_x = 0;
                    direction_y = -1;
                }
                break;
            case KEY_DOWN:
                /* Check it's not UP, we can't turn the opposite direction */
                if (direction_x != 0 && direction_y != -1) {
                    direction_x = 0;
                    direction_y = 1;
                }
                break;
        }

        /* exit game 113 == 'q' */
        if (c == 113)
            break;

        /* set the color */
        if (c != -1 && c != game->color &&
            c != KEY_LEFT && c != KEY_RIGHT &&
            c != KEY_UP && c != KEY_DOWN)
            game->color = c;
        attrset(COLOR_PAIR(game->color % 8));

        /* Global var `stdscr` is created by the call to `initscr()` */
        getmaxyx(stdscr, max_y, max_x);

        /* Clear the screen of all */
        clear();

        /* print the score */
        mvprintw(max_y/2 - 3, max_x/2, "Score: %i", game->score);

        /* print the debug information */
        mvprintw(max_y/2 - 2, max_x/2, "X: %i", *head_x);
        mvprintw(max_y/2 - 1, max_x/2, "Y: %i", *head_y);
        mvprintw(max_y/2, max_x/2, "dirX: %i", direction_x);
        mvprintw(max_y/2 + 1, max_x/2, "dirY: %i", direction_y);
        mvprintw(max_y/2 + 2, max_x/2, "MaxX: %i", max_x);
        mvprintw(max_y/2 + 3, max_x/2, "MaxY: %i", max_y);
        mvprintw(max_y/2 + 4, max_x/2, "Char: %i", c);
        mvprintw(max_y/2 + 5, max_x/2, "Speed: %i", game->speed);
        mvprintw(max_y/2 + 6, max_x/2, "SpInc: %i", game->speed_inc);
        /* Print food at the current xy position */
        mvprintw(game->food_y, game->food_x, "o");

        snake_print(snake);
        refresh();

        /* Shorter delay between movements */
        usleep(game->speed);

        next_x = *head_x + direction_x;
        next_y = *head_y + direction_y;

        /* snake will got the food */
        if (next_x == game->food_x && next_y == game->food_y) {
            game->caught = 1;
            game->score++;
            snake_increase_speed(game);
        }

        snake_move(snake, game->caught, direction_x, direction_y);

        /* Generate a new place for food */
        if (game->caught) {
            srand(time(NULL));
            game->food_x = rand() % max_x;
            game->food_y = rand() % max_y;
            game->caught = 0;
        }

        if (game->score >= game->endscore)
            snake_exit("You won!");

    }

    /* Restore normal terminal behavior */
    endwin();
}

void snake_start_menu()
{
    int highlight = 0;
    int max_x, max_y;
    int choice, i;
    int enter = 0;

    snake_init_screen();

    getmaxyx(stdscr, max_y, max_x);
    WINDOW *menuwin = newwin(5, max_x/2 - 10, max_y/2 - 10, 5);
    box(menuwin, 0, 0);
    refresh();
    wrefresh(menuwin);

    keypad(menuwin, true);

    while (1) {
        highlight %= 2;
        clear();

        for (i = 0; i < 2; i++) {
            if (i == highlight)
                wattron(menuwin, A_REVERSE);
            mvwprintw(menuwin, i + 1, 1, snake_menu[i]);
            wattroff(menuwin, A_REVERSE);
        }

        choice = wgetch(menuwin);

        switch(choice) {
            case KEY_ENTER:
            case ' ':
            case 10:
                enter = 1;
                break;
            case KEY_UP:
                highlight--;
                break;
            case KEY_DOWN:
                highlight++;
                break;
            default:
                break;
        }

        if (enter == 1)
            break;
    }

    if (highlight == 0) {
        struct snake_game game;

        snake_game_init(&game);
        snake_run(&game);
    } else if (highlight == 1) {
        snake_exit("Bye!");
    }

    return;
}

int main(int argc, char *argv[])
{
    snake_start_menu();

    return 0;
}

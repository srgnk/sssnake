#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define DELAY 50000
#define SNAKE_MAX_LENGTH 100

struct coord {
	int x, y;
};

struct snake {
	struct coord full_snake[SNAKE_MAX_LENGTH];
	int len;
};

struct game {
	struct snake *snake;
	int color; 
	int num;
	int food_x, food_y, caught;
	int max_y, max_x;
	int next_x, next_y;
	int direction_x, direction_y;
	int x_was_max, y_was_max;
	int score;
	int *head_x, *head_y;
};

void game_exit() {
	int max_x, max_y, c;
	getmaxyx(stdscr, max_y, max_x);
	clear();
	mvprintw(max_y/2, max_x/2, "TY LOX!");
	//mvprintw(max_y/2 + 1, max_x/2, "Your score: %i\n", score);
	refresh();
	sleep(3);
	timeout(100000);
	c = getch();
	endwin();
	exit(0);
}

int snake_collision(struct snake *snake) {
	int head_x = snake->full_snake[0].x;
	int head_y = snake->full_snake[0].y;
	int max_x, max_y;
	/* Check if snake intersects its body */
	for (int i = 1; i < snake->len; i++) {
	       if (head_x == snake->full_snake[i].x &&
		   head_y == snake->full_snake[i].y)
			return 1; 
	}
	/* Check if snake head hit the wall */
	getmaxyx(stdscr, max_y, max_x);
	if (head_x >= max_x || head_y >= max_y ||
      	    head_x < 0 || head_y < 0)
		return 1;
	return 0;
}

void snake_init(struct snake *snake) {
	for (int i = 1; i < SNAKE_MAX_LENGTH; i++) {
	       snake->full_snake[i].x = -10;
	       snake->full_snake[i].y = -10;
	}
	snake->full_snake[0].x = 0;
	snake->full_snake[0].y = 0;
	/* We have to know the coordinates for new body */
	snake->full_snake[1].x = 0;
	snake->full_snake[1].y = 0;
	snake->len = 1;
}

void snake_move(struct snake *snake, int enlarge, int direction_x, int direction_y) {
	int i;
	if (enlarge) {
		i = snake->len;
		snake->len++;
	}
	else {
		i = snake->len-1;
	}

	for (; i > 0; i--) {
		snake->full_snake[i].x = snake->full_snake[i-1].x;
		snake->full_snake[i].y = snake->full_snake[i-1].y;
	}
	/* move the head */
	snake->full_snake[0].x += direction_x;
	snake->full_snake[0].y += direction_y;
	if (snake_collision(snake))
		game_exit();
}


void snake_print(struct snake *snake) {
	for (int i = snake->len-1; i > 0; i--) {
		int x = snake->full_snake[i].x;
		int y = snake->full_snake[i].y;
		if (x == -10 && y == -10) break;
		mvprintw(y, x, "x"); // Print the body
	}
	mvprintw(snake->full_snake[0].y,
		 snake->full_snake[0].x, "X"); // Print head of the snake
}

void setup_colors() {
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

int main(int argc, char *argv[]) {
	int color = 1; 
	int food_x = 0, food_y = 0, caught = 1;
	int max_y = 0, max_x = 0;
	int next_x = 0, next_y = 0;
	int direction_x = 1, direction_y = 0;
	int x_was_max = 0, y_was_max = 0;
	int score = 0;
	struct snake snake;
	int *x = &snake.full_snake[0].x, *y = &snake.full_snake[0].y;
	int *snake_len = &snake.len;

	// Setup snake
	snake_init(&snake);

	initscr(); // Initialize the window
	//noecho(); // Don't echo any keypresses
	//cbreak(); // Don't wait for new line
	timeout(0.1); // getch() timeout
	keypad(stdscr, TRUE); /* enable keyboard mapping */
	nonl(); /* tell curses not to do NL->CR/NL on output */
	cbreak(); /* take input chars one at a time, no wait for \n */
	echo(); /* echo input - in color */
	curs_set(FALSE); // Don't display a cursor

	if (has_colors()) {
		start_color();
		setup_colors();
	}
	
	/*
		258 KEY_DOWN
		259 KEY_UP
		260 KEY_LEFT
		261 KEY_RIGHT
	*/
	while(1) {
		int c = getch();
		switch(c) {
			case KEY_LEFT:
				/* we are not moving right at the moment */
				if (direction_x != 1 && direction_y != 0) {
					direction_x = -1;
					direction_y = 0;
				}
				break;
			case KEY_RIGHT:
				/* we are not moving right at the moment */
				if (direction_x != -1 && direction_y != 0) {
					direction_x = 1;
					direction_y = 0;
				}
				break;
			case KEY_UP:
				/* we are not moving right at the moment */
				if (direction_x != 0 && direction_y != 1) {
					direction_x = 0;
					direction_y = -1;
				}
				break;
			case KEY_DOWN:
				/* we are not moving right at the moment */
				if (direction_x != 0 && direction_y != -1) {
					direction_x = 0;
					direction_y = 1;
				}
				break;
		}

		/* 113 == 'q' */
		if (c == 113)
			break;
		if (c != -1 && c != color && c != KEY_LEFT && c != KEY_RIGHT && 
			c != KEY_UP && c != KEY_DOWN) color = c;
		attrset(COLOR_PAIR(color % 8));
		 
		/* process the command keystroke */
		(*x == max_x-1) ? (x_was_max = 1) : (x_was_max = 0);
		(*y == max_y-1) ? (y_was_max = 1) : (y_was_max = 0);

		/* Global var `stdscr` is created by the call to `initscr()` */
		getmaxyx(stdscr, max_y, max_x);
		
		/* Correct the position if window was shrinked */
		if (*x >= max_x) *x = max_x - 1;
		if (*y >= max_y) *y = max_y - 1;
		if (*x < 0) *x = 0;
		if (*y < 0) *y = 0;
		/* Correct position if window was enlarged */
		if (x_was_max && *x <= max_x-1) *x = max_x-1;
		if (y_was_max && *y <= max_y-1) *y = max_y-1;

        /* Clear the screen of all */
		clear();
		/* previously-printed characters */
		mvprintw(max_y/2 - 3, max_x/2, "Score: %i", score);
		mvprintw(max_y/2 - 2, max_x/2, "X: %i", *x);
		mvprintw(max_y/2 - 1, max_x/2, "Y: %i", *y);
		mvprintw(max_y/2, max_x/2, "dirX: %i", direction_x);
		mvprintw(max_y/2 + 1, max_x/2, "dirY: %i", direction_y);
		mvprintw(max_y/2 + 2, max_x/2, "MaxX: %i", max_x);
		mvprintw(max_y/2 + 3, max_x/2, "MaxY: %i", max_y);
		mvprintw(max_y/2 + 4, max_x/2, "Char: %i", c);
		mvprintw(max_y/2 + 5, max_x/2, "Len: %i", *snake_len);
        /* Print food at the current xy position */
		mvprintw(food_y, food_x, "o");
		snake_print(&snake);
		//mvprintw(*y, *x, "X"); // Print our "ball" at the current xy position
		refresh();

		usleep(DELAY); // Shorter delay between movements
		//x++; // Advance the ball to the right

		next_x = *x + direction_x;
		next_y = *y + direction_y;

		/* Will got the food */
		if (next_x == food_x && next_y == food_y) {
			caught = 1;
			score++;
		}
		
		snake_move(&snake, caught, direction_x, direction_y);

		/* Generate a new place for food */
		if (caught) {
			srand(time(NULL));
			food_x = rand() % max_x;
			food_y = rand() % max_y;
			caught = 0;
		}

	}

	/* Restore normal terminal behavior */
	endwin();
}

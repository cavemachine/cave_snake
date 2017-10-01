#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <curses.h>
#include <string.h>
#include <time.h> 

WINDOW * mainwindow;
WINDOW * frame;

bool apples[20][40];
bool controller_delay = false;
int score = 0;
int score_sub = 0;

char direction;
int frame_x;
int frame_y;

struct node {
    char data;
    int x;
    int y;
    struct node *next;
    struct node *previous;
};

struct node *head;

//----------------------------

void body_append(struct node *_head, char data) {
    /* - the snake is a linked-list starting with the head. 
       - find the last node of the snake, and the one before the last node
         to discover in which direction is the snake going; based on this,
         insert a new node in the correct position on the game frame and in the
	 last place of the linked-list. */
    struct node *tmp = malloc(sizeof(struct node));

    struct node *last;
    struct node *next_of_last;
    
    while ( _head->previous != NULL ) {
	_head = _head->previous;
    }

    tmp->data = data;
    tmp->next = _head;
    tmp->previous = NULL;
    _head->previous = tmp;

    last = _head;
    next_of_last = _head->next;

    if (last->x == next_of_last->x) {
	if (last->y < next_of_last->y) {
	    tmp->y = last->y - 1;
	    tmp->x = last->x;
	} else {
	    tmp->y = last->y + 1;
	    tmp->x = last->x;
	}
    } else {
	if (last->x < next_of_last->x) {
	    tmp->x = last->x - 1;
	    tmp->y = last->y;
	} else {
	    tmp->x = last->x + 1;
	    tmp->y = last->y;	    
	}
    }
}

void game_over() {
    mvwprintw(mainwindow, 10, 20, "GAME OVER");
    wrefresh(frame);
    sleep(1);
    endwin();
    printf("BYE\n");
    exit(0);
}

void random_apple() {
    int apple_x;
    int apple_y;

    int r = rand() % 10; // 10% of chance to generate an apple
    if (r == 1) {
	apple_y = rand() % 18 + 1;
	apple_x = rand() % 38 + 1;
	if (apples[apple_y][apple_x] == false) {
	    apples[apple_y][apple_x] = true;
	    mvwaddch(frame, apple_y, apple_x, 'M'); 
	}
    }
}

void print_all(struct node *_head) {
    /* - checks first if there is an apple at head position, if so
         grows snake body and add 20 to score. 
       - visits all items in the linked-list snake and add to frame.
       - check if the position of the visited item is equal the head position
         if so call game_over(). (the head touched the body);
       - also check if head touched any screen boarders. if so call game_over();
       - calculate the score, and eventually redraw frame borders in case it 
         get corrupted somehow, and print the score.
       - refresh mainwindow to show all the changes. */
    
    bool gameover = false;
    bool bodyappend = false;
    
    while (_head != NULL) {
	if (mvwinch(frame, _head->y, _head->x) == 'M') { 
	    apples[_head->y][_head->x] = false;
	    bodyappend = true;
	    score += 20;
	}
	mvwaddch(frame, _head->y, _head->x, _head->data);
	_head = _head->previous;	
      	if (_head != NULL && _head->x == head->x && _head->y == head->y) { 
	    gameover = true;
	}
    }
    wrefresh(frame);
    if (gameover == true) { game_over(); };
    if (bodyappend == true) { body_append(head, 'o'); }
    if (head->x == 0 || head->x == frame_x - 1 || head->y == 0 || head->y == frame_y - 1) {
	game_over();
    }
    controller_delay = false; 
    score_sub += 1;
    if (score_sub == 5) {
	score += 2;
	score_sub = 0;
	box(frame,0,0);
    }
    mvwprintw(mainwindow, 22, 3, "score: %i ", score);
    wrefresh(mainwindow);
}

void position_move() {
    /* erases the drawing of snake of last position and
       moves the last snake piece to one place ahead of
       the snake head; then call random_apples() to create apples and
       print_all() to print the snake in the new position to the frame */
    
    struct node *last;
    struct node *tmp;
    tmp = head;
    
    while ( tmp->previous != NULL ) {
	mvwaddch(frame, tmp->y, tmp->x, ' '); // erase old snake drawing
	tmp = tmp->previous;
    }
    mvwaddch(frame, tmp->y, tmp->x, ' ');
    
    last = tmp;
    last->next->previous = NULL;
    last->data = '+';
    
    if (direction == 'u') {
	last->x = head->x;
	last->y = head->y - 1;
    }
    if (direction == 'd') {
	last->x = head->x;
	last->y = head->y + 1;
    }
    if (direction == 'r') {
	last->x = head->x + 1;
	last->y = head->y;
    }
    if (direction == 'l') {
	last->x = head->x - 1;
	last->y = head->y;

    }
    
    last->next = NULL;
    last->previous = head;
    head->next = last;
    head->data = 'o';
    head = last;
    
    random_apple();
    print_all(head);  
}

void initialize_window() {
    mainwindow = initscr();
    frame_x = 40;
    frame_y = 20;
    frame = subwin(mainwindow, frame_y, frame_x, 2, 2);

    box(mainwindow, 0, 0);
    box(frame,0,0);

    noecho();
    cbreak();
    curs_set(0);    
}

void initialize_timer(int speed) {
    struct itimerval it;
    srand(time(NULL));
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = speed;
    it.it_interval = it.it_value;
    setitimer(ITIMER_REAL, &it, NULL);
    signal(SIGALRM,position_move);
}

void initialize_snake() {
    /* - creates the snake head, the first piece of body of the snake;
       - it's necessary at least the head, and one piece of body to call 
         append function.
       - finally appends 4 piece of body to it */
    head = malloc(sizeof(struct node));
    struct node *body_first = malloc(sizeof(struct node));
    
    direction = 'r';
    
    head->x = 8;
    head->y = 8;
    head->next = NULL;
    head->previous = body_first;
    head->data = 'o';

    body_first->x = 7;
    body_first->y = 8;
    body_first->next = head;
    body_first->previous = NULL;
    body_first->data = 'o';

    body_append(head, 'o');
    body_append(head, 'o');
    body_append(head, 'o');
    body_append(head, 'o');
}

void intro_menu() {
    
    mvwaddstr(mainwindow, 1, 3, "cave snake");
    mvwaddstr(frame, 3, 5, "n: new game");
    mvwaddstr(frame, 4, 5, "p: pause");
    mvwaddstr(frame, 5, 5, "q: quit");
    mvwaddstr(frame, 7, 5, "arrow keys: controller");
    mvwaddstr(mainwindow, 22, 3, "score: -");
    
    refresh();
    char ch;
    while (ch != 'n') {
	ch = getch();
	if (ch == 'q') {
	    endwin();
	    printf("BYE\n");
	    exit(0);
	}
	    
    }
    werase(frame);
    box(frame,0,0);

}


int main() {
    int timer_speed = 200000;
    bool paused = false;
    char direction_copy;
    
    initialize_window();
    
    intro_menu();

    initialize_snake();

    initialize_timer(timer_speed);

    char ch;
    while (ch != 'q' ) {
	ch = getch();

	if (ch  == 'p') {
	    if (paused == false) {
		direction_copy = direction;
		initialize_timer(0);
		paused = true;
	    } else {
		direction = direction_copy;
		initialize_timer(timer_speed);
		paused = false;
	    }
	}
	
	if (ch == '\033') {  // arrows: \033 (escape), '[', A B C or D.
	    getch();
	    switch ( getch() ) {
	    case 'A':
		if (direction != 'd' && controller_delay == false) {
		    direction = 'u';
		    controller_delay = true;
		}
		break;
	    case 'B':
		if (direction != 'u' && controller_delay == false) {
		    direction = 'd';
		    controller_delay = true;
		}
		break;
	    case 'C':
		if (direction != 'l' && controller_delay == false) {
		    direction = 'r';
		    controller_delay = true;
		}		
		break;
	    case 'D':
		if (direction != 'r' && controller_delay == false) {
		    direction = 'l';
		    controller_delay = true;
		}		
		break;
	    default:
		break;
	    }
	}

    }
    endwin();
}



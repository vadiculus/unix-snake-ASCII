#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ncurses.h>

#define MAX_BOARD_HEIGHT 24
#define MAX_BOARD_WIDTH 30
#define FPS_WANTED 5
#define TIME_OF_DRAWING 1000 / FPS_WANTED
#define MAX_APPLES 2
// #define UP_ARROW 65
// #define DOWN_ARROW 66
// #define RIGHT_ARROW 67
// #define LEFT_ARROW 68

int BOARD_HEIGHT = 0;
int BOARD_WIDTH  = 0;

typedef struct snake_block
{
  bool head;
  int current[2];
  int previous[2];
} snake_block;

typedef struct {
  int i;
  int j;
} empty_block;

typedef enum {
  UP,
  DOWN,
  LEFT,
  RIGHT
} Rotation;

int kbhit(void);
static void finish(int sig, char* str);
void draw(int score, char board[][BOARD_WIDTH]);
void movement(Rotation rotation, snake_block* snake, int length, char board[][BOARD_WIDTH]);
void generate_apple(int head_i, int head_j, char board[][BOARD_WIDTH]);
snake_block create_snake_block(int cur_i, int cur_j, int pre_i, int pre_j, bool is_head);
bool is_apple(Rotation rotation, int head_i, int head_j, char board[][BOARD_WIDTH]);
bool is_death(char board[][BOARD_WIDTH], snake_block *snake, int length);
bool is_win(char board[][BOARD_WIDTH]);
void change_rotation(int key, Rotation *rotation);

void main(int argc, char *argv[])
{

  if (argc == 2)
  {
    if (strcmp(argv[1], "sm") == 0)
    {
      BOARD_HEIGHT = 8;
      BOARD_WIDTH  = 10;
    }
    
    else if (strcmp(argv[1], "md") == 0 )
    {
      BOARD_HEIGHT = 8 * 2;
      BOARD_WIDTH  = 10 * 2;
    }

    else if (strcmp(argv[1], "bi") == 0)
    {
      BOARD_HEIGHT = 8 * 3;
      BOARD_WIDTH  = 10 * 3;
    }
  }
  else
  {
    printf("\nEROR! Wrong input. \nWaiting for one argument...\n");
    printf("\nsm -> small map; \nmd -> medium map; \nbi -> big map;\n\n");
    exit(-1);
  }

  srand(time(NULL));

  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);
  keypad(stdscr, TRUE);

  // Create board
  char board[BOARD_HEIGHT][BOARD_WIDTH];

  for (int i = 0; i < BOARD_HEIGHT; ++i)
    for (int j = 0; j < BOARD_WIDTH; ++j)
      board[i][j] = ' ';

  // Setup a snake  
  Rotation rotation = RIGHT;

  int avg_i = BOARD_HEIGHT/2;
  int avg_j = BOARD_WIDTH/2;
  snake_block head = create_snake_block(avg_i, avg_j, avg_i, avg_j-1, true);
  snake_block imaginary_block = create_snake_block(avg_i, avg_j-1, avg_i, avg_j-2, false);
  snake_block snake[MAX_BOARD_HEIGHT*MAX_BOARD_WIDTH] = {head, imaginary_block};

  board[avg_i][avg_j] = '$';

  // Create apples
  for (int i = 0; i < MAX_APPLES; ++i)
    generate_apple(snake[0].current[0], snake[0].current[1], board);

  int key = 0; // By default right arrow key
  int score = 0;
  int snake_length = 2; // 1 -> is head, 2 -> is void element
  int now, last_time;
  now = last_time = 0;
  int ms = 10000;

  while (true)
  {
    if ((now = clock())/ms - last_time/ms >= 100 / FPS_WANTED){ // <--- limit fps
      last_time = now;

      key = getch();
      flushinp();

      change_rotation(key, &rotation);
      movement(rotation, snake, snake_length, board);

      // check if next move on apple
      if (is_apple(rotation, snake[0].current[0], snake[0].current[1], board))
      {
        generate_apple(snake[0].current[0], snake[0].current[1], board);
        ++score;
        ++snake_length;
      }
      board[snake[0].current[0]][snake[0].current[1]] = '$';
      draw(score, board);
      refresh();

      if (is_death(board, snake, snake_length))
        finish(2, "\nFAIL. Your snake's dead...\n");

      if (is_win(board))
        finish(1, "\nIMPRESSIVE! You're WON!\n");
    }
  }
}

static void finish(int sig, char* str)
{
  endwin();
  system("clear");
  if (sig == 1)
  {
    printf(" _       __   ____   _   __   __\n");
    printf("| |     / /  /  _/  / | / /  / /\n");
    printf("| | /| / /   / /   /  |/ /  / / \n");
    printf("| |/ |/ /  _/ /   / /|  /  /_/  \n");
    printf("|__/|__/  /___/  /_/ |_/  (_)   \n");

    printf("\n%s\n\n", str);
  }
  if (sig == 2)
  {
    printf("    ______   ___       ____   __       ______   ____               \n");
    printf("   / ____/  /   |     /  _/  / /      / ____/  / __ \\             \n");
    printf("  / /_     / /| |     / /   / /      / __/    / / / /              \n");
    printf(" / __/    / ___ |   _/ /   / /___   / /___   / /_/ /  _    _    _  \n");
    printf("/_/      /_/  |_|  /___/  /_____/  /_____/  /_____/  (_)  (_)  (_) \n");

    printf("\n%s\n\n", str);
  }
  else
    printf("\n%s\n\n", str);

  exit(sig);
}

void draw(int score, char board[][BOARD_WIDTH])
{
  for (int i = 0; i < 50; ++i)
  {
    printw("\n");
  }
  printw("SCORE: %d\n", score);
  for (int i = 0; i < BOARD_WIDTH*2+2; ++i)
  {
    printw("-");
  }
  printw("\n");

  // Draw board background
  for (int i = 0; i < BOARD_HEIGHT; ++i)
  {
    printw("|");
    for (int j = 0; j < BOARD_WIDTH; ++j)
    {
      printw("%c ", board[i][j]);
    }
    printw("|");
    printw("\n");
  }

  for (int i = 0; i < BOARD_WIDTH*2+2; ++i)
  {
    printw("-");
  }
  printw("\n");
}

void movement(Rotation rotation, snake_block* snake, int length, char board[][BOARD_WIDTH])
{
  // copy snake
  snake_block snake_temp[length];

  for (int i = 0; i < length; i++)
  {
    snake_temp[i].head = false;
    snake_temp[i].current[0] = snake[i].current[0];
    snake_temp[i].current[1] = snake[i].current[1];
    snake_temp[i].previous[0] = snake[i].previous[0];
    snake_temp[i].previous[1] = snake[i].previous[1];
  }

  snake_temp[0].head = true;
  snake[0].previous[0] = snake_temp[0].current[0];
  snake[0].previous[1] = snake_temp[0].current[1];

  if (rotation == UP) // UP
  {
    if (snake[0].current[0] - 1 == -1)
      snake[0].current[0] += BOARD_HEIGHT-1;
    else
      snake[0].current[0] -= 1;
  }
  else if (rotation == DOWN)
  {
    if (snake[0].current[0] + 1 == BOARD_HEIGHT)
      snake[0].current[0] -= BOARD_HEIGHT-1;
    else
      snake[0].current[0] += 1;
  }
  else if (rotation == RIGHT)
  {
    if (snake[0].current[1] + 1 == BOARD_WIDTH)
      snake[0].current[1] -= BOARD_WIDTH-1;
    else
      snake[0].current[1] += 1;
  }
  else if (rotation == LEFT)
  {
    if (snake[0].current[1] - 1 == -1)
      snake[0].current[1] += BOARD_WIDTH-1;
    else
      snake[0].current[1] -= 1;
  }
  
  // move each block to 1
  int i = 1;
  
  while (i < length)
  {
    snake[i].current[0] = snake[i-1].previous[0];
    snake[i].current[1] = snake[i-1].previous[1];
    snake[i].previous[0] = snake_temp[i].current[0];
    snake[i].previous[1] = snake_temp[i].current[1];
    ++i;
  }

  // Draw snake
  for (int i = 1; i < length; ++i)
  {
    board[snake[i].current[0]][snake[i].current[1]] = '#';
  }

  board[snake[length-1].current[0]][snake[length-1].current[1]] = ' ';
}

void change_rotation(int key, Rotation *rotation){
  if (key == KEY_UP && *rotation != DOWN) // UP
    *rotation = UP;
  else if (key == KEY_DOWN && *rotation != UP)
    *rotation = DOWN;
  else if (key == KEY_LEFT && *rotation != RIGHT)
    *rotation = LEFT;
  else if (key == KEY_RIGHT && *rotation != LEFT)
    *rotation = RIGHT;
}

void generate_apple(int head_i, int head_j, char board[][BOARD_WIDTH])
{
  int apples_amount = 0;
  empty_block empt_blocks[BOARD_HEIGHT * BOARD_WIDTH];
  for (int i = 0; i < BOARD_HEIGHT; ++i){
    for (int j = 0; j < BOARD_WIDTH; ++j){
      if (board[i][j] == ' ' && (i != head_i && j != head_j)){
        empt_blocks[apples_amount].i = i;
        empt_blocks[apples_amount].j = j;
        apples_amount++;
      }
    }
  }

  int i = rand() % apples_amount;
  board[empt_blocks[i].i][empt_blocks[i].j] = '*';

}

snake_block create_snake_block(int cur_i, int cur_j, int pre_i, int pre_j, bool is_head)
{
  snake_block result;
  result.head = is_head;
  result.current[0] = cur_i;
  result.current[1] = cur_j;
  result.previous[0] = pre_i;
  result.previous[1] = pre_j;
  return result;
}

bool is_apple(Rotation rotation, int head_i, int head_j, char board[][BOARD_WIDTH])
{
  if (board[head_i][head_j] == '*'
  ) return true;
  
  return false;
}

bool is_death(char board[][BOARD_WIDTH], snake_block *snake, int length)
{
  for (int i = 1; i < length; i++){
    if (snake[0].current[0] == snake[i].current[0] && snake[0].current[1] == snake[i].current[1])
      return true;
  }
  return false;
}

bool is_win(char board[][BOARD_WIDTH])
{
  for (int i = 0; i < BOARD_HEIGHT; ++i)
    for (int j = 0; j < BOARD_WIDTH; ++j)
      if (board[i][j] == ' ')
        return false;

  return true;
}

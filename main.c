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
#define UP_ARROW 65
#define DOWN_ARROW 66
#define RIGHT_ARROW 67
#define LEFT_ARROW 68

int BOARD_HEIGHT = 0;
int BOARD_WIDTH  = 0;

typedef struct snake_block
{
  bool head;
  int current[2];
  int previous[2];
} snake_block;

int kbhit(void);
static void finish(int sig, char* str);
void draw(int score, char board[][BOARD_WIDTH]);
void movement(int key[2], snake_block* snake, int length, char board[][BOARD_WIDTH]);
void generate_apple(char board[][BOARD_WIDTH]);
snake_block create_snake_block(int cur_i, int cur_j, int pre_i, int pre_j, bool is_head);
bool is_apple(int head_i, int head_j, char board[][BOARD_WIDTH], int key);
bool is_death(int head_i, int head_j, char board[][BOARD_WIDTH]);
bool is_win(char board[][BOARD_WIDTH]);

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

  // Create board
  char board[BOARD_HEIGHT][BOARD_WIDTH];

  for (int i = 0; i < BOARD_HEIGHT; ++i)
    for (int j = 0; j < BOARD_WIDTH; ++j)
      board[i][j] = ' ';

  // Create apples
  for (int i = 0; i < MAX_APPLES; ++i)
    generate_apple(board);

  // Setup a snake  
  int avg_i = BOARD_HEIGHT/2;
  int avg_j = BOARD_WIDTH/2;
  snake_block head = create_snake_block(avg_i, avg_j, avg_i, avg_j-1, true);
  snake_block imaginary_block = create_snake_block(avg_i, avg_j-1, avg_i, avg_j-2, false);
  snake_block snake[MAX_BOARD_HEIGHT*MAX_BOARD_WIDTH] = {head, imaginary_block};

  board[avg_i][avg_j] = '$';

  int key[2] = {RIGHT_ARROW, RIGHT_ARROW}; // By default right arrow key
  int score = 0;
  int snake_length = 2; // 1 -> is head, 2 -> is void element

  while (true)
  {
    if (kbhit())
    {
      // get input key
      getch();
      getch();

      key[1] = key[0];
      key[0] = getch();

      // check if next move on apple
      if (is_apple(snake[0].current[0], snake[0].current[1], board, key[0]))
      {
        generate_apple(board);
        ++score;
        ++snake_length;
      }
    }
    else
    {
      movement(key, snake, snake_length, board);
      draw(score, board);
      refresh();
      timeout(TIME_OF_DRAWING);

      if (is_apple(snake[0].current[0], snake[0].current[1], board, key[0]))
      {
        generate_apple(board);
        ++score;
        ++snake_length;
      }

      if (is_death(snake[0].current[0], snake[0].current[1], board))
        finish(2, "\nFAIL. Your snake's dead...\n");

      if (is_win(board))
        finish(1, "\nIMPRESSIVE! You're WON!\n");
    }
  }
}

int kbhit(void)
{
  int ch = getch();

  if (ch != ERR)
  {
    ungetch(ch);
    return 1;
  }
  else
    return 0;
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

void movement(int key[2], snake_block* snake, int length, char board[][BOARD_WIDTH])
{
  // copy snake
  snake_block snake_temp[length];
  for (int i = 0; i < length; ++i)
  {
    snake_temp[i].head = false;
    snake_temp[i].current[0] = snake[i].current[0];
    snake_temp[i].current[1] = snake[i].current[1];
    snake_temp[i].previous[0] = snake[i].previous[0];
    snake_temp[i].previous[1] = snake[i].previous[1];
    if (i == 0)
      snake_temp[i].head = true;
  }
  // move each block to 1
  int i = 0;
  while (i < length)
  {
    if (snake[i].head == true)
    {
      snake[i].previous[0] = snake_temp[i].current[0];
      snake[i].previous[1] = snake_temp[i].current[1];

      if ((key[0] == UP_ARROW && key[1] == DOWN_ARROW) || key[0] == UP_ARROW) // UP
      {
        if (snake[i].current[0] - 1 == -1)
          snake[i].current[0] += BOARD_HEIGHT-1;
        else
          snake[i].current[0] -= 1;
      }
      else if ((key[0] == DOWN_ARROW && key[1] == UP_ARROW) || key[0] == DOWN_ARROW)
      {
        if (snake[i].current[0] + 1 == BOARD_HEIGHT)
          snake[i].current[0] -= BOARD_HEIGHT-1;
        else
          snake[i].current[0] += 1;
      }
      else if ((key[0] == RIGHT_ARROW && key[1] == LEFT_ARROW) || key[0] == RIGHT_ARROW)
      {
        if (snake[i].current[1] + 1 == BOARD_WIDTH)
          snake[i].current[1] -= BOARD_WIDTH-1;
        else
          snake[i].current[1] += 1;
      }
      else if ((key[0] == LEFT_ARROW && key[1] == RIGHT_ARROW) || key[0] == LEFT_ARROW)
      {
        if (snake[i].current[1] - 1 == -1)
          snake[i].current[1] += BOARD_WIDTH-1;
        else
          snake[i].current[1] -= 1;
      }

      ++i;
      continue;
    }
    snake[i].current[0] = snake[i-1].previous[0];
    snake[i].current[1] = snake[i-1].previous[1];
    snake[i].previous[0] = snake_temp[i].current[0];
    snake[i].previous[1] = snake_temp[i].current[1];
    ++i;
  }

  // Draw snake
  for (int i = 0; i < length; ++i)
  {
    if (i == 0)
      board[snake[i].current[0]][snake[i].current[1]] = '$';
    
    else if (i == length-1)
      board[snake[i].current[0]][snake[i].current[1]] = ' ';
    else
      board[snake[i].current[0]][snake[i].current[1]] = '#';
  }
}

void generate_apple(char board[][BOARD_WIDTH])
{
  int apples_amount = 0;
  for (int i = 0; i < BOARD_HEIGHT; ++i)
    for (int j = 0; j < BOARD_WIDTH; ++j)
      if (board[i][j] == '*')
        ++apples_amount;
  
  if (apples_amount <= MAX_APPLES)
  {
    int i = rand() % BOARD_HEIGHT;
    int j = rand() % BOARD_WIDTH;
  
    if (board[i][j] != ' ')
      generate_apple(board);
    else
      board[i][j] = '*';
  }
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

bool is_apple(int head_i, int head_j, char board[][BOARD_WIDTH], int key)
{
  if (
    (key == UP_ARROW && board[head_i-1][head_j] == '*') ||
    (key == DOWN_ARROW && board[head_i+1][head_j] == '*') ||
    (key == RIGHT_ARROW && board[head_i][head_j+1] == '*') || 
    (key == LEFT_ARROW && board[head_i][head_j-1] == '*') 
  ) return true;

  return false;
}

bool is_death(int head_i, int head_j, char board[][BOARD_WIDTH])
{
  if (board[head_i][head_j] == '#')
    return true;
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

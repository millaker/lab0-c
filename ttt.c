#include "ttt.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "game.h"
#include "hlist.h"
#include "list.h"
#include "mcts.h"
#include "negamax.h"
#include "task_sched.h"

static int move_record[N_GRIDS];
static int move_count = 0;

static int bot0_won;
static int bot1_won;
static int drawed;
static int sim_count;
static bool sim_active;
static bool sim_freeze_board;
static struct termios orig_term;
static char sim_table[N_GRIDS];
static int (*bot_records)[N_GRIDS];
static int *bot_move_counts;

typedef struct {
    char buf[1024];
    int len;
} screen_buf;

static void disable_rawmode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
}

static void enable_rawmode()
{
    tcgetattr(STDIN_FILENO, &orig_term);

    struct termios raw = orig_term;
    raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void buf_free(screen_buf *sb)
{
    sb->len = 0;
}

static void buf_append(screen_buf *sb, const char *s, int len)
{
    memcpy(&sb->buf[sb->len], s, len);
    sb->len += len;
}

static void draw_bot_board(screen_buf *sb, char *t)
{
    char result[1024] = {0};
    char buf[32];
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (BOARD_SIZE < 10) {
            snprintf(buf, 6, "%2d | ", i + 1);
            strncat(result, buf, 6);
        } else if (BOARD_SIZE >= 10 && BOARD_SIZE < 100) {
            snprintf(buf, 7, "%3d | ", i + 1);
            strncat(result, buf, 7);
        } else {
            snprintf(buf, 8, "%4d | ", i + 1);
            strncat(result, buf, 8);
        }

        for (int j = 0; j < BOARD_SIZE; j++) {
            // make background color alter between high-intensity and standard
            if ((i + j) & 1U) {
                strncat(result, "\x1b[47m", 5);
            } else {
                strncat(result, "\x1b[107m", 6);
            }
            switch (t[GET_INDEX(i, j)]) {
            case 'O':
                strncat(result, "\x1b[31m ○ \x1b[39m", 16);
                break;
            case 'X':
                strncat(result, "\x1b[34m × \x1b[39m", 15);
                break;
            default:
                strncat(result, "   ", 4);
                break;
            }
            strncat(result, "\x1b[49m", 6);
        }
        strncat(result, "\n", 2);
    }
    if (BOARD_SIZE >= 10) {
        strncat(result, "-", 2);
    }
    if (BOARD_SIZE >= 100) {
        strncat(result, "-", 2);
    }
    strncat(result, "---+-", 6);
    for (int i = 0; i < BOARD_SIZE; i++) {
        strncat(result, "---", 4);
    }
    strncat(result, "\n", 2);
    if (BOARD_SIZE >= 10) {
        strncat(result, " ", 2);
    }
    if (BOARD_SIZE >= 100) {
        strncat(result, " ", 2);
    }
    strncat(result, "    ", 5);
    for (int i = 0; i < BOARD_SIZE; i++) {
        snprintf(buf, 32, " %2c", 'A' + i);
        strncat(result, buf, 8);
    }
    strncat(result, "\n", 2);
    buf_append(sb, result, strlen(result));
}

static void draw_time(screen_buf *sb)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[12];
    snprintf(buf, 10, "%02d:%02d:%02d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
    buf_append(sb, buf, 9);
}

static void draw_sim_count(screen_buf *sb)
{
    char buf[20];
    snprintf(buf, 20, "Simulations: %5d\n", sim_count);
    buf_append(sb, buf, 19);
}

static const char *load_icon[] = {"⣼", "⣹", "⢻", "⠿", "⡟", "⣏", "⣧", "⣶"};
static unsigned load_idx = 0;
static void draw_loading(screen_buf *sb, bool flag)
{
    char b[16] = {0};
    snprintf(b, 16, "%s\n", load_icon[load_idx & 0x7]);
    buf_append(sb, b, strlen(b));
    if (flag) {
        load_idx++;
    }
}

static void draw_command(screen_buf *sb)
{
    buf_append(sb, "\x1b[30;100m^Q\x1b[m Quit ", 20);
    buf_append(sb, "\x1b[30;100m^P\x1b[m ", 15);
    if (sim_freeze_board) {
        buf_append(sb, "Unlock", 6);
    } else {
        buf_append(sb, "Lock", 4);
    }
    buf_append(sb, " Board   \n", 10);
}

static void draw_stats(screen_buf *sb)
{
    char buf[28];
    snprintf(buf, 28, "Bot0: %2d Draw: %2d Bot2: %2d\n", bot0_won, drawed,
             bot1_won);
    buf_append(sb, buf, 27);
}

static void delta_sleep(__time_t sec, __time_t nsec)
{
    struct timespec req = {sec, nsec};
    struct timespec rem = {0, 0};
    int nt = nanosleep(&req, &rem);
    while (nt == -1) {
        req.tv_sec = rem.tv_sec;
        req.tv_nsec = rem.tv_nsec;
        nt = nanosleep(&req, &rem);
    }
}

static unsigned quarter_time = 0;
void refresh_screen(char *table)
{
    while (sim_active) {
        screen_buf sb = {{0}, 0};
        buf_append(&sb, "\x1b[H", 3);
        draw_time(&sb);
        if (sim_freeze_board) {
            // Skip board section
            buf_append(&sb, "\x1b[6B", 4);
        } else {
            draw_bot_board(&sb, table);
        }
        draw_sim_count(&sb);
        draw_stats(&sb);
        draw_loading(&sb, !(quarter_time++ & 0xf));
        draw_command(&sb);
        size_t ret = write(STDIN_FILENO, sb.buf, sb.len);
        while (ret != sb.len) {
            ret += write(STDIN_FILENO, &sb.buf[ret], sb.len - ret);
        }
        buf_free(&sb);
        delta_sleep(0, 1000000);
    }
}

#define CTRL_KEY(x) ((x) & (0x1f))
static void process_key()
{
    while (sim_active) {
        int nread;
        char c = '\0';
        if ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
            continue;
        }
        switch (c) {
        case CTRL_KEY('q'):
            sim_active = false;
            printf("Cleaning up...\n");
            break;
        case CTRL_KEY('p'):
            sim_freeze_board = !sim_freeze_board;
            break;
        default:
            // Simply ignore other keystrokes
            break;
        }
    }
}

static void record_move(int move)
{
    move_record[move_count++] = move;
}

static void print_moves()
{
    printf("Moves: ");
    for (int i = 0; i < move_count; i++) {
        printf("%c%d", 'A' + GET_COL(move_record[i]),
               1 + GET_ROW(move_record[i]));
        if (i < move_count - 1) {
            printf(" -> ");
        }
    }
    printf("\n");
}

static int get_input(char player)
{
    char *line = NULL;
    size_t line_length = 0;
    int parseX = 1;

    int x = -1, y = -1;
    while (x < 0 || x > (BOARD_SIZE - 1) || y < 0 || y > (BOARD_SIZE - 1)) {
        printf("%c> ", player);
        int r = getline(&line, &line_length, stdin);
        if (r == -1)
            exit(1);
        if (r < 2)
            continue;
        x = 0;
        y = 0;
        parseX = 1;
        for (int i = 0; i < (r - 1); i++) {
            if (isalpha(line[i]) && parseX) {
                x = x * 26 + (tolower(line[i]) - 'a' + 1);
                if (x > BOARD_SIZE) {
                    // could be any value in [BOARD_SIZE + 1, INT_MAX]
                    x = BOARD_SIZE + 1;
                    printf("Invalid operation: index exceeds board size\n");
                    break;
                }
                continue;
            }
            // input does not have leading alphabets
            if (x == 0) {
                printf("Invalid operation: No leading alphabet\n");
                y = 0;
                break;
            }
            parseX = 0;
            if (isdigit(line[i])) {
                y = y * 10 + line[i] - '0';
                if (y > BOARD_SIZE) {
                    // could be any value in [BOARD_SIZE + 1, INT_MAX]
                    y = BOARD_SIZE + 1;
                    printf("Invalid operation: index exceeds board size\n");
                    break;
                }
                continue;
            }
            // any other character is invalid
            // any non-digit char during digit parsing is invalid
            // TODO: Error message could be better by separating these two cases
            printf("Invalid operation\n");
            x = y = 0;
            break;
        }
        x -= 1;
        y -= 1;
    }
    free(line);
    return GET_INDEX(y, x);
}

static void bot_print_move()
{
    for (int i = 0; i < sim_count; i++) {
        printf("Moves: ");
        for (int j = 0; j < bot_move_counts[i]; j++) {
            printf("%c%d", 'A' + GET_COL(bot_records[i][j]),
                   1 + GET_ROW(bot_records[i][j]));
            if (j < bot_move_counts[i] - 1) {
                printf(" -> ");
            }
        }
        printf("\n");
    }
}

static void bot_record_move(int move)
{
    bot_records[sim_count][bot_move_counts[sim_count]++] = move;
}

static char bot_game(char *table, char bot0, char bot1)
{
    char turn = 'X';
    while (1) {
        if (turn == bot0) {
            int move = mcts(table, bot0);
            if (move != -1) {
                table[move] = bot0;
                bot_record_move(move);
            }
        } else {
            int move = negamax_predict(table, bot1).move;
            if (move != -1) {
                table[move] = bot1;
                bot_record_move(move);
            }
        }
        char win = check_win(table);
        if (win == 'D') {
            return ' ';
        } else if (win != ' ') {
            return turn;
        }
        turn = turn == 'X' ? 'O' : 'X';
    }
}

static void bot_swap(char *a, char *b)
{
    char temp = *a;
    *a = *b;
    *b = temp;
}

void bot_entry(void *arg)
{
    int limit = *(int *) arg;
    char bot0 = 'X';
    char bot1 = 'O';

    while (limit > 0 && sim_active) {
        negamax_init();
        memset(sim_table, ' ', N_GRIDS);
        char result = bot_game(sim_table, bot0, bot1);
        if (result == bot0) {
            bot0_won++;
        } else if (result == bot1) {
            bot1_won++;
        } else {
            drawed++;
        }
        negamax_free();
        sim_count++;
        delta_sleep(1, 0);
        limit--;
        bot_swap(&bot0, &bot1);
    }
    sim_active = false;
}

static void bot_init(int limit)
{
    bot_records = malloc(sizeof(int[N_GRIDS]) * limit);
    bot_move_counts = calloc(limit, sizeof(int));
    sim_count = 0;
    bot0_won = 0;
    bot1_won = 0;
    drawed = 0;
    sim_active = true;
    sim_freeze_board = false;
    printf("\x1b[2J\x1b[H\x1b[?25l\n");

    task_init();
    task_add(bot_entry, &limit);
    task_add(process_key, NULL);
    task_add((task_callback_t *) refresh_screen, sim_table);
}

static void bot_free()
{
    free(bot_move_counts);
    free(bot_records);
    bot_move_counts = NULL;
    bot_records = NULL;
}

void human_game()
{
    move_count = 0;
    char table[N_GRIDS];
    memset(table, ' ', N_GRIDS);
    char turn = 'X';
    char ai = 'O';

    while (1) {
        char win = check_win(table);
        if (win == 'D') {
            draw_board(table);
            printf("It is a draw!\n");
            break;
        } else if (win != ' ') {
            draw_board(table);
            printf("%c won!\n", win);
            break;
        }

        if (turn == ai) {
            int move = mcts(table, ai);
            if (move != -1) {
                table[move] = ai;
                record_move(move);
            }
        } else {
            draw_board(table);
            int move;
            while (1) {
                move = get_input(turn);
                if (table[move] == ' ') {
                    break;
                }
                printf("Invalid operation: the position has been marked\n");
            }
            table[move] = turn;
            record_move(move);
        }
        turn = turn == 'X' ? 'O' : 'X';
    }
    print_moves();
}

bool ttt_entry(int mode, int limit)
{
    if (mode == 0) {
        human_game();
    } else {
        bot_init(limit);
        enable_rawmode();
        task_start();
        disable_rawmode();
        bot_print_move();
        bot_free();
        printf("Simulation done\x1b[?25h\n");
    }
    return true;
}

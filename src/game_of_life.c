#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define ROWS 25
#define COLS 80
#define ALIVE_CELL '#'
#define DEAD_CELL ' '

// Проверка клетки соседей
void cell_checker(char cell[ROWS][COLS], int i, int j, int numberOfNeighbors, char tempmap[ROWS][COLS]);
// Задает стандартное значение для второго массива
void default_value(char map[ROWS][COLS], char tempMap[ROWS][COLS]);
// Передача изменения из временного массива в основной массив
void switch_map(char map[ROWS][COLS], char tempmap[ROWS][COLS]);
// Отрисовка карты
void render_map(char map[ROWS][COLS], int counter, int delay);
// Основная логика программы
void game(char map[ROWS][COLS], char tempmap[ROWS][COLS], int *neighborsCount, int *flag, int *delay,
          int *counter);
// Подсчет количества соседей
int neighbors_count(char cell[ROWS][COLS], int i, int j);
// Вывод информации о скорости и поколении эволюции
void print_info(int counter, int delay);
// Чтение карты
void input(char map[ROWS][COLS]);
// Вывод информации об управлении
void render_map_description();
// Выводит вертикального ограничителя карты
void print_vertical_frame(char map[ROWS][COLS]);
// Выводит горизонтального ограничителя карты
void print_horizontal_frame();
// Обработка ввода символа
int kbhit(void);

int main() {
    char map[ROWS][COLS];
    char tempMap[ROWS][COLS];
    int neighborsCount = 0;
    int flag = 1;
    int delay = 100000;
    int counter = 0;

    game(map, tempMap, &neighborsCount, &flag, &delay, &counter);

    return 0;
}

void game(char map[ROWS][COLS], char tempmap[ROWS][COLS], int *neighborsCount, int *flag, int *delay,
          int *counter) {
    input(map);
    if (freopen("/dev/tty", "r", stdin) != NULL) {
        default_value(map, tempmap);
        while (*flag) {
            if (*counter == 500) *flag = 0;
            render_map(map, *counter, *delay);
            if (kbhit()) {
                char c = getchar();
                if (c == 'k' && *delay > 50000) *delay -= 10000;
                if (c == 'm' && *delay < 200000) *delay += 10000;
                if (c == 'q') *flag = 0;
            }
            usleep(*delay);
            int i = 0;
            while (i < ROWS) {
                int j = 0;
                while (j < COLS) {
                    *neighborsCount = neighbors_count(map, i, j);
                    cell_checker(map, i, j, *neighborsCount, tempmap);
                    j++;
                }
                i++;
            }
            switch_map(map, tempmap);
            (*counter)++;
        }
    } else {
        printf("n/a\n");
    }
}

void input(char map[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            map[i][j] = getchar();
        }
        getchar();
    }
}

void render_map(char map[ROWS][COLS], int counter, int delay) {
    system("clear");

    print_horizontal_frame(map);
    print_vertical_frame(map);
    print_horizontal_frame(map);

    print_info(counter, delay);
    render_map_description();
}

void render_map_description() { printf("print q for exit\n-- k for up speed--\n-- m for down speed\n"); }

void cell_checker(char cell[ROWS][COLS], int i, int j, int numberOfNeighbors, char tempmap[ROWS][COLS]) {
    if (cell[i][j] == DEAD_CELL && numberOfNeighbors == 3)
        tempmap[i][j] = ALIVE_CELL;
    else if (cell[i][j] == ALIVE_CELL && (numberOfNeighbors == 2 || numberOfNeighbors == 3))
        tempmap[i][j] = ALIVE_CELL;
    else
        tempmap[i][j] = DEAD_CELL;
}

int neighbors_count(char cell[ROWS][COLS], int i, int j) {
    int count = 0;
    if (cell[i][j] == ALIVE_CELL) count--;
    int currentK;
    int currentL;

    for (int k = i - 1; k < i + 2; ++k) {
        for (int l = j - 1; l < j + 2; ++l) {
            currentK = k;
            currentL = l;

            if (currentK == -1) currentK += 25;
            if (currentK == 25) currentK -= 25;
            if (currentL == -1) currentL += 80;
            if (currentL == 80) currentL -= 80;
            if (cell[currentK][currentL] == ALIVE_CELL) count++;
        }
    }

    return count;
}

void switch_map(char map[ROWS][COLS], char tempMap[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++) map[i][j] = tempMap[i][j];
}

void default_value(char map[ROWS][COLS], char tempMap[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++) tempMap[i][j] = map[i][j];
}

int kbhit(void) {
    int flag = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        flag = 1;
    }

    return flag;
}

void print_info(int counter, int delay) {
    if (delay == 200000) {
        delay = 1;
    } else if (delay == 100000) {
        delay = 100;
    } else if (delay > 100000) {
        delay /= 1000;
        delay = 100 - (delay % 100);
    } else {
        delay /= 1000;
        delay = (100 - (delay % 100)) + 100;
    }

    printf("\nspeed: %d%% age: %d / 500\n", delay, counter);
}

void print_horizontal_frame() {
    printf("-");
    for (int i = 1; i <= COLS; i++) printf("-");
    printf("-\n");
}

void print_vertical_frame(char map[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++) {
        printf("|");
        for (int j = 0; j < COLS; j++) {
            printf("%c", map[i][j]);
        }
        printf("|\n");
    }
}

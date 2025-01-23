#ifndef S21_CAT_H
#define S21_CAT_H
#define _POSIX_C_SOURCE 200809L
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE
#define FILE_BUFFER_MAX 4096

// Определяем короткие опции для getopt
const char *short_options = "+beEnstTv";

// Определяем длинные опции для getopt
const struct option long_options[] = {
    {"number-nonblank", no_argument, NULL, 'n'},
    {"show-ends", no_argument, NULL, 'E'},
    {"number", no_argument, NULL, 'n'},
    {"squeeze-blank", no_argument, NULL, 's'},
    {"show-tabs", no_argument, NULL, 'T'},
    {"show-nonprinting", no_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}  // Завершающий элемент
};

// Структура для хранения опций
typedef struct {
  int b;  // Опция для нумерации ненулевых строк
  int n;  // Опция для нумерации всех строк
  int s;  // Опция для сжатия пустых строк
  int t;  // Опция для отображения табуляций
  int v;  // Опция для отображения непечатаемых символов
  int tt;  // Дополнительная опция (если нужна)
  int ee;  // Дополнительная опция (если нужна)
  int e;   // Дополнительная опция (если нужна)
  int print_ch_done;
} options;

// Прототипы функций
void parser(int argc, char *argv[], options *opt);
void reader(char *argv[], int argc, options *opt);
void handle_v_flag(int current_symbol, options *opt);

#endif  // S21_CAT_H
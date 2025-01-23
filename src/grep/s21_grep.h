#ifndef S21_CAT_H
#define S21_CAT_H
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE  // getline
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Определяем короткие опции для getopt

// Структура для хранения опций
typedef struct {
  char *pattern;
  size_t size;
  int regex_flag;
  bool invert;
  bool count;
  bool filesMatch;
  bool numberLine;
  bool PrintMatched;
  bool h;
  bool s;
  bool f;              // Добавьте это поле для флага -f
  char *pattern_file;  // Добавьте поле для хранения имени файла с шаблонами
} Flags;
// Прототипы функций
char *string_append_expr(char *string, size_t *size, char const *expr,
                         size_t size_expr);
void grepFile(FILE *file, char const *filename, Flags flags, regex_t *preg,
              int file_count);
int grep(int argc, char *argv[], Flags flag);
Flags GrepReadFlags(int argc, char *argv[]);
void GrepCount(FILE *file, char const *filename, Flags flags, regex_t *preg,
               int file_count);
void processLine(char const *filename, Flags flags, regex_t *preg, char *line,
                 int count, int argc);
#endif  // S21_CAT_H
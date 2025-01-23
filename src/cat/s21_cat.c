#include "s21_cat.h"

int main(int argc, char *argv[]) {
  options options = {0};
  parser(argc, argv, &options);
  if (options.b) options.n = 0;
  reader(argv, argc, &options);
};

void parser(int argc, char *argv[], options *opt) {
  int option_switch_case;
  int option_index;
  while ((option_switch_case = getopt_long(
              argc, argv, short_options, long_options, &option_index)) != -1) {
    switch (option_switch_case) {
        // short_options = "+beEnstTv"
      case 'b':
        opt->b = 1;
        break;
      case 'n':
        opt->n = 1;
        break;
      case 's':
        opt->s = 1;
        break;
      case 'e':
        opt->e = 1;
        opt->v = 1;
        break;
      case 't':
        opt->t = 1;
        opt->v = 1;
        break;
      case 'T':
        opt->tt = 1;
        break;
      case 'E':
        opt->ee = 1;
        break;
      case 'v':
        opt->v = 1;
        break;
      default:
        fprintf(stderr, "usage: cat [-benstuv] [file ...]\n");
        exit(1);
    }
  }
}

void handle_v_flag(int current_symbol, options *opt) {
  if (opt->e && current_symbol == '\n') {
    printf("$");
  }
  if (opt->t && current_symbol == '\t') {
    printf("^");
    current_symbol = 'I';
  }
  if ((current_symbol >= 0 && current_symbol <= 8) ||
      (current_symbol >= 11 && current_symbol <= 31)) {
    printf("^%c", current_symbol + 64);
  } else if (current_symbol == 127) {
    printf("^?");
  } else if (current_symbol >= 128 && current_symbol <= 159) {
    printf("M-^%c", current_symbol - 64);
  } else if (current_symbol >= 160 && current_symbol <= 254) {
    printf("M-%c", current_symbol - 128);
  }
}

void reader(char *argv[], int argc, options *opt) {
  for (int i = optind; i < argc; i++) {  // Обрабатываем все файлы
    FILE *f = fopen(argv[i], "r");
    if (f) {
      int current_symbol;
      int str_count = 0;
      char previous_character =
          '\n';  // Считаем, что предыдущая строка была пустой
      int blank_strings_counter = 0;

      while ((current_symbol = fgetc(f)) != EOF) {
        if (current_symbol == '\n' && previous_character == '\n') {
          blank_strings_counter++;
        } else {
          blank_strings_counter = 0;
        }
        if (opt->s && blank_strings_counter > 1) continue;

        if (previous_character == '\n' &&
            (opt->n || (opt->b && current_symbol != '\n'))) {
          printf("%6d\t", ++str_count);
        }
        if (opt->tt && current_symbol == '\t') {
          printf("^");
          current_symbol = 'I';
        }
        if (opt->ee && current_symbol == '\n') {
          printf("$");
        }
        if (opt->v) {
          handle_v_flag(current_symbol, opt);
        }
        putchar(current_symbol);
        previous_character = current_symbol;
      }
      fclose(f);  // Закрываем файл после завершения чтения
    } else {
      fprintf(stderr, "No such file: %s\n",
              argv[i]);  // Сообщаем об ошибке с файлом
    }
  }
}
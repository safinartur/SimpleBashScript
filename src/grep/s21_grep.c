#include "s21_grep.h"

int main(int argc, char *argv[]) {
  Flags flags = GrepReadFlags(argc, argv);
  argv += optind;
  argc -= optind;
  if (argc == 0) {
    fprintf(stderr, "no pattern\n");
    exit(1);
  }
  grep(argc, argv, flags);
};

void GrepCount(FILE *file, char const *filename, Flags flags, regex_t *preg,
               int argc) {
  (void)flags;
  (void)filename;
  char *line = 0;
  size_t length = 0;
  regmatch_t match;
  int count = 0;
  int count_inverse = 0;
  while (getline(&line, &length, file) > 0) {
    if (!regexec(preg, line, 1, &match, 0)) {
      if (!flags.invert) ++count;
    } else
      count_inverse++;
  }
  if (!flags.invert) {
    if (argc == 2 || (flags.h && argc > 2))
      printf("%i\n", count);
    else
      printf("%s:%i\n", filename, count);
  } else {
    if (argc == 2 || (flags.h && argc > 2))
      printf("%i\n", count_inverse);
    else
      printf("%s:%i\n", filename, count_inverse);
  }
  free(line);
}

void processLine(char const *filename, Flags flags, regex_t *preg, char *line,
                 int count, int argc) {
  regmatch_t match;

  if (!regexec(preg, line, 1, &match, 0) && !flags.filesMatch) {
    if (flags.PrintMatched || flags.numberLine) {
      if (flags.numberLine && !flags.PrintMatched) {
        if (argc == 2 || (flags.h && argc > 2))
          printf("%i:%s", count, line);
        else
          printf("%s:%i:%s", filename, count, line);
      } else {
        if (argc == 2 || (flags.h && argc > 2)) {
          printf("%.*s\n", (int)(match.rm_eo - match.rm_so),
                 line + match.rm_so);
          char *remaining = line + match.rm_eo;
          while (regexec(preg, remaining, 1, &match, 0) == 0) {
            printf("%.*s\n", (int)(match.rm_eo - match.rm_so),
                   remaining + match.rm_so);
            remaining += match.rm_eo;
          }
        } else {
          printf("%s:%.*s\n", filename, (int)(match.rm_eo - match.rm_so),
                 line + match.rm_so);
          char *remaining = line + match.rm_eo;
          while (!regexec(preg, remaining, 1, &match, 0)) {
            printf("%s:%.*s\n", filename, (int)(match.rm_eo - match.rm_so),
                   remaining + match.rm_so);
            remaining += match.rm_eo;
          }
        }
      }
    } else {
      if (argc == 1 || (flags.h && argc > 2))
        printf("%s", line);
      else
        printf("%s:%s", filename, line);
    }
  } else {
    if (flags.filesMatch) {
      if (!regexec(preg, line, 1, &match, 0)) {
        printf("%s\n", filename);
      }
    }
  }
}

void grepFile(FILE *file, char const *filename, Flags flags, regex_t *preg,
              int argc) {
  (void)flags;
  char *line = 0;
  size_t length = 0;
  regmatch_t match;

  int count = 0;

  while (getline(&line, &length, file) > 0) {
    count++;

    if (flags.invert && !flags.filesMatch) {
      if (regexec(preg, line, 1, &match, 0)) {
        if (flags.PrintMatched)
          ;
        else {
          if (flags.numberLine) {
            if (argc == 2 || (flags.h && argc > 2))
              printf("%i:%s", count, line);
            else
              printf("%s:%i:%s", filename, count, line);
          } else {
            if (argc == 2 || (flags.h && argc > 2))
              printf("%s", line);
            else {
              printf("%s:%s", filename, line);
            }
          }
        }
      }
    } else {  // если не v
      processLine(filename, flags, preg, line, count, argc);
    }
  }
  free(line);
}

int grep(int argc, char *argv[], Flags flags) {
  char **end = &argv[argc];
  regex_t preg_storage;
  regex_t *preg = &preg_storage;
  int error = 0;

  // Если указан файл с шаблонами, читаем его
  if (flags.pattern_file) {
    FILE *pattern_file = fopen(flags.pattern_file, "r");
    if (!pattern_file) {
      perror("Could not open pattern file");
      return 1;
    }
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, pattern_file) != -1) {
      line[strcspn(line, "\n")] = 0;  // Удаляем символ новой строки
      flags.pattern =
          string_append_expr(flags.pattern, &flags.size, line, strlen(line));
    }
    free(line);
    fclose(pattern_file);
  }

  // Компиляция регулярного выражения
  if (flags.size == 0) {
    if (regcomp(preg, argv[0], flags.regex_flag)) {
      fprintf(stderr, "failed to compile regex\n");
      error = 1;
    }
  } else {
    if (regcomp(preg, flags.pattern + 2, flags.regex_flag)) {
      fprintf(stderr, "failed to compile regex\n");
      error = 1;
    }
  }
  free(flags.pattern);
  if (argc == (flags.size ? 2 : 1) && !flags.s && !flags.pattern) {
    fprintf(stderr, "no file\n");
    error = 1;
  }
  for (char **filename = argv + (flags.size ? 0 : 1); filename != end;
       ++filename) {
    FILE *file = fopen(*filename, "rb");
    if (errno && !flags.s) {
      perror(*filename);
      continue;
    }
    if (errno && flags.s) {
      error = 1;
      break;
    }
    if (flags.count) {
      GrepCount(file, *filename, flags, preg, argc);
    } else {
      grepFile(file, *filename, flags, preg, argc);
    }
    fclose(file);
  }
  regfree(preg);
  return error;
}

void *xmalloc(size_t size) {
  void *temp;
  temp = malloc(size);
  if (!temp) exit(errno);
  return temp;
}

void *xrealloc(void *block, size_t size) {
  void *temp;
  temp = realloc(block, size);
  if (!temp) exit(errno);
  return temp;
}

char *string_append_expr(char *string, size_t *size, char const *expr,
                         size_t size_expr) {
  string = xrealloc(string, *size + size_expr + 7);
  string[*size] = '\\';
  string[*size + 1] = '|';
  string[*size + 2] = '\\';
  string[*size + 3] = '(';

  memcpy(string + *size + 4, expr, size_expr);
  *size += size_expr + 4;
  string[*size] = '\\';
  string[*size + 1] = ')';
  string[*size + 2] = '\0';
  *size += 2;
  return string;
}
Flags GrepReadFlags(int argc, char *argv[]) {
  Flags flags = {NULL,  0,     0,     false, false, false,
                 false, false, false, false, NULL,  NULL};
  int currentFlag;
  flags.pattern = xmalloc(2);
  flags.pattern[0] = '\0';
  flags.pattern[1] = '\0';
  size_t pattern_size = 0;

  while ((currentFlag = getopt_long(argc, argv, "e:ivclnof:hs", 0, 0)) != -1) {
    switch (currentFlag) {
      case 'e':
        flags.pattern = string_append_expr(flags.pattern, &pattern_size, optarg,
                                           strlen(optarg));
        break;
      case 'f':
        flags.pattern_file = optarg;  // Сохраняем имя файла с шаблонами
        break;
      case 'i':
        flags.regex_flag |= REG_ICASE;
        break;
      case 'v':
        flags.invert = true;
        break;
      case 'c':
        flags.count = true;
        break;
      case 'l':
        flags.filesMatch = true;
        break;
      case 'n':
        flags.numberLine = true;
        break;
      case 'o':
        flags.PrintMatched = true;
        break;
      case 'h':
        flags.h = true;
        break;
      case 's':
        flags.s = true;
        break;
      default:
        break;
    }
  }
  if (pattern_size) {
    flags.size = pattern_size;
  }
  return flags;
}
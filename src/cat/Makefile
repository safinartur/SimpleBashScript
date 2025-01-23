# Установка компилятора и флагов
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11

# Имя выходного файла
TARGET = s21_cat

# Исходные файлы
SRCS = s21_cat.c # Добавьте здесь другие исходные файлы, если необходимо

# Объектные файлы
OBJS = $(SRCS:.c=.o)

# Правило по умолчанию
all: $(TARGET)

# Правило для создания выходного файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Правило для компиляции объектных файлов
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка объектных файлов и выходного файла
clean:
	rm -f $(OBJS) $(TARGET)

# Правило для полного очищения
fclean: clean

# Правило для перезаписи
re: fclean all

.PHONY: all clean fclean re
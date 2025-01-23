#!/bin/bash

#Modifiable options, you can change those according to your needs
target="s21_cat"
#chars checked (128 or 256)
char_num=256
#a modifier to change number of memtests
magic_number=1
#flags to be checked
options=(-A 
        -b 
        -e 
        -E
        -n
        -s
        -t
        -T
        -v
        --number-nonblank
        --number
        --squeeze-blank)
options_size=${#options[@]}

variants=(
""
"-"
"memtests/long_string.txt"
"nofile.txt"
"memtests"
"FILES"
"FILES FILES FILES FILES FILES FILES FILES FILES FILES FILES FILES FILES"
)
variants_size=${#variants[@]}

#function to memtest s21_cat 
memtest() {
    valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind.log ./$target $params <memtests/long_string.txt > /dev/null 2>&1
    grep "All heap blocks were freed -- no leaks are possible" valgrind.log > /dev/null
    if [ $? -eq 0 ];
    then
    printf  "[SUCCESS]\tTest number: %5d\tOption is $params\n" "$test_number">> test.log
    else
    if [ $? -eq 1 ];
    then
    printf  "[FAIL]\t\tTest number: %5d\tFailed with $params\n" "$test_number" >> test.log
    printf  "\n\n\tTEST NUMBER %5d\n\t$params\n\n" "$test_number" >> debug.txt
    cat valgrind.log >> debug.txt
    else
    printf  "[ERROR]\tTest number: %5d couldnt parse valgrind output\n" "$test_number" >> test.log
    fi
    fi
    test_number=$((test_number + 1))
}

progress_bar() {
  percent=$(( test_number * 100 / total_tests_num ))
  printf "\rTests progress: [%-50s]%3d%%\ttest number:%5d " "$(printf '#%.0s' $(seq 1 $(( percent / 2))))" "${percent}" "${test_number}"
}

#build memtests randomizer
build_randomizer() {
  echo "#include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <time.h>
  const char ctrl_chars[] = {0x0c, 0x0a, 0x0d, 0x09, 0x0b};
  void place_ctrl_chars(FILE* memtestfile) {
    int count = 16;
    while (count--) {
      fwrite(ctrl_chars + (rand() % sizeof(ctrl_chars)), sizeof(char), 1,
            memtestfile);
    }
  }
  void do_newline_seq(FILE* memtestfile) {
    int count = 4;
    while (count--) {
      fwrite(ctrl_chars + 1, sizeof(char), 1, memtestfile);
    }
  }
  void place_rand_char(FILE* memtestfile) {
    int count = $char_num;
    while (count--) {
      char byte = rand() % $char_num;
      fwrite(&byte, sizeof(char), 1, memtestfile);
    }
  }
  void init_file(const char* filename) {
    FILE* memtestfile = fopen(filename, \"w\");
    if (!strcmp(filename, \"memtests/empty\")) {
    } else if (!strcmp(filename, \"memtests/linefeed\")) {
      do_newline_seq(memtestfile);
    } else {
      if (rand() % 2) do_newline_seq(memtestfile);
      if (rand() % 2) place_ctrl_chars(memtestfile);
      place_rand_char(memtestfile);
      if (rand() % 2) do_newline_seq(memtestfile);
    }
    fclose(memtestfile);
  }

  int main(int argc, char* argv[]) {
    srand(time(NULL));
    while (--argc) {
      init_file(argv[argc]);
    }
    return 0;
  }" > testfile_maker.c

  gcc testfile_maker.c -o make_test.out
}

#start memtests

#Usage msg
if [[ ! -s ./$target ]]; then
    echo "You must put cat_memtest.sh to one directory with your $target executable file"  
    exit 1
fi
if [ $# -eq 0 ]; then
    echo "Usage: $0 \"number\""
    echo "number \"1\" for one flag per memtest, \"2\" for flag combinations"
    exit 1
fi

if [[ $1 != 1 && $1 != 2 ]]; then
    echo "Error: accepted arguments are \"1\" or \"2\""
    exit 1
fi

test_number=0
start_number=$(($magic_number * 4 / $1 / $1))
total_tests_num=$(($start_number * ($options_size ** $1) * $variants_size ))

echo -e "***Memory tests running...\n"

#create memtest directory
rm -rf memtest_directory
mkdir memtest_directory
cp ./$target memtest_directory/
cd memtest_directory/
mkdir memtests
printf 'c%.0s' {1..5000} > memtests/long_string.txt
build_randomizer

while [ $start_number != 0 ]; do
    files_list="memtests/test_$start_number.1 memtests/empty memtests/test_$start_number.2 memtests/linefeed memtests/test_$start_number.3 memtests/test_$start_number.4"
    ./make_test.out $files_list
    if [[ $1 -eq 1 ]]; then
            for opt1 in "${options[@]}"; do
                for files in "${variants[@]}"; do
                    var=$(echo "$files" | sed "s|FILES|${files_list}|g")
                    params="$opt1 $var"
                    memtest
                    progress_bar
                done
            done
    else
        for opt1 in "${options[@]}"; do
            for opt2 in "${options[@]}"; do
                for files in "${variants[@]}"; do
                    var=$(echo "$files" | sed "s|FILES|${files_list}|g")
                    params="$opt1 $opt2 $var"
                    memtest
                    progress_bar
                done
            done
        done
    fi
    start_number=$((start_number - 1))
done

echo
if [ $(grep -c FAIL test.log) -eq 0 ]; then
    echo -e "\tTEST RUSULT: 100% in $test_number tests!!!"
    cd ..
else
    result=$(grep -c SUCCESS test.log)
    percentage=$((100 * result / test_number))
    echo -e "\tTEST RESULT:\t$percentage%\nSee test.log and debug.txt for info"
fi
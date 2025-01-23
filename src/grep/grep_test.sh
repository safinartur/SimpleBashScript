#!/bin/bash

#Modifiable options, you can change those according to your needs
target="s21_grep"
#chars checked (128 or 256)
char_num=128
#a modifier to change number of tests
magic_number=2
#flags to be checked
options=(    
      "-v"
      "-c"
      "-l"
      "-n"
      "-h"
      "-o"
      "-i"
      "-s"
      "")
options_size=${#options[@]}

files2grep=(
        "tests/long_string.txt"
        "nofile.txt"
        "tests"
        "FILES FILES FILES FILES FILES FILES FILES FILES FILES FILES"
    )
files2grep_size=${#files2grep[@]}

    patterns=(
        "h"
        "[^w]"
        "-e d"
        "-e w -e h -e a -e t -e d -e a -e f -e q"
        "-e d -f PATTERN"
        "-f PATTERN -f PATTERN -f PATTERN -f PATTERN -f PATTERN -f PATTERN -f PATTERN -f PATTERN -f PATTERN -f PATTERN"
        "[^")
patterns_size=${#patterns[@]}


#function to test grep
test() {
    ./"$target" $params < tests/long_string.txt > s21_result 2>/dev/null
    grep $params < tests/long_string.txt > cat_result 2>/dev/null
    diff -q cat_result s21_result > /dev/null 2>&1
    if [ $? -eq 0 ];
    then
    printf  "[SUCCESS]\tTest number: %5d\tOption is $params\n" "$test_number">> test.log
    else
    if [ $? -eq 1 ];
    then
    printf  "[FAIL]\t\tTest number: %5d\tFailed with $params\n" "$test_number" >> test.log
    echo -e "TRY AGAN\n./$target $params\n\n" >> debug.txt
    else
    printf  "[ERROR]\tTest number: %5d\tFailed with $params\n" "$test_number" >> test.log
    fi
    fi
    test_number=$((test_number + 1))

}

#display progress bar
progress_bar() {
  percent=$(( test_number * 100 / total_tests_num ))
  printf "\rTests progress: [%-50s]%3d%%\ttest number:%5d " "$(printf '#%.0s' $(seq 1 $(( percent / 2))))" "${percent}" "${test_number}"
}

#build tests randomizer
build_randomizer() {
  echo "#include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <time.h>
  const char ctrl_chars[] = {0x0c, 0x0a, 0x0d, 0x09, 0x0b};
  void place_ctrl_chars(FILE* testfile) {
    int count = 16;
    while (count--) {
      fwrite(ctrl_chars + (rand() % sizeof(ctrl_chars)), sizeof(char), 1,
            testfile);
    }
  }
  void do_newline_seq(FILE* testfile) {
    int count = 4;
    while (count--) {
      fwrite(ctrl_chars + 1, sizeof(char), 1, testfile);
    }
  }
  void place_rand_char(FILE* testfile) {
    int count = $char_num;
    while (count--) {
      char byte = 32+rand() % ($char_num-32);
      fwrite(&byte, sizeof(char), 1, testfile);
    }
  }
  void place_rand_char2(FILE* testfile) {
    int count = 3;
    while (count--) {
      char byte = 32 +rand() % 224;
      fwrite(&byte, sizeof(char), 1, testfile);
    }
  }
  void init_file(const char* filename) {
    FILE* testfile = fopen(filename, \"w\");
    if (!strcmp(filename, \"tests/empty\")) {
    } else if (!strcmp(filename, \"tests/linefeed\")) {
      do_newline_seq(testfile);
    } else if (!strcmp(filename, \"tests/pattern\")) {
      place_rand_char2(testfile);
    } else {
      if (rand() % 2) do_newline_seq(testfile);
      place_rand_char(testfile);
    }
    fclose(testfile);
  }

  int main(int argc, char* argv[]) {
    srand(time(NULL));
    while (--argc) {
      init_file(argv[argc]);
    }
    return 0;
  }" > testfile_maker.c

  gcc testfile_maker.c -o make_test.out
  rm testfile_maker.c
}

#start tests
#Usage msg
if [[ ! -s ./$target ]]; then
    echo "You must put cat_test.sh to one directory with your $target executable file"  
    exit 1
fi
if [ $# -eq 0 ]; then
    echo "Usage: $0 \"number\""
    echo "number \"1\" for one flag per test, \"2\" for flag combinations"
    exit 1
fi

if [[ $1 != 1 && $1 != 2 ]]; then
    echo "Error: accepted arguments are \"1\" or \"2\""
    exit 1
fi

echo -e "***Functional tests running...\n"
#create test directory
rm -rf test_directory
mkdir  test_directory
cp ./$target test_directory/
cd test_directory/
mkdir tests
echo start >> test.log
printf 'c%.0s' {1..5000} > tests/long_string.txt
build_randomizer

test_number=0
start_number=$(( $magic_number * 4 / $1 / $1))
total_tests_num=$(($start_number * ($options_size ** $1) * $patterns_size * $files2grep_size ))

while [ $start_number != 0 ]; do
    files_list="tests/test_$start_number.1 tests/empty tests/test_$start_number.2 tests/linefeed tests/test_$start_number.3 tests/test_$start_number.4 "
    pattern_file="tests/pattern"
    ./make_test.out $files_list
    ./make_test.out $pattern_file

    if [[ $1 -eq 1 ]]; then
        for opt1 in "${options[@]}"; do
            for files1 in "${files2grep[@]}"; do
                for ptrn1 in "${patterns[@]}"; do
                    files=$(echo "$files1" | sed "s|FILES|${files_list}|g")
                    ptrn=$(echo "$ptrn1" | sed "s|PATTERN|${pattern_file}|g")
                    params="$ptrn $opt1 $files"
                    test
                    progress_bar
                done
            done
        done
    else
        for opt1 in "${options[@]}"; do
            for opt2 in "${options[@]}"; do
                for files1 in "${files2grep[@]}"; do
                    for ptrn1 in "${patterns[@]}"; do
                        files=$(echo "$files1" | sed "s|FILES|${files_list}|g")
                        ptrn=$(echo "$ptrn1" | sed "s|PATTERN|${pattern_file}|g")
                        params="$ptrn $opt1 $opt2 $files"
                        test
                        progress_bar
                    done
                done
            done
        done
    fi
  start_number=$((start_number - 1))
done
rm make_test.out cat_result s21_result
echo
if [ $(grep -c FAIL test.log) -eq 0 ]; then
    echo -e "\tTEST RUSULT: 100% in $test_number tests!!!\n"
    cd ..
else
    result=$(grep -c SUCCESS test.log)
    percentage=$((100 * result / test_number))
    echo -e "\tTEST RESULT:\t$percentage%\nSee test.log and debug.txt for info"
fi
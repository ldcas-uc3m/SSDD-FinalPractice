#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX_VALUE1 256

typedef int (*FUNC_TEST)();

int ID_INICIAL = 0;

void format_time(char *time_str, int total_seconds) {
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;
    sprintf(time_str, "%02im ,%02is", minutes, seconds);
}

void execute_test(FUNC_TEST f_test, char *msg) {
    clock_t start, elapsed;
    start = clock();
    int res = f_test();
    elapsed = ((double)(clock() - start)) / CLOCKS_PER_SEC;

    char time_str[128];
    format_time(time_str, elapsed);

    if (res == 0) {
        printf("✅ Success %s => %s\n", msg, time_str);
    } else {
        printf("❌ Error %s => %s\n", msg, time_str);
    }
}




int main(int argc, char* argv[]) {
    // Leemos offset para los tests
    int clean_state = 1;
    if (argc > 1) {
        ID_INICIAL = atoi(argv[1]);
        clean_state = 0;
    }

    
}
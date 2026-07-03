#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pty.h>

static int32_t masterfd;

int main(void) {
    if(forkpty(&masterfd, NULL, NULL, NULL) == 0) {
        execlp("usr/bin/bash", "bash", NULL);
        perror("execlp");
        exit(1);
    }

    bool running = true;

    while(running) {
        
    }

    printf("Hello World!");
    return EXIT_SUCCESS;
}

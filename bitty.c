#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pty.h>

static int32_t masterfd;

size_t read_from_pty(void) {

}

int main(void) {
    if(forkpty(&masterfd, NULL, NULL, NULL) == 0) {
        execlp("usr/bin/bash", "bash", NULL);
        perror("execlp");
        exit(1);
    }

    bool running = true;

    fd_set fdset;
    while(running) {
        //Clearing and rebuilding the fdset each loop
        FD_ZERO(&fdset);
        FD_SET(masterfd, &fdset);
        select(masterfd, &fdset, NULL, NULL, NULL);
        
        if(FD_ISSET(masterfd, &fdset)) {
            //read_from_pty();
        }
    }

    printf("Hello World!");
    return EXIT_SUCCESS;
}

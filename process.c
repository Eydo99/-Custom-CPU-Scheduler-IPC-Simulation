#include "headers.h"

int main(int argc, char * argv[]) {
    // The process just waits for signals from the scheduler.
    // pause() uses 0% CPU, making our simulation highly efficient!
    // We wrap it in an infinite loop just in case a signal interrupts the pause.
    while (1) {
        pause(); 
    }
    return 0;
}
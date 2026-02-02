#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HISTORY_SIZE 5

// Store last 5 lines as strings
char *history[HISTORY_SIZE] = {NULL};
int current_position = 0;  // Next position to write
int stored_count = 0;      // How many lines stored (0-5)

void add_to_history(char *line);
void print_history(void);
void free_history(void);

int main(void) {
    char *line = NULL;     // getline allocates this
    size_t len = 0;
    ssize_t nread;

    while (1) {
        printf("Enter input: ");
        nread = getline(&line, &len, stdin);
        
        
        if (nread == -1) {
            break;
        }
        
        // Check if user typed "print"
        if (strcmp(line, "print\n") == 0) {
            print_history();
        }
        
        
        add_to_history(line);
    }
    
    // Clean up memory
    free(line);
    free_history();
    
    return 0;
}

void add_to_history(char *line) {
    // Free old string at this position if it exists
    if (history[current_position] != NULL) {
        free(history[current_position]);
    }
    
    // Make a copy of the line
    history[current_position] = strdup(line);
    
    // Move to next position 
    current_position = (current_position + 1) % HISTORY_SIZE;
    
    // Track how many lines we have 
    if (stored_count < HISTORY_SIZE) {
        stored_count++;
    }
}

void print_history(void) {
    // Find oldest entry position
    int start = (stored_count < HISTORY_SIZE) ? 0 : current_position;
    
    // Print all stored lines in order
    for (int i = 0; i < stored_count; i++) {
        int index = (start + i) % HISTORY_SIZE;
        printf("%s", history[index]);
    }
}

void free_history(void) {
    // Free all stored strings
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (history[i] != NULL) {
            free(history[i]);
        }
    }
}

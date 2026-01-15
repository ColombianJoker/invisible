#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#define MB (1024 * 1024)
#define CHUNK_SIZE (1024 * 1024)
#define WRITE_BLOCK 10

long parse_seconds(char *arg) {
    char *endptr;
    double value = strtod(arg, &endptr);

    if (arg == endptr) return -1; // No number found

    // Skip leading whitespace in the suffix
    while (isspace((unsigned char)*endptr)) endptr++;

    if (*endptr == '\0' || strstr(endptr, "s") == endptr || strstr(endptr, "sec") == endptr || 
        strstr(endptr, "seg") == endptr) {
        return (long)value;
    } else if (strstr(endptr, "m") == endptr || strstr(endptr, "min") == endptr) {
        return (long)(value * 60);
    }
    
    return -1; // Invalid suffix
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <filename> [duration]\n", argv[0]);
        fprintf(stderr, "©️ 2026, Ramón Barrios Láscar.\n");
        return 2;
    }

    char *filename = argv[1];
    long duration = 60;

    // 1. Check if file exists
    if (access(filename, F_OK) == 0) {
        fprintf(stderr, "%s: Error, file '%s' already exists.\n", argv[0], filename);
        return 1;
    }

    // 2. Parse duration if provided
    if (argc == 3) {
        duration = parse_seconds(argv[2]);
        if (duration <= 0) {
            fprintf(stderr, "%s: Error, invalid time format '%s'.\n", argv[0], argv[2]);
            return 2;
        }
    }

    // 3. Create the file
    int fd = open(filename, O_RDWR | O_CREAT | O_EXCL, 0644);
    if (fd == -1) {
        perror("Error creating file");
        return 1;
    }

    printf("File created: %s\n", filename);

    // 4. Unlink the file (it becomes "invisible")
    if (unlink(filename) == -1) {
        perror("Error unlinking file");
        close(fd);
        return 1;
    }
    printf("%s: File unlinked. It is now invisible to 'ls' but open in this process.\n", argv[0]);

    // 5. Start writing loop
    unsigned char *buffer = malloc(CHUNK_SIZE);
    if (!buffer) {
        perror("Failed to allocate buffer");
        return 1;
    }
    memset(buffer, 'A', CHUNK_SIZE);

    printf("Writing 10MB/s for %ld seconds...\n", duration);

    for (int i = 0; i < duration; i++) {
        for (int j = 0; j < WRITE_BLOCK; j++) { // Write 1MB WRITE_BLOCK times
            if (write(fd, buffer, CHUNK_SIZE) == -1) {
                perror("\nWrite error");
                free(buffer);
                close(fd);
                return 1;
            }
        }
        printf("\rElapsed: %d/%ld seconds", i + 1, duration);
        fflush(stdout);
        sleep(1);
    }

    printf("\nFinished. Closing file and exiting.\n");
    printf("©️ 2026, Ramón Barrios Láscar.\n");

    // 6. Cleanup
    free(buffer);
    close(fd); // Disk space is finally reclaimed here

    return 0;
}
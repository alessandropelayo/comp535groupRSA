#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main() {
    const char *folderPath = "./data/flowers/";


    struct dirent *entry;
    DIR *dir = opendir(folderPath);
    if (dir == NULL) {
        printf("Could not open directory: %s\n", folderPath);
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignore the current (".") and parent ("..") directory entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Prepare the full path to the image file
        char filePath[256];
        snprintf(filePath, sizeof(filePath), "%s%s", folderPath, entry->d_name);

        // Prepare the command to call the image_loader with the file path
        char command[300];
        snprintf(command, sizeof(command), "./rsa_image_encrypter %s", filePath);

        // Call the image loader program
        int result = system(command);

        // Check for errors in running the command
        if (result == -1) {
            printf("Error executing image loader for file: %s\n", filePath);
            closedir(dir);
            return -1;
        }

        printf("Processed file: %s\n", filePath);
    }
    closedir(dir);
    return 0;
}
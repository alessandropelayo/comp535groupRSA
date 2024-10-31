#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

// THIS FILE CURRENTLY HAS NO PARALLEL PROGRAMING
int main() {
    // const char *folders[] = {"./data/flowers/", "./data/dogs/", "./data/cats/", "./data/horses/", "./data/human/"};
    const char *folders[] = {"./data/flowers/"};
    int numFolders = sizeof(folders) / sizeof(folders[0]);


    for (int i = 0; i < numFolders; i++) {
        const char *folderPath = folders[i];
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

            // Prepare the command to call the rsa_encrypter with the file path
            char command[300];
            snprintf(command, sizeof(command), "./rsa_image_encrypter %s", filePath);

            // Call the program
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
    }
    
    return 0;
}
// so sanh noi dung va kich thuoc 2 file nhi phan: gcc binarycompare.c -o binarycompare && ./binarycompare file1 file2
#include <stdio.h>
#include <sys/stat.h>
 
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s file1 file2\n", argv[0]);
        return 1;
    }
    
    char *file1_name = argv[1];
    char *file2_name = argv[2];
    
    struct stat stat1, stat2;
    
    if (stat(file1_name, &stat1) != 0) {
        printf("Error getting %s size\n", file1_name);
        return 1;
    }
    
    if (stat(file2_name, &stat2) != 0) {
        printf("Error getting %s size\n", file2_name);
        return 1;
    }
    
    if (stat1.st_size != stat2.st_size) {
        printf("Files differ in size: %ld vs %ld bytes\n", stat1.st_size, stat2.st_size);
        return 1;
    }
    
    FILE *file1 = fopen(file1_name, "rb");
    if (file1 == NULL) {
        printf("Error opening %s\n", file1_name);
        return 1;
    }
 
    FILE *file2 = fopen(file2_name, "rb");
    if (file2 == NULL) {
        printf("Error opening %s\n", file2_name);
        fclose(file1);
        return 1;
    }
 
    int byte_count = 0;
    while (!feof(file2) && !feof(file1)) {
        unsigned char byte1 = fgetc(file1);
        unsigned char byte2 = fgetc(file2);
        
        if (byte1 != byte2) {
            printf("Files differ at byte %d (0x%02X vs 0x%02X)\n", byte_count, byte1, byte2);
            fclose(file1);
            fclose(file2);
            return 1;
        }
        byte_count++;
    }
 
    printf("Files are identical with %d bytes compared\n", byte_count - 1);
    fclose(file1);
    fclose(file2);
    return 0;
}
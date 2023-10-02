#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(1)

typedef struct {
    short type;
    int file_size;
    short reserved1;
    short reserved2;
    int offset;
} BMPHeader;

typedef struct {
    int size;
    int width;
    int height;
    short planes;
    short bits_per_pixel;
    unsigned compression;
    unsigned image_size;
    int x_resolution;
    int y_resolution;
    int n_colors;
    int important_colors;
} BMPInfoHeader;

void extractMessage(unsigned char *data, char *message, int imageSize) {
    int messageIdx = 0;  // index for our message
    char currentChar = 0;

    for (int dataIdx = 0; dataIdx < imageSize; dataIdx++) {
        // Retrieve the LSB and set it to the respective bit of our character
        currentChar = currentChar | ((data[dataIdx] & 1) << (dataIdx % 8));

        // Once we've parsed 8 bits, store the constructed character and reset for the next one
        if (dataIdx % 8 == 7) {
            message[messageIdx] = currentChar;
            
            if(currentChar == '\0') {  // Null terminator found
                break;
            }
            
            messageIdx++;
            currentChar = 0;  // Reset for next character
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: %s <path_to_image.bmp>\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];

    FILE *image = fopen(image_path, "rb");
    if(!image) {
        perror("Unable to open image");
        return 1;
    }

    BMPHeader header;
    BMPInfoHeader info_header;

    fread(&header, sizeof(BMPHeader), 1, image);
    fread(&info_header, sizeof(BMPInfoHeader), 1, image);

    if(header.type != 0x4D42) {
        printf("Not a BMP file\n");
        fclose(image);
        return 1;
    }

    if(info_header.bits_per_pixel != 24) {
        printf("Only 24-bit BMP files are supported\n");
        fclose(image);
        return 1;
    }

    unsigned char *data = (unsigned char*)malloc(info_header.image_size);
    fseek(image, header.offset, SEEK_SET);
    fread(data, info_header.image_size, 1, image);

    char message[info_header.image_size / 8];  // 8 pixels can store 1 character
    memset(message, 0, sizeof(message));

    extractMessage(data, message, info_header.image_size);

    printf("Extracted message: %s\n", message);

    free(data);
    fclose(image);
    return 0;
}

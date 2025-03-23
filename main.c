#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16
#define INITIAL_WIDTH 800
#define INITIAL_HEIGHT 600

// Load PSF Font into memory
unsigned char font[256 * CHAR_HEIGHT];

void load_font(const char *font_path) {
    int font_fd = open(font_path, O_RDONLY);
    if (font_fd == -1) {
        perror("Error opening font");
        exit(1);
    }
    
    // Read PSF header
    unsigned char header[4];
    if (read(font_fd, header, 4) != 4) {
        perror("Error reading font header");
        close(font_fd);
        exit(1);
    }
    
    // Check for Unicode table
    int has_unicode_table = header[3] & 1;
    
    // Skip Unicode table if present
    if (has_unicode_table) {
        lseek(font_fd, 512, SEEK_CUR);
    }
    
    // Read font glyphs - FIXED: incorrect read() parameters
    if (read(font_fd, font, sizeof(font)) != sizeof(font)) {
        perror("Error reading font data");
        close(font_fd);
        exit(1);
    }
    
    close(font_fd);
    printf("Font loaded successfully.\n");
}

// Render a character onto the bitmap
void draw_char(unsigned char *bitmap, int img_width, int x, int y, char c) {
    if (c < 0) return; // Invalid character check
    
    unsigned char *glyph = font + (c * CHAR_HEIGHT);
    for (int i = 0; i < CHAR_HEIGHT; i++) {
        for (int j = 0; j < CHAR_WIDTH; j++) {
            if (glyph[i] & (1 << (7 - j))) {
                bitmap[(y + i) * img_width + (x + j)] = 0; // Black pixel
            } else {
                bitmap[(y + i) * img_width + (x + j)] = 255; // White pixel
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <font path> <output.pgm>\n", argv[0]);
        return 1;
    }
    
    load_font(argv[1]);
    
    // Read entire input into buffer first to determine required dimensions
    char *input_buffer = NULL;
    size_t buffer_size = 0;
    size_t buffer_capacity = 1024;
    size_t max_line_length = 0;
    size_t line_count = 1; // Start with 1 to account for last line
    size_t current_line_length = 0;
    
    input_buffer = malloc(buffer_capacity);
    if (!input_buffer) {
        perror("Memory allocation failed");
        return 1;
    }
    
    char c;
    while (read(STDIN_FILENO, &c, 1) > 0) {
        // Expand buffer if needed
        if (buffer_size >= buffer_capacity - 1) {
            buffer_capacity *= 2;
            input_buffer = realloc(input_buffer, buffer_capacity);
            if (!input_buffer) {
                perror("Memory reallocation failed");
                return 1;
            }
        }
        
        input_buffer[buffer_size++] = c;
        
        // Track longest line and line count
        if (c == '\n') {
            if (current_line_length > max_line_length) {
                max_line_length = current_line_length;
            }
            current_line_length = 0;
            line_count++;
        } else {
            current_line_length++;
        }
    }
    
    // Check the last line if it didn't end with a newline
    if (current_line_length > max_line_length) {
        max_line_length = current_line_length;
    }
    
    // Calculate required image dimensions
    int img_width = (max_line_length + 1) * CHAR_WIDTH; // +1 for safety
    int img_height = line_count * CHAR_HEIGHT;
    
    // Use at least the initial dimensions
    if (img_width < INITIAL_WIDTH) img_width = INITIAL_WIDTH;
    if (img_height < INITIAL_HEIGHT) img_height = INITIAL_HEIGHT;
    
    printf("Creating image with dimensions: %d x %d pixels\n", img_width, img_height);
    
    unsigned char *bitmap = calloc(img_width * img_height, sizeof(unsigned char));
    if (!bitmap) {
        perror("Memory allocation failed");
        free(input_buffer);
        return 1;
    }
    
    // Initialize with white background
    memset(bitmap, 255, img_width * img_height);
    
    // Render text from buffer
    int x = 0, y = 0;
    for (size_t i = 0; i < buffer_size; i++) {
        c = input_buffer[i];
        if (c == '\n') {
            x = 0;
            y += CHAR_HEIGHT;
        } else if (c == '\r') {
            x = 0;
        } else {
            if (x + CHAR_WIDTH > img_width) {
                x = 0;
                y += CHAR_HEIGHT;
            }
            if (y + CHAR_HEIGHT > img_height) {
                // Reallocate the bitmap to accommodate more text
                int new_height = img_height * 2;
                printf("Expanding image height to %d pixels\n", new_height);
                
                unsigned char *new_bitmap = calloc(img_width * new_height, sizeof(unsigned char));
                if (!new_bitmap) {
                    perror("Memory allocation failed during expansion");
                    free(bitmap);
                    free(input_buffer);
                    return 1;
                }
                
                // Copy old bitmap and fill the rest with white
                memcpy(new_bitmap, bitmap, img_width * img_height);
                memset(new_bitmap + img_width * img_height, 255, img_width * (new_height - img_height));
                
                free(bitmap);
                bitmap = new_bitmap;
                img_height = new_height;
            }
            draw_char(bitmap, img_width, x, y, c);
            x += CHAR_WIDTH;
        }
    }
    
    // Save as PGM
    FILE *pgm = fopen(argv[2], "wb");
    if (!pgm) {
        perror("Error opening output file");
        free(bitmap);
        free(input_buffer);
        return 1;
    }
    
    fprintf(pgm, "P5\n%d %d\n255\n", img_width, img_height);
    fwrite(bitmap, 1, img_width * img_height, pgm);
    fclose(pgm);
    
    printf("PGM image saved to %s\n", argv[2]);
    
    free(bitmap);
    free(input_buffer);
    return 0;
}

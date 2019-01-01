#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define MAX_BUFFER     100000
#define TMP_BUFFER     24
#define RGB_SIZE       3
#define COLOR_MAP_SIZE 36

#define RAND_TO_X(x) (rand() % x + 1) - 1

static uint8_t color_map[COLOR_MAP_SIZE][RGB_SIZE] = {{07, 07, 07},
    {0x1f, 0x07, 0x07}, {0x2f, 0x0f, 0x07}, {0x47, 0x0f, 0x07}, {0x57, 0x17, 0x07}, {0x67, 0x1f, 0x07},
    {0x77, 0x1f, 0x07}, {0x8f, 0x27, 0x07}, {0x9f, 0x2f, 0x07}, {0xaf, 0x3f, 0x07}, {0xbf, 0x47, 0x07},
    {0xc7, 0x47, 0x07}, {0xdf, 0x4f, 0x07}, {0xdf, 0x57, 0x07}, {0xdf, 0x57, 0x07}, {0xd7, 0x5f, 0x07},
    {0xd7, 0x67, 0x0f}, {0xcf, 0x6f, 0x0f}, {0xcf, 0x77, 0x0f}, {0xcf, 0x7f, 0x0f}, {0xcf, 0x87, 0x17},
    {0xc7, 0x87, 0x17}, {0xc7, 0x8f, 0x17}, {0xc7, 0x97, 0x1f}, {0xbf, 0x9f, 0x1f}, {0xbf, 0x9f, 0x1f},
    {0xbf, 0xa7, 0x27}, {0xbf, 0xa7, 0x27}, {0xbf, 0xaf, 0x2f}, {0xb7, 0xaf, 0x2f}, {0xb7, 0xb7, 0x2f},
    {0xb7, 0xb7, 0x37}, {0xcf, 0xcf, 0x6f}, {0xdf, 0xdf, 0x9f}, {0xef, 0xef, 0xc7}, {0xff, 0xff, 0xff},
};

struct inferno {
    int height;
    int width;
    uint8_t *grid;
};

static struct inferno 
*inferno_new(const int height, const int width) 
{
    struct inferno *i = malloc(sizeof(struct inferno));
    if (!i) 
        return NULL;
    memset(i, 0, sizeof(struct inferno));
    if (height > 0) 
        i->height = height;
    if (width > 0) 
        i->width = width;
    i->grid = malloc(sizeof(uint8_t)*(i->height*i->width));
    if (!i->grid) 
        return NULL;
    memset(i->grid, 0, sizeof(uint8_t));
    for (int j = 0; j < i->width; j++)
        i->grid[((i->height-1)*i->width)+j] = 35;
    return i;
}

static void 
inferno_free(struct inferno *i) 
{
    if (!i) 
        return;
    if (!i->grid)
        free(i->grid);
    free(i);
}

static void 
spread(struct inferno *i) 
{   
    for (int y = i->height - 1; y > 0; y--) {
        for (int x = 0; x < i->width; x++) {
            int src = (y * i->width) + x;
            int dst = (src - i->width) + RAND_TO_X(4) - 2;
            if (dst < 0)
                dst = 0;
            i->grid[dst] = i->grid[src] - RAND_TO_X(6) - 1;
            if (i->grid[dst] > 35) 
                i->grid[dst] = 35;
            if (i->grid[dst] < 0)
                i->grid[dst] = 0;
        }
    }
}

static void
render(struct inferno *i) 
{
    printf("\x1b[48;2;0;0;0m");
    printf("\x1b[;f");

    char buffer[MAX_BUFFER];
    memset(buffer, 0, MAX_BUFFER);

    int8_t prev = -1;
    for (int y = 0; y < i->height; y++) {
        for (int x = 0; x < i->width; x++) {
            int pos = (y * i->width) + x;
            if (i->grid[pos] != prev) {
                char tbuf[TMP_BUFFER];
                if (i->grid[pos] < 0 || i->grid[pos] >= COLOR_MAP_SIZE - 1) {
                    sprintf(tbuf, "\x1b[48;2;%d;%d;%dm", 0, 0, 0);
                } else {
                    sprintf(tbuf, "\x1b[48;2;%d;%d;%dm", 
                        color_map[i->grid[pos]][0], 
                        color_map[i->grid[pos]][1], 
                        color_map[i->grid[pos]][2]);
                }
                strcat(buffer, tbuf);
            }
            strcat(buffer, " ");
            prev = i->grid[pos];
        }
    }
    printf("%s", buffer);

    struct timespec t = {0, 100000000L};
    nanosleep(&t, NULL);
}

int 
main(int argc, char **argv) 
{
    srand(time(0));

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_height = w.ws_row;
    int term_width = w.ws_col;

    struct inferno *i = inferno_new(term_height, term_width);
    while (1) {
        render(i);
        spread(i);
    }
    inferno_free(i);

    printf("\x1b[48;2;0;0;0m");
	printf("\x1b[;f");

    return EXIT_SUCCESS;  
}

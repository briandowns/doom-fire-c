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

#define RAND_TO_4 (rand() % 4 + 1)
#define RAND_TO_6 (rand() % 6 + 1)

static const uint8_t color_map[COLOR_MAP_SIZE][RGB_SIZE] = {{07, 07, 07},
    {0x1f, 0x07, 0x07}, {0x2f, 0x0f, 0x07}, {0x47, 0x0f, 0x07}, {0x57, 0x17, 0x07}, {0x67, 0x1f, 0x07},
    {0x77, 0x1f, 0x07}, {0x8f, 0x27, 0x07}, {0x9f, 0x2f, 0x07}, {0xaf, 0x3f, 0x07}, {0xbf, 0x47, 0x07},
    {0xc7, 0x47, 0x07}, {0xdf, 0x4f, 0x07}, {0xdf, 0x57, 0x07}, {0xdf, 0x57, 0x07}, {0xd7, 0x5f, 0x07},
    {0xd7, 0x67, 0x0f}, {0xcf, 0x6f, 0x0f}, {0xcf, 0x77, 0x0f}, {0xcf, 0x7f, 0x0f}, {0xcf, 0x87, 0x17},
    {0xc7, 0x87, 0x17}, {0xc7, 0x8f, 0x17}, {0xc7, 0x97, 0x1f}, {0xbf, 0x9f, 0x1f}, {0xbf, 0x9f, 0x1f},
    {0xbf, 0xa7, 0x27}, {0xbf, 0xa7, 0x27}, {0xbf, 0xaf, 0x2f}, {0xb7, 0xaf, 0x2f}, {0xb7, 0xb7, 0x2f},
    {0xb7, 0xb7, 0x37}, {0xcf, 0xcf, 0x6f}, {0xdf, 0xdf, 0x9f}, {0xef, 0xef, 0xc7}, {0xff, 0xff, 0xff},
};

static uint8_t 
*map_color(const int8_t v)
{
    static uint8_t rgb[3];
    if ((v < 0) || (v >= COLOR_MAP_SIZE - 1)) {
        rgb[0] = 0;
        rgb[1] = 0;
        rgb[2] = 0;
        return rgb;
    }
    rgb[0] = color_map[v][0];
    rgb[1] = color_map[v][1];
    rgb[2] = color_map[v][2];
    return rgb;
}

struct inferno {
    int height;
    int width;
    uint8_t grid[8];
};

static struct inferno 
*inferno_new(const int height, const int width) 
{
    struct inferno *i = malloc(sizeof(struct inferno));
    if (height > 0) 
        i->height = height;
    if (width > 0) 
        i->width = width;
    return i;
}

void 
inferno_free(struct inferno *i) 
{
    if (!i) 
        return;
    free(i);
}

static void 
set_dimensions(struct inferno *i, const int height, const int width) 
{
    i->height = height;
    i->width = width;
}

static void 
init(struct inferno *i) 
{
    int j;
    for (j = 0; j < i->width; j++)
        i->grid[((i->height-1)*i->width)+j] = 35;
}

static void 
spread(struct inferno *i) 
{   
    int y, x;
    int src, dst;
    for (y = i->height - 1; y > 0; y--) {
        for (x = 0; x < i->width; x++) {
            src = (y * i->width) + x;
            dst = (src - i->width) + RAND_TO_4 - 2;
            if (dst < 0) {
                dst = 0;
            }
            i->grid[dst] = i->grid[src] - RAND_TO_6 - 1;
            if (i->grid[dst] > 35) 
                i->grid[dst] = 35;
            if (i->grid[dst] < 0)
                i->grid[dst] = 0;
        }
    }
}

void
render(struct inferno *i) 
{
    printf("\x1b[48;2;0;0;0m");
    printf("\x1b[;f");

    char buffer[MAX_BUFFER];
    char tbuf[TMP_BUFFER];

    static uint8_t *rgb;

    int8_t prev = -1;
    int pos;
    int y, x;

    for (y = 0; y < i->height; y++) {
        for (x = 0; x < i->width; x++) {
            pos = (y * i->width) + x;
            if (i->grid[pos] != prev) {
                rgb = map_color(i->grid[pos]);
                sprintf(tbuf, "\x1b[48;2;%d;%d;%dm", rgb[0], rgb[1], rgb[2]);
                strcat(buffer, tbuf);
                memset(tbuf, 0, TMP_BUFFER);
            }
            strcat(buffer, " ");
            prev = i->grid[pos];
        }
    }

    printf("%s", buffer);
    memset(buffer, 0, MAX_BUFFER);

    struct timespec t = {0, 100000000L};
    nanosleep(&t, NULL);
}

static void
fire(const int height, const int width) 
{
    struct inferno *i = inferno_new(height, width);
    set_dimensions(i, height, width);
    init(i);
    while (1) {
        render(i);
        spread(i);
    }
}

int 
main(int argc, char **argv) 
{
    srand(time(0));

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_height = w.ws_row;
    int term_width = w.ws_col;

    fire(term_height, term_width);

    printf("\x1b[48;2;0;0;0m");
	printf("\x1b[;f");

    return EXIT_SUCCESS;  
}

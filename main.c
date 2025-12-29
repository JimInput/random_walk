#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

#define WIDTH 900
#define HEIGHT 600

#define FPS 60

#define SIZE 5
#define PATH_LENGTH 8
#define WHITE 0xFFFFFF
#define MIN_COLOR 100
#define MAX_COLOR 255


void swap(int *x, int *y) {
    *x ^= *y;
    *y ^= *x;
    *x ^= *y;
}

void draw_pixel(SDL_Surface* surface, int x, int y, uint32_t color) {
    int bpp = SDL_BYTESPERPIXEL(surface->format);
    uint8_t *p = (uint8_t *) surface->pixels + y * surface->pitch + x * bpp;
    switch(bpp) {
        case 1:
            *p = color;
            break;
        case 2:
            *(uint16_t *)p = color;
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (color >> 16) & 0xFF;
                p[1] = (color >> 8) & 0xFF;
                p[2] = color & 0xFF;
            } else {
                p[0] = color & 0xFF;
                p[1] = (color >> 8) & 0xFF;
                p[2] = (color >> 16) & 0xFF;
            }
            break;
        case 4:
            *(uint32_t *)p = color;
            break;

    }
}

void draw_horizontal_line(SDL_Surface *psurface, int x1, int x2, int y, int thickness, uint32_t color) {
    if (x2 < x1) {
        swap(&x1,&x2);
    }
    for (int i = 0; i < thickness; i++) {
        for (int x = x1; x <= x2; x++) {
            draw_pixel(psurface, x, y+i, color);
        }
    }
}

void draw_vertical_line(SDL_Surface *psurface, int x, int y1, int y2, int thickness, uint32_t color) {
    if (y2 < y1) {
        swap(&y1,&y2);
    }
    for (int i = 0; i < thickness; i++) {
        for (int y = y1; y <= y2; y++) {
            draw_pixel(psurface, x+i, y, color);
        }
    }
}

void update_rect(SDL_Rect* rect) {
    int r = rand()%4;

    switch(r) {
        case 0:
            rect->x += PATH_LENGTH;
            break;
        case 1:
            rect->x -= PATH_LENGTH;
            break;
        case 2:
            rect->y += PATH_LENGTH;
            break;
        case 3:
            rect->y -= PATH_LENGTH;
            break;
    }
}

int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

uint32_t hsl_to_rgb(double h, double s, double l) {
    double c = (1 - fabs(2*l-1)) * s;
    double x = c * (1-fabs(fmod(h,2) -1));
    double r1, g1, b1;

    if (0 <= h && h < 1) {r1 = c; g1 = x; b1 = 0;}
    if (1 <= h && h < 2) {r1 = x; g1 = c; b1 = 0;}
    if (2 <= h && h < 3) {r1 = 0; g1 = c; b1 = x;}
    if (3 <= h && h < 4) {r1 = 0; g1 = x; b1 = c;}
    if (4 <= h && h < 5) {r1 = x; g1 = 0; b1 = c;}
    if (5 <= h && h < 6) {r1 = c; g1 = 0; b1 = x;}

    double m = l - c/2.0;
    uint8_t r = (uint8_t) ((r1+m)*255);
    uint8_t g = (uint8_t) ((g1+m)*255);
    uint8_t b = (uint8_t) ((b1+m)*255);

    return (r << 16) | (g << 8) | b;
}

int random_color() {
    double h = (rand() / (double) RAND_MAX)*6;
    return hsl_to_rgb(h, 1, 0.5);
}

typedef struct Agent {
    SDL_Rect rect;
    uint32_t color;
} Agent;

int main(int argc, char* argv[]) {
    int num_agents;
    if (argc == 1) {
        num_agents = 1;
    } else if (argc == 2) {
        num_agents = atoi(argv[1]);
    } else {
        printf("Usage: ./prog <NUM_AGENTS>\n");
        return 0;
    }

    SDL_Window *pwindow = SDL_CreateWindow("Random Walk", WIDTH, HEIGHT, 0);
    SDL_SetWindowPosition(pwindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_Surface *psurface = SDL_GetWindowSurface(pwindow);

    srand(time(NULL));

    Agent *agents = malloc(num_agents*sizeof(Agent));

    for (int i = 0; i < num_agents; i++) {
        agents[i] = (Agent) {(SDL_Rect) {WIDTH/2 - SIZE/2, HEIGHT/2 - SIZE/2, SIZE, SIZE}, random_color()};
    }

    int app_running = TRUE;
    while(app_running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                app_running = FALSE;
            }
        }

        for (int i = 0; i < num_agents; i++) {
            SDL_Rect *agent_rect = &agents[i].rect;

            int prev_x = agent_rect->x;
            int prev_y = agent_rect->y;

            update_rect(agent_rect);
            SDL_FillSurfaceRect(psurface, agent_rect, agents[i].color);

            if (prev_x == agent_rect->x) {
                draw_vertical_line(psurface, prev_x, prev_y, agent_rect->y, SIZE, agents[i].color);
            } else {
                draw_horizontal_line(psurface, prev_x, agent_rect->x, prev_y, SIZE, agents[i].color);
            }
        }

        SDL_UpdateWindowSurface(pwindow);
        SDL_Delay(1000/FPS);
    }

    free(agents);
    return 0;
}

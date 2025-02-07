#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#define WIDTH 800
#define HEIGHT 600
#define FPS 15
#define SQUARE_SIZE 5
#define CELL_CHUNK_SIZE 32

typedef struct cell
{
    SDL_Rect rect;
    int alive;
} cell;

void check_input(SDL_Event* ev, int* running, int* rand_init, int* run, int* mouse_clicked)
{
    while(SDL_PollEvent(ev) != 0)
    {
        if(ev->type == SDL_QUIT)
        {
            *running = !(*running);
        }
        else if (ev->type == SDL_KEYDOWN)
        {
            if (ev->key.keysym.sym == SDLK_r)
            {
                *rand_init = 1;
            }
            else if (ev->key.keysym.sym == SDLK_s)
            {
                *run = !(*run);
            }
        }
        else if (ev->type == SDL_MOUSEBUTTONDOWN)
        {
            *mouse_clicked = 1;
        }
    }
}

void init_cells(cell cells[HEIGHT/SQUARE_SIZE][WIDTH/SQUARE_SIZE])
{
    for (int i = 0; i < HEIGHT/SQUARE_SIZE; i++)
    {
        for (int j = 0; j < WIDTH/SQUARE_SIZE; j++)
        {
            cells[i][j].rect.h = SQUARE_SIZE;
            cells[i][j].rect.w = SQUARE_SIZE;
            cells[i][j].rect.x = j * SQUARE_SIZE;
            cells[i][j].rect.y = i * SQUARE_SIZE;
            cells[i][j].alive = 0;
        }
    }
}

void draw_board(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i < HEIGHT; i += SQUARE_SIZE)
    {
        SDL_RenderDrawLine(renderer, 0, i, WIDTH, i);
        for (int j = 0; j < WIDTH; j+= SQUARE_SIZE)
        {
            SDL_RenderDrawLine(renderer, j, 0, j, HEIGHT);
        }
    }
}

void draw_cells(SDL_Renderer* renderer, cell cells[HEIGHT/SQUARE_SIZE][WIDTH/SQUARE_SIZE])
{
    for (int i = 0; i < HEIGHT/SQUARE_SIZE; i++)
    {
        for (int j = 0; j < WIDTH/SQUARE_SIZE; j++)
        {
            if (cells[i][j].alive)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &(cells[i][j].rect));
            }
        }
    }
}

void update_cells(cell cells[HEIGHT/SQUARE_SIZE][WIDTH/SQUARE_SIZE], int* population)
{
    int cells_h = HEIGHT/SQUARE_SIZE;
    int cells_w = WIDTH/SQUARE_SIZE;

    cell next_state[HEIGHT/SQUARE_SIZE][WIDTH/SQUARE_SIZE];
    memcpy(next_state, cells, sizeof(cell) * (HEIGHT/SQUARE_SIZE) * (WIDTH/SQUARE_SIZE));

    for (int i = 0; i < cells_h; i++)
    {
        for (int j = 0; j < cells_w; j++)
        {
            int nbor_count = 0;
            for (int k = i - 1; k <= i + 1 && k < cells_h && k >= 0; k++)
            {
                for (int l = j - 1; l <= j + 1 && l < cells_w && l >= 0; l++)
                {
                    if (l == j && k == i)
                    {
                        continue;
                    }

                    if (cells[k][l].alive)
                    {
                        nbor_count++;
                    }

                }
            }
            
            if (cells[i][j].alive)
            {
                (*population)++;
                if (nbor_count > 3 || nbor_count < 2)
                {
                    next_state[i][j].alive = 0;
                }
            }
            else
            {
                if (nbor_count == 3)
                {
                    next_state[i][j].alive = 1;
                }
            }

        }
    }

    memcpy(cells, next_state, sizeof(cell) * (HEIGHT/SQUARE_SIZE) * (WIDTH/SQUARE_SIZE));
}

void init_random_pattern(cell cells[HEIGHT/SQUARE_SIZE][WIDTH/SQUARE_SIZE])
{
    int random_x = rand() % (WIDTH / SQUARE_SIZE);
    int random_y = rand() % (HEIGHT / SQUARE_SIZE);
    int cells_h = HEIGHT/SQUARE_SIZE;
    int cells_w = WIDTH/SQUARE_SIZE;

    for (int i = random_y - CELL_CHUNK_SIZE; i <= random_y + CELL_CHUNK_SIZE && i < cells_h && i >= 0; i++)
        {
            for (int j = random_x - CELL_CHUNK_SIZE; j <= random_x + CELL_CHUNK_SIZE && j < cells_w && j >= 0; j++)
            {
                if (rand() % 2)
                {
                    cells[i][j].alive = 1;
                }
            }
        }
}

int main(int argc, char** argv)
{
    srand(time(NULL));

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* win = SDL_CreateWindow("Game of Life", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

    if (!win)
    {
        printf("Couldn't create window.\n");
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer)
    {
        printf("Couldn't create renderer.\n");
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    int run = 0;
    int rand_init = 0;
    int mouse_clicked = 0;
    int population;
    SDL_Event ev;
    cell cells[HEIGHT/SQUARE_SIZE][WIDTH/SQUARE_SIZE];
    init_cells(cells);
    
    while(running)
    {
        population = 0;
        mouse_clicked = 0;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // draw_board(renderer);
        draw_cells(renderer, cells);

        SDL_RenderPresent(renderer);
        check_input(&ev, &running, &rand_init, &run, &mouse_clicked);
        if (rand_init)
        {
            init_cells(cells);
            init_random_pattern(cells);
            rand_init = 0;
        }

        if (run)
        {
            update_cells(cells, &population);
        }

        if (mouse_clicked)
        {
            if (ev.button.button == SDL_BUTTON_LEFT)
            {
                int mouse_cell_y = ((ev.button.y/SQUARE_SIZE));
                int mouse_cell_x = ((ev.button.x/SQUARE_SIZE));
                cells[mouse_cell_y][mouse_cell_x].alive = !cells[mouse_cell_y][mouse_cell_x].alive;
            }
        }
        SDL_Delay(1000/FPS);
        // SDL_Delay(1000);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
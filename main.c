#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "info.h"
#include <SDL3/SDL_surface.h>
/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *playerTexture;
SDL_FRect playerbody[1];
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
void resetPlayer() {
    playerbody[0].x =0;
    playerbody[0].y =0;
    playerbody[0].w =20;
    playerbody[0].h =20;
}
void checkForInputs() {
	const bool *key_states = SDL_GetKeyboardState(NULL);
	if (key_states[SDL_SCANCODE_S]) {
        	playerbody[0].y += .01;  
        } 
	
	if (key_states[SDL_SCANCODE_W]) {
        	playerbody[0].y -= .01;  
	}
	if (key_states[SDL_SCANCODE_D]) {
        	playerbody[0].x += .01;  
	}
	if (key_states[SDL_SCANCODE_A]) {
        	playerbody[0].x -= .01;  
	}
	
}
SDL_AppResult loadTextures() {
    char * bmp_path = NULL;
    SDL_Surface *surface = NULL;
    SDL_asprintf(&bmp_path, "%simages/character.bmp", SDL_GetBasePath());  /* allocate a string of the full file path */
    surface = SDL_LoadBMP(bmp_path);
    if (!surface) {
        SDL_Log("Couldn't load bitmap: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_free(bmp_path);  /* done with this, the file is loaded. */

      surface->w = 20;
      surface->h = 20;

    playerTexture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!playerTexture) {
        SDL_Log("Couldn't create static texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_DestroySurface(surface);  /* done with this, the texture has a copy of the pixels now. */
    return SDL_APP_CONTINUE;

}
/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata(TITLE, VERSION, NAMESPACE);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    loadTextures();
    resetPlayer();
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    switch(event->type) {
    	case  SDL_EVENT_QUIT:
        	return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
	break;
    }
    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    checkForInputs();    
    SDL_SetRenderDrawColor(renderer, 210, 180, 140, SDL_ALPHA_OPAQUE);  
    SDL_RenderClear(renderer);  /* start with a blank canvas. */
    SDL_SetRenderDrawColor(renderer, 0,0,0, SDL_ALPHA_OPAQUE);
    //SDL_RenderFillRect(renderer, playerbody);
    SDL_RenderTexture(renderer, playerTexture, NULL, playerbody);
    SDL_RenderPresent(renderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}

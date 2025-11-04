#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "info.h"
#include <SDL3/SDL_surface.h>
#include <math.h>
/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *playerTexture;
SDL_FRect playerBody[1];
int playerRotation = 0;
SDL_FRect walls[1];
static SDL_Texture *wallTexture;

float mouseX,mouseY = 0;
int windowW = 640;
int windowH = 480;
void resetPlayer() {
    playerBody[0].x =0;
    playerBody[0].y =0;
    playerBody[0].w =20;
    playerBody[0].h =20;
}
void generateWalls() {
	walls->x =100;
	walls->y =150;
	walls->w =50;
	walls->h =50;
}
	
void checkForInputs() {
	const bool *key_states = SDL_GetKeyboardState(NULL);
	SDL_FRect intersect[1];
	if (key_states[SDL_SCANCODE_S]) {
        	playerBody[0].y += .01;  
        } 
	
	if (key_states[SDL_SCANCODE_W]) {
        	playerBody[0].y -= .01;  
	}	
	if (key_states[SDL_SCANCODE_D]) {
        	playerBody[0].x += .01;  
	}
	if (key_states[SDL_SCANCODE_A]) {
        	playerBody[0].x -= .01;  
	}
	if(key_states[SDL_SCANCODE_ESCAPE]) {
		SDL_SetWindowMouseGrab(window,false);
		SDL_CaptureMouse(false);
	}
	//Move back player by how far they intersected on their respective side
	if(SDL_GetRectIntersectionFloat(playerBody, walls, intersect)) {
		if(intersect->w < intersect->h) {
			if(intersect->x > playerBody->x) {
				playerBody->x -= intersect->w;
			}
			//invert for opposite sides
			else {
				playerBody->x += intersect->w;
			}
		}
		else {
			if(intersect->y > playerBody->y) {
				playerBody->y -= intersect->h;
			}
			//invert for opposite sides
			else {
				playerBody->y += intersect->h;
			}
		}

	}
	SDL_GetMouseState(&mouseX, &mouseY);
}
int getRotationRelativeToPoint(int x, int y, int a, int b) {
return (atan2((y-b), x-a)*180.0000)/3.1416;
}
SDL_AppResult loadTextureFromBMP(SDL_Texture **texture, float w, float h, char * path) {
    char * bmp_path = NULL;
    SDL_Surface *surface = NULL;
    SDL_asprintf(&bmp_path, path, SDL_GetBasePath());  /* allocate a string of the full file path */
    surface = SDL_LoadBMP(bmp_path);
    if (surface == NULL) {
        SDL_Log("Couldn't load bitmap: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_free(bmp_path);  /* done with this, the file is loaded. */

      surface->w = w;
      surface->h = h;

    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (*texture == NULL) {
        SDL_Log("Couldn't create static texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_DestroySurface(surface);  /* done with this, the texture has a copy of the pixels now. */
    return SDL_APP_CONTINUE;

}
SDL_AppResult loadTextures() {
	if(loadTextureFromBMP(&playerTexture, 20,20, "%simages/character.bmp") != SDL_APP_CONTINUE) {
		return SDL_APP_FAILURE;	
	}
	
	if(loadTextureFromBMP(&wallTexture, 10,10, "%simages/wallTile.bmp") != SDL_APP_CONTINUE) {
		return SDL_APP_FAILURE;	
	}
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

    if (!SDL_CreateWindowAndRenderer(TITLE, windowW, windowW, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GetWindowSize(window,&windowW,&windowH);
    SDL_SetRenderLogicalPresentation(renderer, windowW, windowH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    generateWalls();
    if(loadTextures() != SDL_APP_CONTINUE) {
    	return SDL_APP_FAILURE;
    }
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
	case SDL_EVENT_WINDOW_FOCUS_GAINED:
		SDL_SetWindowMouseGrab(window, true);
	break;
	case SDL_EVENT_WINDOW_RESIZED:
		SDL_GetWindowSize(window,&windowW,&windowH);
		SDL_SetRenderLogicalPresentation(renderer, windowW, windowH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	break;
    }
    return SDL_APP_CONTINUE; /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    checkForInputs();    
    playerRotation = getRotationRelativeToPoint(playerBody->x+(playerBody->w/2),playerBody->y+(playerBody->h/2), mouseX, mouseY);
    SDL_SetRenderDrawColor(renderer, 210, 180, 140, SDL_ALPHA_OPAQUE);  
    SDL_RenderClear(renderer);  /* start with a blank canvas. */
    SDL_SetRenderDrawColor(renderer, 0,0,0, SDL_ALPHA_OPAQUE);
    SDL_SetTextureBlendMode(wallTexture, SDL_BLENDMODE_NONE);
    SDL_RenderTextureTiled(renderer, wallTexture,NULL,1, walls);
    SDL_RenderTextureRotated(renderer,playerTexture,NULL,playerBody,playerRotation,NULL,SDL_FLIP_NONE);
    SDL_RenderPresent(renderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#define PI 0x1.921fb6p+1f
#define BULLET_LIMIT 1000
#define BULLET_COOLDOWN 100
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "info.h"
#include <SDL3/SDL_surface.h>
/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
SDL_FRect walls[1];
static SDL_Texture *wallTexture;
static SDL_Texture *bulletTexture;
float mouseX,mouseY = 0;
int windowW = 640;
int windowH = 480;
typedef struct player {
	SDL_Texture *texture;
	SDL_FRect boundingBox; 
	int rotation;
	int bulletfiredt; // Last time this player fired a bullet
}player;
struct player mainPlayer;
unsigned int bulletc =0; /* Total bullets in game */
unsigned int bulletn =0; /* Next index in bullet array that we will write to */ 
typedef struct bullet {
	SDL_FRect rect;
	float velocityX;
	float velocityY;
	float velocityLoss;
	float rotation;
}bullet;
struct bullet *bullets;
void updateBullet(struct bullet *bullet) {
	if(bullet->velocityX ==0 && bullet->velocityY ==0){bullet->rect.x=3000000; return;} /* To make this code simpler (and ultimately more robust)
	just move this out of the way */

	if(bullet->velocityLoss > SDL_fabs(bullet->velocityX)) {
		bullet->velocityX =0;
	}
	else {
		bullet->rect.x += bullet->velocityX;
		if(bullet->velocityX < 0) {
			bullet->velocityX += bullet->velocityLoss;
		} 
		else {
			bullet->velocityX -= bullet->velocityLoss;
		}
	}
	if(bullet->velocityLoss > SDL_fabs(bullet->velocityY)) {
		bullet->velocityY =0;
	}
	else {
		bullet->rect.y += bullet->velocityY;
		if(bullet->velocityY < 0) {
			bullet->velocityY += bullet->velocityLoss;
		} 
		else {
			bullet->velocityY -= bullet->velocityLoss;
		}
	}
	}
void fireBullet() {
			mainPlayer.bulletfiredt = SDL_GetTicks(); 
			bullets[bulletn].rect.x = mainPlayer.boundingBox.x+(mainPlayer.boundingBox.w/2);
			bullets[bulletn].rect.y = mainPlayer.boundingBox.y+(mainPlayer.boundingBox.h/2);
			bullets[bulletn].rect.w = 5;
			bullets[bulletn].rect.h = 10;
			bullets[bulletn].velocityX = -1*SDL_cos(mainPlayer.rotation*PI/180)/60;
			bullets[bulletn].velocityY = -1*SDL_sin(mainPlayer.rotation*PI/180)/60;
			bullets[bulletn].velocityLoss = 0.000001;
			bullets[bulletn].rotation = mainPlayer.rotation;
			/* Prevent from writing memory past the limit next time
			if this is attempted, begin overwriting the bullets in order of first to last */
			if(bulletc < BULLET_LIMIT) {bulletc++; bulletn=bulletc;}
			if(bulletn >= BULLET_LIMIT-1) bulletn=0;
			else bulletn++;

			
}
void resetPlayer() {
    mainPlayer.boundingBox.x =0;
    mainPlayer.boundingBox.y =0;
    mainPlayer.boundingBox.w =20;
    mainPlayer.boundingBox.h =20;
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
        	mainPlayer.boundingBox.y += .01;  
        } 
	
	if (key_states[SDL_SCANCODE_W]) {
        	mainPlayer.boundingBox.y -= .01;  
	}	
	if (key_states[SDL_SCANCODE_D]) {
        	mainPlayer.boundingBox.x += .01;  
	}
	if (key_states[SDL_SCANCODE_A]) {
        	mainPlayer.boundingBox.x -= .01;  
	}
	if(key_states[SDL_SCANCODE_ESCAPE]) {
		SDL_SetWindowMouseGrab(window,false);
		SDL_CaptureMouse(false);
	}
	if (key_states[SDL_SCANCODE_E] && (SDL_GetTicks()-mainPlayer.bulletfiredt) > BULLET_COOLDOWN) {
		fireBullet();
	}
	//Move back player by how far they intersected on their respective side
	if(SDL_GetRectIntersectionFloat(&mainPlayer.boundingBox, walls, intersect)) {
		if(intersect->w < intersect->h) {
			if(intersect->x > mainPlayer.boundingBox.x) {
				mainPlayer.boundingBox.x -= intersect->w;
			}
			//invert for opposite sides
			else {
				mainPlayer.boundingBox.x += intersect->w;
			}
		}
		else {
			if(intersect->y > mainPlayer.boundingBox.y) {
				mainPlayer.boundingBox.y -= intersect->h;
			}
			//invert for opposite sides
			else {
				mainPlayer.boundingBox.y += intersect->h;
			}
		}

	}
	SDL_GetMouseState(&mouseX, &mouseY);
}
int getRotationRelativeToPoint(int x, int y, int a, int b) {
return (SDL_atan2((y-b), x-a)*180.0000)/PI;
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
	if(loadTextureFromBMP(&mainPlayer.texture, 20,20, "%simages/character.bmp") != SDL_APP_CONTINUE) {
		return SDL_APP_FAILURE;	
	}
	
	if(loadTextureFromBMP(&wallTexture, 10,10, "%simages/wallTile.bmp") != SDL_APP_CONTINUE) {
		return SDL_APP_FAILURE;	
	}
	if(loadTextureFromBMP(&bulletTexture, 10,5, "%simages/bullet.bmp") != SDL_APP_CONTINUE) {
		return SDL_APP_FAILURE;	
	}

	return SDL_APP_CONTINUE;
}
/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    bullets = SDL_malloc(sizeof(struct bullet)*BULLET_LIMIT);
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
    mainPlayer.rotation = getRotationRelativeToPoint(mainPlayer.boundingBox.x+(mainPlayer.boundingBox.w/2),mainPlayer.boundingBox.y+(mainPlayer.boundingBox.h/2), mouseX, mouseY);
    SDL_SetRenderDrawColor(renderer, 210, 180, 140, SDL_ALPHA_OPAQUE);  
    SDL_RenderClear(renderer);  /* start with a blank canvas. */
    SDL_SetRenderDrawColor(renderer, 0,0,0, SDL_ALPHA_OPAQUE);
    SDL_RenderTextureTiled(renderer, wallTexture,NULL,1, walls);
    SDL_SetTextureColorMod(mainPlayer.texture, 0,0,255);
    SDL_SetTextureColorMod(bulletTexture, 0,0,255);
    SDL_RenderTextureRotated(renderer,mainPlayer.texture,NULL,&mainPlayer.boundingBox,mainPlayer.rotation,NULL,SDL_FLIP_NONE);
    if(bulletc > 0) {
		for(int i=0; i<bulletc; ++i){
			updateBullet(&bullets[i]);
			SDL_RenderTextureRotated(renderer,bulletTexture,NULL,&bullets[i].rect,bullets[i].rotation+90,NULL,SDL_FLIP_NONE);
		}
	}
    SDL_RenderPresent(renderer);  /* put it all on the screen! */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}

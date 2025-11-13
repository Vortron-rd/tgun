#define SDL_MAIN_USE_CALLBACKS 1	/* use the callbacks instead of main() */
#define PI 0x1.921fb6p+1f
#define BULLET_LIMIT 1000
#define BULLET_COOLDOWN 100
#define BULLET_DAMAGE 25
#define MOB_LIMIT 3
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "info.h"
#include <SDL3/SDL_surface.h>
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
SDL_FRect walls[1];
static SDL_Texture *wallTexture;
static SDL_Texture *bulletTexture;
float mouseX, mouseY = 0;
int windowW = 640;
int windowH = 480;
/* Moving entity that can die and fire bullets */
typedef struct mob {
	SDL_Texture *texture;	/* Texture to be rendered */
	SDL_FRect boundingBox;	/* Rect to be used as a bounding box for physics */
	int rotation;
	int bulletfiredt;	// Last time this mob fired a bullet
	int color[3];		/* RGB color modulation for its texture */
	int health;
	unsigned int index;
} mob;
struct mob mainPlayer;		/* Struct for player character */
struct mob *mobs;		/* Holds data for mobs other than mainPlayer */
unsigned int mobc = 0;		/* Total Mobs in game (Excluding mainPlayer) */
unsigned int bulletc = 0;	/* Total bullets in game */
/* Store info about a single bullet */
typedef struct bullet {
	SDL_FRect rect;		/* Bounding box for Physics */
	float velocityX;
	float velocityY;
	float velocityLoss;	/* How much is subtracted from velocityX and velocityY each update */
	float rotation;
	unsigned int index;
} bullet;
struct bullet *bullets;		/* Holds data for all bullets in-game */

/* Handles re-organizing the array everytime a mob is killed */
void killMob(struct mob *mob)
{
	/* For each mob above the one we killed, move it's index (variable and array index) back by one */
	for (int i = mob->index + 1; i < mobc - mob->index; i++) {
		mobs[i - 1] = mobs[i];
		mobs[i - 1].index--;
	}
	mobc--;
}

/* Function to eventually hold all of damage logic */
int attack(struct mob *mob, int damage)
{
	mob->health -= damage;
	if (mob->health <= 0)
		killMob(mob);
	return damage;		/* Although it is the same now, eventually damage dealt != weapon damage */
}
/* Handles re-organizing the array everytime a mob is killed */
void killBullet(struct bullet *bullet)
{
	/* For each bullet above the one we killed, move it's index (variable and array index) back by one */
	for (int i = bullet->index + 1; i < bulletc - bullet->index; i++) {
		bullet[i - 1] = bullet[i];
		bullet[i - 1].index--;
	}
	bulletc--;
}

/* Update bullet's position based on velocity */
void updateBullet(struct bullet *bullet)
{

	if (bullet->velocityX == 0 && bullet->velocityY == 0) {
		killBullet(bullet);
		return;
	}
	if (bullet->velocityLoss > SDL_fabs(bullet->velocityX)) {
		bullet->velocityX = 0;
	} else {
		bullet->rect.x += bullet->velocityX;
		if (bullet->velocityX < 0) {
			bullet->velocityX += bullet->velocityLoss;
		} else {
			bullet->velocityX -= bullet->velocityLoss;
		}
	}
	if (bullet->velocityLoss > SDL_fabs(bullet->velocityY)) {
		bullet->velocityY = 0;
	} else {
		bullet->rect.y += bullet->velocityY;
		if (bullet->velocityY < 0) {
			bullet->velocityY += bullet->velocityLoss;
		} else {
			bullet->velocityY -= bullet->velocityLoss;
		}
	}
	for (int i = 0; i < mobc; i++) {
		if (SDL_HasRectIntersectionFloat(&bullet->rect, &mobs[i].boundingBox)) {
			killBullet(bullet);
			attack(&mobs[i], BULLET_DAMAGE);
			SDL_Log("Shot mob %d, health:%d", i, mobs[i].health);
			//destroyBullet(*bullet);
		}

	}

}

/* Fire bullet from the player in the direction they are facing */
void fireBullet()
{
	if(bulletc >= BULLET_LIMIT) {
		return;
	}
	mainPlayer.bulletfiredt = SDL_GetTicks();
	bullets[bulletc].rect.x = mainPlayer.boundingBox.x + (mainPlayer.boundingBox.w / 2);
	bullets[bulletc].rect.y = mainPlayer.boundingBox.y + (mainPlayer.boundingBox.h / 2);
	bullets[bulletc].rect.w = 5;
	bullets[bulletc].rect.h = 10;
	bullets[bulletc].velocityX = -1 * SDL_cos(mainPlayer.rotation * PI / 180) / 60;
	bullets[bulletc].velocityY = -1 * SDL_sin(mainPlayer.rotation * PI / 180) / 60;
	bullets[bulletc].velocityLoss = 0.000001;
	bullets[bulletc].rotation = mainPlayer.rotation;
	bullets[bulletc].index = bulletc;
	bulletc++;
}

/* Set mainPlayer's initial values */
void resetPlayer()
{
	mainPlayer.boundingBox.x = 0;
	mainPlayer.boundingBox.y = 0;
	mainPlayer.boundingBox.w = 20;
	mainPlayer.boundingBox.h = 20;
	/* set RGB color for player, should be completely blue */
	mainPlayer.color[0] = 0;
	mainPlayer.color[1] = 0;
	mainPlayer.color[2] = 255;
	mainPlayer.health = 100;
}

/* For now, set hard-coded values for walls */
void generateWalls()
{
	walls->x = 100;
	walls->y = 150;
	walls->w = 50;
	walls->h = 50;
}

/* Set random intitial values for mobs */
void generateMobs()
{
	mobc = (SDL_rand(MOB_LIMIT) + 1);
	for (int i = 0; i < mobc; ++i) {
		mobs[i].boundingBox.x = SDL_rand(windowW);
		mobs[i].boundingBox.y = SDL_rand(windowH);
		mobs[i].boundingBox.w = 20;
		mobs[i].boundingBox.h = 20;
		mobs[i].rotation = 0;
		mobs[i].texture = mainPlayer.texture;
		/* set RGB color for mobs should be completely green for now */
		mobs[i].color[0] = 0;
		mobs[i].color[1] = 255;
		mobs[i].color[2] = 0;
		mobs[i].health = 100;
		mobs[i].index = i;
	}
}

/* check for player inputs and act accordingly */
void checkForInputs()
{
	const bool *key_states = SDL_GetKeyboardState(NULL);
	SDL_FRect intersect[1];
	/* move player according to WASD, but reject anymovements that would clip them off screen. */
	if (key_states[SDL_SCANCODE_W] && mainPlayer.boundingBox.y > 0)
		mainPlayer.boundingBox.y -= .01;
	if (key_states[SDL_SCANCODE_A] && mainPlayer.boundingBox.x > 0)
		mainPlayer.boundingBox.x -= .01;
	if (key_states[SDL_SCANCODE_S] && (mainPlayer.boundingBox.y + mainPlayer.boundingBox.h) < windowH)
		mainPlayer.boundingBox.y += .01;
	if (key_states[SDL_SCANCODE_D] && (mainPlayer.boundingBox.x + mainPlayer.boundingBox.w) < windowW)
		mainPlayer.boundingBox.x += .01;
	/* if BULLET_COOLDOWN time has passed since the last shot, shoot a bullet */
	if (key_states[SDL_SCANCODE_E] && (SDL_GetTicks() - mainPlayer.bulletfiredt) > BULLET_COOLDOWN)
		fireBullet();
	if (key_states[SDL_SCANCODE_ESCAPE]) {
		SDL_SetWindowMouseGrab(window, false);
		SDL_CaptureMouse(false);
	}
	//Move back player by how far they intersected a wall on their respective side, if they did
	if (SDL_GetRectIntersectionFloat(&mainPlayer.boundingBox, walls, intersect)) {
		if (intersect->w < intersect->h) {
			if (intersect->x > mainPlayer.boundingBox.x) {
				mainPlayer.boundingBox.x -= intersect->w;
			}
			//invert for opposite sides
			else {
				mainPlayer.boundingBox.x += intersect->w;
			}
		} else {
			if (intersect->y > mainPlayer.boundingBox.y) {
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

/* Wrapper function for SDL_atan2 to make code more readable and consistent */
int getRotationRelativeToPoint(int x, int y, int a, int b)
{
	return (SDL_atan2((y - b), x - a) * 180.0000) / PI;
}

/* Function to simplify the process of file->surface->texture */
SDL_AppResult loadTextureFromBMP(SDL_Texture **texture, float w, float h, char *path)
{
	char *bmp_path = NULL;
	SDL_Surface *surface = NULL;
	SDL_asprintf(&bmp_path, path, SDL_GetBasePath());	/* allocate a string of the full file path */
	surface = SDL_LoadBMP(bmp_path);
	if (surface == NULL) {
		SDL_Log("Couldn't load bitmap: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_free(bmp_path);	/* done with this, the file is loaded. */

	surface->w = w;
	surface->h = h;

	*texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (*texture == NULL) {
		SDL_Log("Couldn't create static texture: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_DestroySurface(surface);	/* done with this, the texture has a copy of the pixels now. */
	return SDL_APP_CONTINUE;

}

/* Wrapper function for error checking loadTextureFromBMP */
SDL_AppResult loadTextures()
{
	if (loadTextureFromBMP(&mainPlayer.texture, 20, 20, "%simages/character.bmp") != SDL_APP_CONTINUE) {
		return SDL_APP_FAILURE;
	}

	if (loadTextureFromBMP(&wallTexture, 10, 10, "%simages/wallTile.bmp") != SDL_APP_CONTINUE) {
		return SDL_APP_FAILURE;
	}
	if (loadTextureFromBMP(&bulletTexture, 10, 5, "%simages/bullet.bmp") != SDL_APP_CONTINUE) {
		return SDL_APP_FAILURE;
	}

	return SDL_APP_CONTINUE;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
	/* Allocate memory for some arrays; these will be used throughout the whole program, so there is no need to SDL_free() */
	bullets = SDL_malloc(sizeof(struct bullet) * BULLET_LIMIT);
	mobs = SDL_malloc(sizeof(struct mob) * MOB_LIMIT);
	/* Set metadata based off of defines from info.h */
	SDL_SetAppMetadata(TITLE, VERSION, NAMESPACE);
	/* Initialize video */
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	/* Create window and renderer */
	if (!SDL_CreateWindowAndRenderer(TITLE, windowW, windowW, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	/* Get Window sizes as the OS may have changed them */
	SDL_GetWindowSize(window, &windowW, &windowH);
	/* Set renderer values */
	SDL_SetRenderLogicalPresentation(renderer, windowW, windowH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	/* Load textures into memory */
	if (loadTextures() != SDL_APP_CONTINUE) {
		return SDL_APP_FAILURE;
	}
	/* Generate values for in-game objects */
	generateWalls();
	generateMobs();
	resetPlayer();
	return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	switch (event->type) {
	case SDL_EVENT_QUIT:
		/* end the program, reporting success to the OS. */
		return SDL_APP_SUCCESS;
		break;
	case SDL_EVENT_WINDOW_FOCUS_GAINED:
		/* Regain mouse grab on focus */
		SDL_SetWindowMouseGrab(window, true);
		break;
	case SDL_EVENT_WINDOW_RESIZED:
		/* Set windowW and windowH to the new size & Reset renderer to those values */
		SDL_GetWindowSize(window, &windowW, &windowH);
		SDL_SetRenderLogicalPresentation(renderer, windowW, windowH, SDL_LOGICAL_PRESENTATION_LETTERBOX);
		break;
	}
	return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
	/* Program logic to be ran per frame */
	checkForInputs();
	mainPlayer.rotation = getRotationRelativeToPoint(mainPlayer.boundingBox.x + (mainPlayer.boundingBox.w / 2), mainPlayer.boundingBox.y + (mainPlayer.boundingBox.h / 2), mouseX, mouseY);
	/* Refresh frame */
	SDL_SetRenderDrawColor(renderer, 210, 180, 140, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	/* Render Walls */
	SDL_RenderTextureTiled(renderer, wallTexture, NULL, 1, walls);
	/* Render mainPlayer */
	SDL_SetTextureColorMod(mainPlayer.texture, mainPlayer.color[0], mainPlayer.color[1], mainPlayer.color[2]);
	SDL_RenderTextureRotated(renderer, mainPlayer.texture, NULL, &mainPlayer.boundingBox, mainPlayer.rotation, NULL, SDL_FLIP_NONE);
	/* Render bullets with their color set to the mainPlayer's */
	SDL_SetTextureColorMod(bulletTexture, mainPlayer.color[0], mainPlayer.color[1], mainPlayer.color[2]);
	if (bulletc > 0) {
		for (int i = 0; i < bulletc; ++i) {
			updateBullet(&bullets[i]);
			SDL_RenderTextureRotated(renderer, bulletTexture, NULL, &bullets[i].rect, bullets[i].rotation + 90, NULL, SDL_FLIP_NONE);
		}
	}
	/* Render Mobs with modulated colors */
	for (int i = 0; i < mobc; ++i) {
		SDL_SetTextureColorMod(mobs[i].texture, mobs[i].color[0], mobs[i].color[1], mobs[i].color[2]);
		SDL_RenderTextureRotated(renderer, mobs[i].texture, NULL, &mobs[i].boundingBox, mobs[i].rotation, NULL, SDL_FLIP_NONE);
	}
	/* Push render buffer to screen */
	SDL_RenderPresent(renderer);

	return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}

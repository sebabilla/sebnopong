#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "pong_libraries.h"

//------------------Fonctions pour vérifier que ça tourne rond----------

int VerificationSauvegarde(const char *s)
{
	FILE *sauvegarde;
	if (sauvegarde = fopen(s, "r"))
	{
		printf("Sauvegarde trouvee\n");
		fclose(sauvegarde);
		return 0;
	}
		
	sauvegarde = fopen(s, "w");
	if (sauvegarde == NULL)
	{
		printf("impossible de creer une sauvegarde\n");
		exit(1);
	}
	fprintf(sauvegarde, "%d", 0);
	printf("sauvegarde creee\n");
	fclose(sauvegarde);
	return 0;
}

void SDL_ExitWithError(const char *message)
{
	SDL_Log("ERREUR : %s > %s\n", message, SDL_GetError());
	SDL_Quit();
	exit(EXIT_FAILURE);
}

void limit_fps(unsigned int limit)
{
	unsigned t = SDL_GetTicks();
	
	if (limit < t)
		return; // laisser tourner l'affichage
	else if (limit > t + FPS)
		SDL_Delay(FPS); // faire une pause
	else
		SDL_Delay(limit - t); // faire une petite pause
}

//------------------Fonctions gérant la SDL-----------------------------

Mix_Music *ChargementBruitage(const char *lien)
{
Mix_Music *s = Mix_LoadMUS(lien);
	if (s == NULL)
	{
		Mix_CloseAudio();
		SDL_ExitWithError("chargement son");
	}
	return s;
}

SDL_Texture *NouvelleTexture(SDL_Window *w, SDL_Renderer *r, char *lien)
{
	SDL_Surface *image;
	SDL_Texture *texture;
	
	if ((image = SDL_LoadBMP(lien)) == NULL)
	{
		SDL_DestroyRenderer(r);
		SDL_DestroyWindow(w);
		SDL_ExitWithError("Impossible de charger l'image");
	}
	
	if ((texture = SDL_CreateTextureFromSurface(r, image)) == NULL)
	{
		SDL_FreeSurface(image);
		SDL_DestroyRenderer(r);
		SDL_DestroyWindow(w);
		SDL_ExitWithError("Impossible de charger l'image");
	}
	SDL_FreeSurface(image);
	return texture;
}

void NouveauTerrain(SDL_Renderer *r, SDL_Rect *rinfo, SDL_Rect *rterrain)
{
	if (SDL_RenderClear(r) != 0) 
		SDL_ExitWithError("Liberation du rendu echouee");
		
	if (SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE) != 0)
		SDL_ExitWithError("Impossible de changer la couleur du rendu");
		
	if (SDL_RenderFillRect(r, rinfo) != 0)
		SDL_ExitWithError("Impossible de dessiner un rectangle");
		
	if (SDL_SetRenderDrawColor(r, 100, 0, 0, SDL_ALPHA_TRANSPARENT) != 0)
		SDL_ExitWithError("Impossible de changer la couleur du rendu");
		
	if (SDL_RenderFillRect(r, rterrain) != 0)
		SDL_ExitWithError("Impossible de dessiner un rectangle");
	
	if (SDL_SetRenderDrawColor(r, 255, 255, 255, SDL_ALPHA_TRANSPARENT) != 0)
		SDL_ExitWithError("Impossible de changer la couleur du rendu");
		
	if (SDL_RenderDrawLine(r, WINDOW_WIDTH/2, 60, WINDOW_WIDTH/2, WINDOW_HEIGHT - 20) != 0)
		SDL_ExitWithError("Impossible de dessiner une ligne");
}

void EcrireTexte(SDL_Renderer *r, char *texte, TTF_Font *f, int X, int W)
{
	SDL_Color White = {255, 255, 255};									
	SDL_Surface *surfaceMessage = TTF_RenderText_Blended(f, texte, White);
	SDL_Texture *Message = SDL_CreateTextureFromSurface(r, surfaceMessage);
	SDL_Rect MessageRect = {.x = X, .y = 5, .w = W, .h = 20};
	SDL_RenderCopy(r, Message, NULL, &MessageRect);
	SDL_FreeSurface(surfaceMessage);
	SDL_DestroyTexture(Message);
}

void AffichageBalle(SDL_Renderer *r, Balle *b, unsigned int color)
{
	if (SDL_SetRenderDrawColor(r, color, color, 0, SDL_ALPHA_TRANSPARENT) != 0)
		SDL_ExitWithError("Impossible de changer la couleur du rendu");
	if (SDL_RenderFillRect(r, &b->rect_verticale) != 0)
		SDL_ExitWithError("Impossible de dessiner un rectangle");
	if (SDL_RenderFillRect(r, &b->rect_horizontale) != 0)
		SDL_ExitWithError("Impossible de dessiner un rectangle");
	return;
}

SDL_bool AffichageRaquette(SDL_Renderer *r, Raquette *rq, SDL_Texture *text_player, SDL_Texture *text_racket)
{
	if (SDL_RenderCopy(r, text_player, NULL, &rq->rect_joueur) != 0) // copier dans le rendu l'image en memoire
		return SDL_FALSE;
	
	if (SDL_RenderCopy(r, text_racket, NULL, &rq->rect_raquette) != 0) // copier dans le rendu l'image en memoire
		return SDL_FALSE;	

	return SDL_TRUE;
}

void AffichageMenu(SDL_Renderer *r, SDL_Rect *rmenu, TTF_Font *f)
{
	if (SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE) != 0)
		SDL_ExitWithError("Impossible de changer la couleur du rendu");
	if (SDL_RenderFillRect(r, rmenu) != 0)
		SDL_ExitWithError("Impossible de dessiner un rectangle");
	SDL_Color White = {0, 255, 255};									
	SDL_Surface *surfaceMessage = TTF_RenderText_Blended(f, GAME_TITLE, White);
	SDL_Texture *Message = SDL_CreateTextureFromSurface(r, surfaceMessage);
	SDL_RenderCopy(r, Message, NULL, rmenu);
	SDL_FreeSurface(surfaceMessage);
	SDL_DestroyTexture(Message);
	return;
}

//------------------Fonctions gérant les raquettes----------------------

Raquette *NouvelleRaquette(GaucheDroite gd)
{
	Raquette *rq = malloc(sizeof(Raquette));
	if (rq == NULL)
	{
		free(rq);
		printf("Erreur d'allocation mémoire\n");
		return NULL;
	}
	rq->yh = (WINDOW_HEIGHT/2) -10;
	rq->yb = (WINDOW_HEIGHT/2) + 50;
	rq->hb = IMMOBILE;
	rq->gd = gd;
	rq->vitesse = 5;
	rq->rect_joueur.w = 34;
	rq->rect_joueur.h = 50;
	rq->rect_raquette.w = 8;
	rq->rect_raquette.h = 60;
	if (gd == LEFT)
	{
		rq->x = 40;
		rq->rect_joueur.x = rq->x - 33;
		rq->rect_joueur.y = rq->yh - 50;
		rq->rect_raquette.x = rq->x - 7;
		rq->rect_raquette.y = rq->yh;
	}
	else
	{
		rq->x = WINDOW_WIDTH - 40;
		rq->rect_joueur.x = rq->x - 1;
		rq->rect_joueur.y = rq->yh + 60;
		rq->rect_raquette.x = rq->x - 1;
		rq->rect_raquette.y = rq->yh;
	}
	return rq;
}

void MouvementRaquette(Raquette *rq)
{
	switch(rq->hb)
	{
		case UP:
			if (rq->yh < 30)
				return;
			rq->yh -= rq->vitesse;
			rq->yb -= rq->vitesse;
			break;
		case DOWN:
			if (rq->yb > WINDOW_HEIGHT + 10)
				return;
			rq->yh += rq->vitesse;
			rq->yb += rq->vitesse;
			break;	
		case IMMOBILE:
			break;
		default:
			printf("ERREUR la raquette n'a pas de direction\n");
			exit(EXIT_FAILURE);
	}
	if (rq->gd == LEFT)
	{
		rq->rect_joueur.y = rq->yh - 50;
		rq->rect_raquette.y = rq->yh;
	}
	if (rq->gd == RIGHT)
	{
		rq->rect_joueur.y = rq->yh + 60;
		rq->rect_raquette.y = rq->yh;
	}
	return;
}

void IntelligenceRaquette(Raquette *rq, Balle *b)
{
	if (b->x < WINDOW_WIDTH/6)
	{
		rq->hb = IMMOBILE;
		return;
	}
	if ((b->vx < 0) && (b->x < WINDOW_WIDTH - (WINDOW_WIDTH/4)) && (b->x > WINDOW_WIDTH/4))
	{
		rq->hb = IMMOBILE;
		return;
	}
	if (rq->yh + 5 > b->y)
		rq->hb = UP;
	else if (rq->yh + 55 < b->y)
		rq->hb = DOWN;
	else
		rq->hb = IMMOBILE;
	return;
}

//------------------Fonctions gérant la balle---------------------------

Balle *NouvelleBalle(void)
{
	Balle *b = malloc(sizeof(Balle));
	if (b == NULL)
	{
		free(b);
		printf("Erreur d'allocation mémoire\n");
		return NULL;
	}
	b->x = WINDOW_WIDTH/2; 
	b->y = WINDOW_HEIGHT/2 + 20;
	b->rect_verticale.w = 7;
	b->rect_verticale.h = 11;
	b->rect_horizontale.w = 11;
	b->rect_horizontale.h = 7;
	b->vx = -2 - (float)rand() / 2 /RAND_MAX;
	b->vy = -2.5 - b->vx;
	
	return b;
}

void MouvementBalle(Balle *b)
{
	b->x += b->vx;
	b->y += b->vy; 
	b->rect_verticale.x = b->x - 3;
	b->rect_verticale.y = b->y - 5;
	b->rect_horizontale.x = b->x - 5;
	b->rect_horizontale.y = b->y - 3;
	return;
}

int TestCollision(Balle *b, Raquette *RqG, Raquette *RqD)
{
	if (b->vx < 0)
	{
		if (b->x > RqG->x)
			if (b->y > 40 && b->y < WINDOW_HEIGHT)
				return 1; //continue
			else
				return 2; //collision mur
		else if ((b->y > RqG->yh) && (b->y < RqG->yb))
			return 3; //collision raquette
		else
			return 0; //outside
	}
	else if (b->vx > 0)
	{
		if (b->x < RqD->x)
			if (b->y > 40 && b->y < WINDOW_HEIGHT)
				return 1; //continue
			else
				return 2; // collision mur
		else if ((b->y > RqD->yh) && (b->y < RqD->yb))
			return 3; //collision raquette
		else
			return 0; //outside
	}
	else
		return 0;
}

void MultiplicationAngle(Balle *b, float angle)
{
	float tmpx = b->vx;
	float tmpy = b->vy;
	b->vx = cos(angle)*tmpx - sin(angle)*tmpy;
	b->vy = -sin(angle)*tmpx - cos(angle)*tmpy;
	
	return;
}

void ModificationDirection(Balle *b, Raquette *RqG, Raquette *RqD, int c)
{
	b->vx *= 1.1;
	if (b->vx < 0)
	{
		if (RqG->hb == IMMOBILE)
		{
			b->vx = -b->vx;		
			return;
		}
		if (RqG->hb == UP)
		{
			MultiplicationAngle(b, -PI*9/10);
			return;
		}
		if (RqG->hb == DOWN)
		{
			MultiplicationAngle(b, PI*9/10);
			return;
		}
	}
	if (b->vx > 0)
	{
		if (RqD->hb == IMMOBILE)
		{
			b->vx = -b->vx;		
			return;
		}
		if (RqD->hb == UP)
		{
			MultiplicationAngle(b, -PI*9/10);
			return;
		}
		if (RqD->hb == DOWN)
		{
			MultiplicationAngle(b, PI*9/10);
			return;
		}
	}
	return;
}

void Rebond(Balle *b)
{
	b->vy = -b->vy;
	return;
}

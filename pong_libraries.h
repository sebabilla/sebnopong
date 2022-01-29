#ifndef __PONG_LIBRARIES__H__
#define __PONG_LIBRARIES__H__

#define GAME_TITLE "Seb no Pong in C"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FPS 16 // 60 FPS, remplacer par 33 pour 30 FPS
#define PI 3.14

//------------------Enumérations et structures--------------------------

typedef enum GaucheDroite
{
	LEFT, RIGHT, VERT
} GaucheDroite;

typedef enum HautBas
{
	UP, DOWN, IMMOBILE
} HautBas;

typedef enum Directions
{
	HAUT, BAS, HORIZ, DIAGHAUT, DIAGBAS
} Directions;

typedef struct Balle
{
	float x;
	float y;
	float vx;
	float vy;
	SDL_Rect rect_verticale;
	SDL_Rect rect_horizontale;
} Balle;

typedef struct Raquette
{
	signed int x;
	signed int yh; // haut et bas de la raquette pour teste de collision
	signed int yb;
	HautBas hb;
	GaucheDroite gd;
	signed int vitesse;
	SDL_Rect rect_joueur;
	SDL_Rect rect_raquette;
} Raquette;

//------------------Fonctions pour vérifier que ça tourne rond----------

int VerificationSauvegarde(const char *s);
void SDL_ExitWithError(const char *message); // Copiée telle quelle de Jason Champagne @ https://www.youtube.com/c/Formationvidéo8
void limit_fps(unsigned int limit); // Copiée telle quelle de Jason Champagne @ https://www.youtube.com/c/Formationvidéo8

//------------------Fonctions gérant la SDL-----------------------------

Mix_Music *ChargementBruitage(const char *lien);
SDL_Texture *NouvelleTexture(SDL_Window *w, SDL_Renderer *r, char *lien);
void NouveauTerrain(SDL_Renderer *r, SDL_Rect *rinfo, SDL_Rect *rterrain);
void EcrireTexte(SDL_Renderer *r, char *texte, TTF_Font *f, int X, int W);
void AffichageBalle(SDL_Renderer *r, Balle *b, unsigned int color);
SDL_bool AffichageRaquette(SDL_Renderer *r, Raquette *rq, SDL_Texture *text_player, SDL_Texture *text_racket);
void AffichageMenu(SDL_Renderer *r, SDL_Rect *rmenu, TTF_Font *f);

//------------------Fonctions gérant les raquettes----------------------

Raquette *NouvelleRaquette(GaucheDroite gd);
void MouvementRaquette(Raquette *rq); // actualise la position
void IntelligenceRaquette(Raquette *rq, Balle *b); // adversaire

//------------------Fonctions gérant la balle---------------------------

Balle *NouvelleBalle(void);
void MouvementBalle(Balle *b); // actualise la position
int TestCollision(Balle *b, Raquette *RqG, Raquette *RqD);
void MultiplicationAngle(Balle *b, float angle); // sur la raquette plus fun qu'une simple réflexion
void ModificationDirection(Balle *b, Raquette *RqG, Raquette *RqD, int c);
void Rebond(Balle *b); // sur le mur

#endif

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "pong_libraries.h"


int main(int argc, char *argv[])
{
	SDL_bool program_launched = SDL_TRUE;

//------------------Chargement Sauvegarde-------------------------------	
	
	int meilleur;
	if (VerificationSauvegarde("conf/sauvegarde.txt") == 0)
	{
		FILE *sauvegarde = fopen("conf/sauvegarde.txt", "r");
		fscanf(sauvegarde, "%d", &meilleur);
		fclose(sauvegarde);
	}
	else
	{
		printf("Impossible de creer une sauvegarde");
		exit(1);
	}
	
//------------------Chargement de la police de texte--------------------

	if (TTF_Init() < 0)
		SDL_ExitWithError("Initiation TTF");
		
	TTF_Font *font = TTF_OpenFont("res/sans.ttf", 50);
	if (!font)
		SDL_ExitWithError("Chargement de la police"); 
	
//------------------Chargement des bruitages----------------------------

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) < 0)
	{
		Mix_CloseAudio();
		SDL_ExitWithError("Initiation son");
	}
		
	Mix_Music *petit_pong = ChargementBruitage("res/petit_pong.mp3");
	Mix_Music *pong_gauche = ChargementBruitage("res/pong_gauche.mp3");
	Mix_Music *pong_droit = ChargementBruitage("res/pong_droit.mp3");
	Mix_Music *defeat = ChargementBruitage("res/defeat.mp3");
	Mix_Music *crowd = ChargementBruitage("res/crowd.mp3");
	
//------------------Création d'une fenêtre et de son rendu--------------	
	
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		SDL_ExitWithError("Initiation SDL");
		
	if (SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer))
		SDL_ExitWithError("Impossible de creer la fenetre et le rendu");
	SDL_SetWindowTitle(window, GAME_TITLE);

//------------------Chargement des images-------------------------------

	SDL_Texture *text_character_blue = NULL;
	SDL_Texture *text_character_green = NULL;
	SDL_Texture *text_racket_haut = NULL;
	SDL_Texture *text_racket_bas = NULL;

	text_character_blue = NouvelleTexture(window, renderer,"res/character_blue.bmp");
	text_character_green = NouvelleTexture(window, renderer,"res/character_green.bmp");
	text_racket_haut = NouvelleTexture(window, renderer,"res/racket_haut.bmp");
	text_racket_bas = NouvelleTexture(window, renderer,"res/racket_bas.bmp");
	
//------------------Chargement d'un éventuel joystic--------------------

	if (SDL_Init(SDL_INIT_JOYSTICK) != 0)
		SDL_ExitWithError("Initiation SDL");
	
	SDL_Joystick *joy;
	if (SDL_NumJoysticks()>0)
		joy=SDL_JoystickOpen(0);
	else
		printf("Pas de joystick\n");

//------------------Définition des variables globales-------------------
			
	unsigned int ticks;
	unsigned int frame_limit = 0;
	
	SDL_Rect rect_info = {.x = 0, .y = 0, .w = WINDOW_WIDTH, .h = 40};
	SDL_Rect terrain = {.x = 0, .y = 40, .w = WINDOW_WIDTH, .h = WINDOW_HEIGHT - 40};
	SDL_Rect menu = {.x = WINDOW_WIDTH/2 - 200, .y = WINDOW_HEIGHT/2 - 100, .w = 400, .h = 200};
	
	Balle *balle = NULL;
	if ((balle = NouvelleBalle()) == NULL)
		program_launched = SDL_FALSE;
	Raquette *raquette_gauche = NULL;
	if ((raquette_gauche = NouvelleRaquette(LEFT)) == NULL)
		program_launched = SDL_FALSE;
	Raquette *raquette_droite = NULL;
	if ((raquette_droite = NouvelleRaquette(RIGHT)) == NULL)
		program_launched = SDL_FALSE;
	
	int coups = 0;
	char score[25];
	char top_score[10];
	int perdu = 0;
	int tempmeilleur = meilleur;
	
	char *recommencer = "Nouvelle partie: ENTREE, Pause: ESPACE";
	
	int bhaut = 0, bbas = 0, bpause = 1, bstart = 0;

//------------------Lancement du programme, boucle infinie--------------		
	
	while (program_launched)
	{
		
//------------------Détection des actions du joueur---------------------
	
		SDL_Event event;		
		if (SDL_PollEvent(&event)) 
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
					{
						case SDLK_DOWN:
							bbas = 1;
							continue;
						case SDLK_UP:
							bhaut = 1;
							continue;
						case SDLK_RETURN:
							bstart = 1;
							continue;
						case SDLK_SPACE:
							bpause = 1;
							continue;
						case SDLK_ESCAPE:
							program_launched = SDL_FALSE;
							break;
						default:				
							continue;
					}
				case SDL_KEYUP:
					switch(event.key.keysym.sym)
					{
						case SDLK_DOWN:
							bbas = 0;
							continue;
						case SDLK_UP:
							bhaut = 0;
							continue;						
						default:
							continue;
					}
				case SDL_JOYBUTTONDOWN:
					switch(event.jbutton.button)
					{
						case 7:
							bstart = 1;
							continue;
						case 0:
							bpause = 1;
							continue;
						case 6:
							program_launched = SDL_FALSE;;
							break;
						default:				
							continue;
					}
				case SDL_JOYAXISMOTION:
					if ((event.jaxis.value > -8000) && (event.jaxis.value < 8000))
					{
						bbas = 0;
						bhaut = 0;
						continue;
					}
					else if (event.jaxis.value > 0)
					{
						bbas = 1;
						continue;
					}
					else if (event.jaxis.value < 0)
					{
						bhaut = 1;
					continue;
					}				
				case SDL_QUIT: 
					program_launched = SDL_FALSE;
					break;			
				default:
					continue;
			}
		}
		
//------------------Transmission des actions----------------------------

		if (bbas == 0 && bhaut == 0)
			raquette_gauche->hb = IMMOBILE;
		else if (bhaut == 1)
		{
			raquette_gauche->hb = UP;
			MouvementRaquette(raquette_gauche);
		}
		else if (bbas == 1)
		{
			raquette_gauche->hb = DOWN;
			MouvementRaquette(raquette_gauche);
		}
		
		if (bstart == 1)
		{
			bstart = 0;
			coups = 0;
			perdu = 1;
		}		
//------------------Mouvement de l'adversaire---------------------------
		
		IntelligenceRaquette(raquette_droite, balle);
		MouvementRaquette(raquette_droite);

//------------------Calculer le rendu frame par frame-------------------
		
		NouveauTerrain(renderer, &rect_info, &terrain);

		sprintf(score, "Nombre de coups: %3d", coups);
		sprintf(top_score, "Top: %3d", tempmeilleur);
		EcrireTexte(renderer, score, font, 10, 180);
		EcrireTexte(renderer, recommencer, font, WINDOW_WIDTH/2 - 120, 240);
		EcrireTexte(renderer, top_score, font, WINDOW_WIDTH - 70, 60);
		
		if (!AffichageRaquette(renderer, raquette_gauche, text_character_blue, text_racket_bas))
			SDL_ExitWithError("Rendu Raquette");
		if (!AffichageRaquette(renderer, raquette_droite, text_character_green, text_racket_haut))
			SDL_ExitWithError("Rendu Raquette");
		
		if (bpause == 1)
		{
			Mix_PlayMusic(crowd, -1);
			AffichageMenu(renderer, &menu, font);
			SDL_RenderPresent(renderer);
			while (bpause == 1)
				{
					SDL_WaitEvent(&event);
						if (event.type == SDL_KEYDOWN || event.type == SDL_JOYBUTTONDOWN)
						{
							if (event.key.keysym.sym == SDLK_SPACE || event.jbutton.button == 0 || event.key.keysym.sym == SDLK_RETURN || event.jbutton.button == 7)
								bpause = 0;
							else if (event.key.keysym.sym == SDLK_ESCAPE)
							{
								bpause = 0;
								program_launched = SDL_FALSE;
							}
						}
						else if (event.type == SDL_QUIT) 
						{
							bpause = 0;
							program_launched = SDL_FALSE;
						}
				}
		Mix_HaltMusic();
		}
		else if (perdu == 0)
		{
			AffichageBalle(renderer, balle, 255);
			SDL_RenderPresent(renderer);
		}
		else
		{	
			for (int i = 1; i <= 5; i++)
			{
				AffichageBalle(renderer, balle, 255/(i));	
				SDL_RenderPresent(renderer);
				SDL_Delay(300);
			}
			free(balle);
			if ((balle = NouvelleBalle()) == NULL)
				break;
			perdu = 0;
		}		
			
		frame_limit = SDL_GetTicks() + FPS; 
		limit_fps(frame_limit);
		frame_limit = SDL_GetTicks() + FPS;
				

//------------------Destin de la balle----------------------------------

		switch(TestCollision(balle, raquette_gauche, raquette_droite))
		{
			case 1:
				MouvementBalle(balle);
				break;
			case 2:
				Mix_PlayMusic(petit_pong, 1);
				Rebond(balle);
				MouvementBalle(balle);
				break;
			case 3:
				coups++;
				if (balle->vx < 0)
					Mix_PlayMusic(pong_gauche, 1);
				else
					Mix_PlayMusic(pong_droit, 1);
				ModificationDirection(balle, raquette_gauche, raquette_droite, coups);
				MouvementBalle(balle);
				break;
			case 0:
				Mix_PlayMusic(defeat, 1);
				if (coups > tempmeilleur)
					tempmeilleur = coups;
				coups = 0;
				perdu = 1;
				break;
			default:
				break;
		}
	}	

//------------------Sauvegarder le score? ------------------------------

	if (tempmeilleur > meilleur)
	{
		FILE *sauvegarde = fopen("conf/sauvegarde.txt", "w");
		fprintf(sauvegarde, "%d", tempmeilleur);
		printf("sauvegarde mise a jour\n");
		fclose(sauvegarde);
	}
		
//------------------Libération de la mémoire et fermeture de la fenêtre-
	
	free(balle);
	free(raquette_gauche);
	free(raquette_droite);
	if (joy != NULL)
		SDL_JoystickClose(joy);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	Mix_FreeMusic(pong_droit);
	Mix_FreeMusic(pong_gauche);
	Mix_FreeMusic(petit_pong);
	Mix_FreeMusic(defeat);
	Mix_FreeMusic(crowd);
	Mix_CloseAudio();
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	
	return EXIT_SUCCESS;
}
	





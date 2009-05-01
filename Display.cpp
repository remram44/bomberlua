#include "Display.h"

#include <sstream>

void Display::init(const Engine *engine)
	throw (InitException)
{
	std::string err;
	if(SDL_Init(SDL_INIT_VIDEO))
    {
		err = "Erreur à l'initialisation de SDL : ";
		err += SDL_GetError ();
		throw InitException (err.c_str ());
	}

	if(SDL_SetVideoMode(engine->getMapWidth()*32, engine->getMapHeight()*32, 32,
        SDL_HWSURFACE | SDL_DOUBLEBUF) == NULL)
	{
		err = "Erreur à la création de la fenêtre : ";
		err += SDL_GetError ();
		SDL_Quit ();
		throw InitException (err.c_str ());
	}
	SDL_WM_SetCaption("BomberLua", NULL);

	try 
	{
		loadImage (get()->m_pBrickSurface, "./Images/Cells/Brick.png");
		loadImage (get()->m_pRockSurface, "./Images/Cells/Rock.png");
		loadImage (get()->m_pEmptySurface, "./Images/Cells/Empty.png");
		loadImage (get()->m_pBombermanSurface, "./Images/SFX/Bomberman.png");
		loadImage (get()->m_pBombSurface , "./Images/SFX/Bomb.png");
		loadImage (get()->m_pBoomSurface , "./Images/SFX/Wave.png");
	} 
	catch (InitException &e)
	{
		SDL_Quit ();
		throw e;
	}
}

void Display::loadImage (SDL_Surface *&surf, const char *path)
	throw (InitException)
{
	surf = IMG_Load (path);
	if (!surf) 
	{
		std::ostringstream oss ("");
		oss << "Erreur au chargement de : " << path << " : " << IMG_GetError ();
		throw InitException (oss.str().c_str ());
	}
}
void Display::update(const Engine *engine)
{
	const std::vector<Engine::ECell> map(engine->getMap());
	Display::get()->drawMapSurface(
        engine->getMapWidth(), engine->getMapHeight(), map);
	
	std::vector<const Engine::Bomb*> bombs(engine->getBombs());
	Display::get()->drawBombs(bombs);
	
	std::vector<const Engine::Bomber*> bombers(engine->getBombers());
	Display::get()->drawBombermen(bombers);
	
	const std::vector<double>& explosions(engine->getExplosions());
	Display::get()->drawExplosions(engine->getMapWidth(), engine->getMapHeight(), explosions);

	SDL_Flip(SDL_GetVideoSurface());
}

void Display::drawMapSurface(int width, int height, const std::vector<Engine::ECell> &map)
{
	SDL_Rect blitPos;
	blitPos.w = 32;
	blitPos.h = 32;
	blitPos.x = 0;
	blitPos.y = 0;
	SDL_Surface *screen = SDL_GetVideoSurface();
	for (int x=0; x<width; x++)
	{
		for (int y=0; y<height; y++)
		{
			blitPos.x = x*32;
			blitPos.y = y*32;
			
			switch (map[y * width + x])
			{
				case Engine::CELL_BRICK:
					SDL_BlitSurface(m_pBrickSurface, NULL, screen, &blitPos);
				break;
				
				case Engine::CELL_ROCK:
					SDL_BlitSurface(m_pRockSurface, NULL, screen, &blitPos);
				break;
				
				case Engine::CELL_EMPTY:
					SDL_BlitSurface(m_pEmptySurface, NULL, screen, &blitPos);
				break;
			}
		}
	}
}

void Display::drawBombermen(std::vector<const Engine::Bomber*> bombers)
{
	std::vector<const Engine::Bomber*>::iterator it = bombers.begin();
	for(; it != bombers.end(); it++)
	{
		if ((*it)->m_bAlive)
		{
			//On regarde le temps écoulé depuis le début de l'action pour en déduire la position et la frame du bomber.
			double timeElapsed = ((SDL_GetTicks()/1000.0) - (*it)->m_dBeginAction);
			int frameDrawed = (int)(timeElapsed/0.03125);
			//std::cerr << "Frame drawed :" << frameDrawed << "/" << timeElapsed << "\n";
			
			SDL_Rect blitPos;
			blitPos.w = 32;
			blitPos.h = 32;
			blitPos.x = (*it)->m_iPosX*32;//On place le bomber sur le terrain.
			blitPos.y = (*it)->m_iPosY*32;
	
			//std::cout << "Bomber pos : " << blitPos.x << "," << blitPos.y  << "(" << bombers[i]->m_iPosX << "," << bombers[i]->m_iPosY << ")" << std::endl;
	
			//Par défaut le bomber est immobile.
			SDL_Rect framePos;
			framePos.w = 32;
			framePos.h = 32;
			framePos.x = 0;
			framePos.y = 64;
			//On décale le bomber si il est en mouvement en fonction du temps écoulé.
			//On définit la frame dessinée via le temps écoulé.
			//Si il pose une bombe ou si il est immobile, on le fixe sur la case ou il est.
			switch ((*it)->m_eAction)
			{
				case Engine::ACT_MOV_LEFT:
					framePos.x = 32*frameDrawed;
					framePos.y = 96;
					blitPos.x -= 2*frameDrawed;
				break;
				case Engine::ACT_MOV_RIGHT:
					framePos.x = 32*frameDrawed;
					framePos.y = 32;
					blitPos.x += 2*frameDrawed;
				break;
				case Engine::ACT_MOV_UP:
					framePos.x = 32*frameDrawed;
					framePos.y = 0;
					blitPos.y -= 2*frameDrawed;
				break;
				case Engine::ACT_MOV_DOWN:
					framePos.x = 32*frameDrawed;
					framePos.y = 64;
					blitPos.y += 2*frameDrawed;
				break;
				case Engine::ACT_DROP_BOMB:
					framePos.x = 0;
					framePos.y = 64;
				break;
				case Engine::ACT_IDLE:
					framePos.x = 0;
					framePos.y = 64;
					break;
				default:
					framePos.x = 0;
					framePos.y = 64;
				break;
			}
			//On vérifie que la frame n'ai pas dépassé l'image.
			if (framePos.x > 480)
				framePos.x = 480;
			
			//std::cerr << "Frame pos : " << framePos.x << "," << framePos.y  << "-" << frameDrawed << "/" << timeElapsed << "\n";
			SDL_BlitSurface(m_pBombermanSurface, &framePos,
				SDL_GetVideoSurface(), &blitPos);
			
		}
	}
}

void Display::drawBombs(std::vector<const Engine::Bomb*> bombs)
{
	std::vector<const Engine::Bomb*>::iterator it = bombs.begin();
	for(; it != bombs.end(); it++)
	{
		//On regarde le temps restant pour en déduire la frame affichée.
		double remainingTime = ((*it)->m_dExplodeDate - SDL_GetTicks()/1000.0);
		int frameDrawed = (int)(remainingTime/1.33);
		
		SDL_Rect blitPos;
		blitPos.w = 32;
		blitPos.h = 32;
		blitPos.x = (*it)->m_iPosX*32;//On place le bomber sur le terrain.
		blitPos.y = (*it)->m_iPosY*32;

		//std::cout << "Bomber pos : " << blitPos.x << "," << blitPos.y  << "(" << bombers[i]->m_iPosX << "," << bombers[i]->m_iPosY << ")" << std::endl;

		//Par défaut le bomber est immobile.
		SDL_Rect framePos;
		framePos.w = 32;
		framePos.h = 32;
		framePos.x = 32*frameDrawed;
		framePos.y = 0;
		
		//std::cerr << "Frame pos : " << framePos.x << "," << framePos.y  << "-" << frameDrawed << "/" << timeElapsed << "\n";
		SDL_BlitSurface(m_pBombSurface, &framePos,
						SDL_GetVideoSurface(), &blitPos);
		
	}
}

void Display::drawExplosions(int width, int height, const std::vector<double>& explosions)
{
	for (int x=1; x<width-1; x++)
	{
		for (int y=1; y<height-1; y++)
		{
			SDL_Rect blitPos;
			blitPos.x = x*32;
			blitPos.y = y*32;
			blitPos.w = 32;
			blitPos.h = 32;
			
			SDL_Rect framePos;
			framePos.x = 32;
			framePos.y = 64;
			framePos.w = 32;
			framePos.h = 32;
			
			if (explosions[y*width+x] >= SDL_GetTicks()/1000.0)
			{
				if (explosions[(y-1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[(y+1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x+1)] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x-1)] >= SDL_GetTicks()/1000.0
								
					|| explosions[(y-1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[(y+1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x+1)] >= SDL_GetTicks()/1000.0
								
					|| explosions[(y-1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[(y+1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x-1)] >= SDL_GetTicks()/1000.0
								
					|| explosions[(y-1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x-1)] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x+1)] >= SDL_GetTicks()/1000.0
								
					|| explosions[(y+1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x-1)] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x+1)] >= SDL_GetTicks()/1000.0
				   
					|| explosions[(y+1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x+1)] >= SDL_GetTicks()/1000.0
								
					|| explosions[(y+1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x-1)] >= SDL_GetTicks()/1000.0
								
					|| explosions[(y-1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x-1)] >= SDL_GetTicks()/1000.0
								
					|| explosions[(y-1)*width+x] >= SDL_GetTicks()/1000.0
					&& explosions[y*width+(x+1)] >= SDL_GetTicks()/1000.0)
				{
					framePos.x = 0;
					framePos.y = 64;
				}
				else
				{		
					if (explosions[(y-1)*width+x] >= SDL_GetTicks()/1000.0
							&& explosions[(y+1)*width+x] >= SDL_GetTicks()/1000.0)
					{
						framePos.x = 32;
						framePos.y = 32;
					}
					else if (explosions[(y-1)*width+x] >= SDL_GetTicks()/1000.0)
					{
						framePos.x = 64;
						framePos.y = 32;
					}
					else if (explosions[(y+1)*width+x] >= SDL_GetTicks()/1000.0)
					{
						framePos.x = 0;
						framePos.y = 32;
					}
					
					if (explosions[y*width+(x-1)] >= SDL_GetTicks()/1000.0
						&& explosions[y*width+(x+1)] >= SDL_GetTicks()/1000.0)
					{
						framePos.x = 32;
						framePos.y = 0;
					}
					else if (explosions[y*width+x-1] >= SDL_GetTicks()/1000.0)
					{
						framePos.x = 64;
						framePos.y = 0;
					}
					else if (explosions[y*width+x+1] >= SDL_GetTicks()/1000.0)
					{
						framePos.x = 0;
						framePos.y = 0;
					}
				}
				SDL_BlitSurface(m_pBoomSurface, &framePos, SDL_GetVideoSurface(), &blitPos);
			}
			
		}
	}
}


Display::~Display()
{
	SDL_FreeSurface(m_pBrickSurface);
	SDL_FreeSurface(m_pRockSurface);
	SDL_FreeSurface(m_pEmptySurface);
	SDL_FreeSurface(m_pBombermanSurface);
	SDL_FreeSurface(m_pBombSurface);
	SDL_FreeSurface(m_pBoomSurface);
	SDL_Quit();
}


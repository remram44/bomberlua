#include "Output.h"

void Output::init(bool graphic, int server, const Engine *engine)
{
    get()->m_bGraphic = false;
    if(server != -1)
    {
        get()->m_bServer = true;
        get()->m_Socket.Listen(server);
    }
    else
        get()->m_bServer = false;
	
    if(graphic) 
    {
        try 
        {
            Display::init(engine);
            get()->m_bGraphic = true;
        }
        catch (Display::InitException &e) 
        {
                std::cerr << e.what () << std::endl;
            std::cerr << "Mode graphique désactivé." << std::endl;
        }
    }
}

bool Output::update(const Engine *engine)
{
    if(get()->m_bServer)
    {
        // TODO : Acceptation des connexions

        // TODO : Envoi des données aux clients
    }

    if(get()->m_bGraphic)
    {
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type) /* Test du type d'évènement */
			{
				case SDL_QUIT: /* Si c'est un évènement de type "Quitter" */
					return false;
				break;
			}
		}
		
		Display::update(engine);
    }
    return true;
}

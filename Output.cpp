#include "Output.h"

void Output::init(bool graphic, const Engine *engine)
{
    get()->m_bGraphic = false;
#ifndef _NO_GRAPHICS
    if(graphic)
    {
        try
        {
            Display::init(engine);
            get()->m_bGraphic = true;
        }
        catch(Display::InitException &e)
        {
            std::cerr << e.what() << std::endl;
            std::cerr << "Mode graphique désactivé." << std::endl;
        }
    }
#endif
}

bool Output::update(const Engine *engine)
{
#ifndef _NO_GRAPHICS
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
#endif
    return true;
}

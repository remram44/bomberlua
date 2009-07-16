#include "Output.h"

void Output::init(bool graphic, const Engine *engine)
{
    get()->m_bGraphic = false;
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
            std::cerr << "Graphic mode disabled." << std::endl;
        }
    }
}

bool Output::update(const Engine *engine)
{
    if(get()->m_bGraphic)
    {
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type) /* Test the type of the event */
			{
				case SDL_QUIT: /* If it is a QUIT event */
					return false;
				break;
			}
		}
		
		Display::update(engine);
    }
    return true;
}

#include "game.hpp"

Game* s_Game = nullptr;
int frameCount = 0;

int main(int argc, char* argv[]) {

	const int frameDelay = 1000 / FPS;

	Uint32 frameStart;
	int frameTime;

	s_Game = new Game();
	s_Game->init("INSTABILITY", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DEFAULT_RESOLUTION_W, DEFAULT_RESOLUTION_H, false);
	

	while (s_Game->IsRunning()) 
	{

		frameStart = SDL_GetTicks();

		s_Game->HandleEvents();
		s_Game->Update();
		s_Game->Render();

		frameTime = SDL_GetTicks() - frameStart;

		if (frameDelay > frameTime)
		{
			SDL_Delay(frameDelay - frameTime);
			frameCount++;
			s_Game->GLOBAL_TIMER_SECONDS = frameCount / 60;
		}
	}

	s_Game->Clean();

	return 0;
}

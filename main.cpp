#include "game.hpp"

Game* s_Game = nullptr;
int frameCount = 0;

int main(int argc, char* argv[]) {

	//const int frameDelay = 1000 / FPS;

	Uint32 lastFrameTime = 0;
	float deltaTime = 0.0f;
	static float timer = 0.0f;
	static int frameCount = 0;
	s_Game = new Game();
	s_Game->init("INSTABILITY", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DEFAULT_RESOLUTION_W, DEFAULT_RESOLUTION_H, false);
	

	while (s_Game->IsRunning()) 
	{

		Uint32 now = SDL_GetTicks64();

		if (lastFrameTime == 0) lastFrameTime = now;

		deltaTime = (now - lastFrameTime) / 1000.0f;
		lastFrameTime = now;

		s_Game->m_DeltaTime = deltaTime;
		s_Game->m_LastFrameTime = lastFrameTime;

		s_Game->HandleEvents();
		s_Game->Update(deltaTime);
		s_Game->Render();

		timer += deltaTime;
		frameCount++;

		if (timer >= 1.0f) {
			//std::cout << "FPS: " << frameCount << " | deltaTime: " << deltaTime << std::endl;
			timer = 0.0f;
			frameCount = 0;
		}
	}

	s_Game->Clean();

	return 0;
}

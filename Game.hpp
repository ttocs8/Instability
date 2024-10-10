#include "SDL.h"
#include "SDL_image.h"
#include "GUI.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "GlobalHelpers.hpp"
#include <direct.h>
#include <map> 

class Game {

public:
	Game();

	int init(const char* title, int xpos, int ypos, int width, int height, bool isFullscreen);

	void HandleEvents();
	void Update();
	void Render();
	void Reset();
	void CreateInitialSaveSate();
	void SetCurrentSceneName(std::string theSceneName);
	std::string GetCurrentSceneName() { return m_CurrentSceneName; };
	void Clean();
	std::vector<Sprite*> GetAllSprites(GUI& theGUI);

	bool IsRunning() { return isRunning; };

	~Game();

private:
	int count = 0;
	bool isRunning = false;
	SDL_Window *m_mainWindow = NULL;
	SDL_Renderer* m_Renderer = NULL;
	std::string m_GameDataFolder = "";
	std::string m_CurrentSceneName = "";
	void ClickOnSprite(SDL_Event& theEvent, Sprite* theSprite);

};
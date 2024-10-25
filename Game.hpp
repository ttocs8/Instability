#include "SDL.h"
#include "SDL_image.h"
#include "Scene.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <direct.h>
#include <map> 
#include <algorithm>

using namespace std;

struct ScreenResolution_t {
	int W = DEFAULT_RESOLUTION_W;
	int H = DEFAULT_RESOLUTION_H;
};

class Game {

public:
	Game();

	int init(const char* title, int xpos, int ypos, int width, int height, bool isFullscreen);

	void HandleEvents();
	void Update();
	void Render();
	void Reset();
	void CreateInitialSaveSate();
	void Clean();

	bool IsRunning() { return IS_RUNNING_MAIN; };
	bool IsGamePlayGridSetUp() { return IS_RUNNING_MAIN; };

	~Game();

private:
	int count = 0;
	bool IS_RUNNING_MAIN = false;
	SDL_Window *m_mainWindow = NULL;
	SDL_Renderer* m_Renderer = NULL;
	string m_GameDataFolder = "";
	ScreenResolution_t m_CurrentScreenResolution;
	Scene m_CurrentScene;
	bool m_IsGameplayGridSetUp = false;

	void SetResolution(int w, int h);
	ScreenResolution_t GetCurrentResolution() { return m_CurrentScreenResolution; };
	void SetUpGameplayGrid();
	void ClickOnSprite(SDL_Event& theEvent, vector<Sprite*> theClickableSprites);
	void HoverOverSprite(SDL_Event& theEvent, vector<Sprite*> theHoverableSprites);
	void GoToNextScene(string theButtonClicked);
	void SortSpritesForRendering();


};
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
	void SetCurrentSceneName(string theSceneName);
	string GetCurrentSceneName() { return m_CurrentSceneName; };
	void Clean();
	vector<Sprite*> GetAllSprites(GUI& theGUI);


	bool IsRunning() { return IS_RUNNING_MAIN; };

	~Game();

private:
	int count = 0;
	bool IS_RUNNING_MAIN = false;
	SDL_Window *m_mainWindow = NULL;
	SDL_Renderer* m_Renderer = NULL;
	string m_GameDataFolder = "";
	string m_CurrentSceneName = "";
	ScreenResolution_t m_CurrentScreenResolution;

	void SetResolution(int w, int h);
	ScreenResolution_t GetCurrentResolution() { return m_CurrentScreenResolution; };
	void ClickOnSprite(SDL_Event& theEvent, vector<Sprite*> theClickableSprites);
	void HoverOverSprite(SDL_Event& theEvent, vector<Sprite*> theHoverableSprites);

};
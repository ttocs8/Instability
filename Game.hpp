#include "SDL.h"
#include "SDL_image.h"
#include "Scene.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <direct.h>
#include <map> 
#include <algorithm>
#include <numeric>
#include <mutex>

using namespace std;

struct ScreenResolution_t {
	int W = DEFAULT_RESOLUTION_W;
	int H = DEFAULT_RESOLUTION_H;
	int RW = RENDER_RESOLUTION_W;
	int RH = RENDER_RESOLUTION_H;
};

class Game {

public:
	Game();
	int GLOBAL_TIMER_SECONDS = 0;

	float m_DeltaTime = 0.0f;
	Uint32 m_LastFrameTime = 0;

	int init(const char* title, int xpos, int ypos, int width, int height, bool isFullscreen);

	void HandleEvents();
	void Update(float deltaTime);
	void Render();
	void Reset();
	void CreateInitialSaveSate();
	void Clean();

	bool IsRunning() { return IS_RUNNING_MAIN; };
	bool IsGamePlayGridSetUp() { return IS_RUNNING_MAIN; };

	~Game();

private:
	
	bool IS_RUNNING_MAIN = false;
	SDL_Window *m_mainWindow = NULL;
	SDL_Renderer* m_Renderer = NULL;
	string m_GameDataFolder = "";
	ScreenResolution_t m_CurrentScreenResolution;
	Scene m_CurrentScene;
	int m_LastDisplayedEnergy = 0;
	bool m_IsGameplayGridSetUp = false;

	void SetResolution(int w, int h);
	ScreenResolution_t GetCurrentResolution() { return m_CurrentScreenResolution; };
	void SetUpGameplayGrid();
	//the Vector is const so it doesnt copy the entire list every single call
	void ClickOnSprite(SDL_Event& theEvent, const vector<Sprite*> theClickableSprites);
	//the Vector is const so it doesnt copy the entire list every single call
	void HoverOverSprite(SDL_Event& theEvent, const vector<Sprite*> theHoverableSprites);
	void UpdateGrid();
	void ResetGridPosition();
	void GoToNextScene(string theButtonClicked);
	void SortSpritesForRendering();
	//void DestroyAllSpritesOfName(string spriteName);
	void RemoveSpriteFromRenderingList(Sprite* spriteToRemove);


};

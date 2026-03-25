#include "SDL.h"
#include "SDL_image.h"
#include "Sprite.hpp"
#include <iostream>
#include <vector>
#include <list>    


class Scene {
public:
	Scene();
	Scene(string sceneName, SceneType currentScene);
	~Scene();
	void AddSpriteToRenderList(Sprite* theSprite);
	void AddSpritesToRenderList(std::vector<Sprite*> theSprites);
	void AddClickableSpriteToList(Sprite* theSprite);
	void AddClickableSpritesToList(std::vector<Sprite*> theSprites);
	std::vector<Sprite*> GetSpriteList() { return m_ListOfSpritesToRender; };
	Sprite* GetSprite(std::string theSpriteName);
	string GetName() { return  m_SceneName; }
	void SetName(std::string theName) { m_SceneName = theName; }
	void EnableAllSprites();
	void EnableAllSprites_Except(std::string theNameToFilter);
	void DisableAllSprites();
	SceneType GetSceneType();

	std::vector<Sprite*> m_ListOfSpritesToRender;
	std::vector<Sprite*> m_ListOfClickableSprites;
	
	

private:
	std::string m_SceneName = "";
	int m_SceneType = -1;
};
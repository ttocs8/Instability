#include "SDL.h"
#include "SDL_image.h"
#include "Sprite.hpp"
#include <iostream>
#include <vector>
#include <list>    


class Scene {
public:
	Scene();
	~Scene();
	void AddSpriteToList(Sprite* theSprite);
	void AddSpritesToList(std::vector<Sprite*> theSprites);
	std::vector<Sprite*> GetSpriteList() { return m_ListOfSprites; };
	Sprite* GetSprite(std::string theSpriteName);
	string GetName() { return  m_SceneName; }
	void SetName(std::string theName) { m_SceneName = theName; }
	void EnableAllSprites();
	void EnableAllSprites_Except(std::string theNameToFilter);
	void DisableAllSprites();

	std::vector<Sprite*> m_ListOfSprites;
	//0 - main menu
	//1 - options menu
	//2 - gameplay
	int m_SceneEnum = 0;

private:
	std::string m_SceneName = "";
};
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
	string GetName() { return  m_SceneName; }
	void SetName(std::string theName) { m_SceneName = theName; }
	void EnableAllSprites();
	void DisableAllSprites();

	std::vector<Sprite*> m_ListOfSprites;

private:
	std::string m_SceneName = "";
};
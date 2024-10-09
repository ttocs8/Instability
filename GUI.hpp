#include "SDL.h"
#include "SDL_image.h"
#include "Sprite.hpp"
#include <iostream>
#include <vector>
#include <list>    


class GUI {
public:
	GUI();
	~GUI();
	void AddSpriteToList(Sprite& theSprite);
	void AddSpritesToList(std::vector<Sprite*> theSprites);
	std::vector<Sprite*> GetSpriteList() { return m_ListOfSprites; };

private:
	Sprite m_Sprite;
	std::vector<Sprite*> m_ListOfSprites;
};
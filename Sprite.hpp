#include "SDL.h"
#include "SDL_image.h"
#include <iostream>


class Sprite {
public:
	Sprite();
	~Sprite();

	void setTexture(SDL_Renderer* theRenderer, const char* theFullPathToImage);
	void setDimenstions(int w, int h);
	void setPosition(int x, int y);
	void setSpriteName(const char* theName);

	void SaveSprite();

	std::string getSpriteName();
	SDL_Texture* getTexture();
	std::string getTextureSource() { return m_SpriteTextureSource; };
	SDL_Rect* getRect();

	void Destroy();

private:
	SDL_Texture* m_Texture = 0;
	SDL_Rect* m_Rect;
	std::string m_SpriteName = "";
	std::string m_SpriteTextureSource = "";
	
};
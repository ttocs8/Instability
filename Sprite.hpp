#include "SDL.h"
#include "SDL_image.h"
#include "GlobalHelpers.hpp"
#include <iostream>

using namespace std;


class Sprite {
public:
	Sprite();
	Sprite(SDL_Renderer* theRenderer, const char* theFullPathToImage, int w, int h, int x, int y, const char* theName, bool isScreenCentered, int xOffset, int yOffset);
	Sprite(SDL_Renderer* theRenderer, const char* theFullPathToImage, int w, int h, int x, int y, const char* theName);
	~Sprite();
	string m_SpriteName = "";

	void Create(SDL_Renderer* theRenderer, const char* theFullPathToImage, int w, int h, int x, int y, const char* theName, bool isScreenCentered = false, int xOffset = 0, int yOffset = 0);
	void setTexture(SDL_Renderer* theRenderer, const char* theFullPathToImage);
	void setTexture(SDL_Texture* theTexture);
	void setDimenstions(int w, int h);
	void setWidth(int w);
	void setPosition(int x, int y);
	void setYPos(int theYPos);
	void setXPos(int theXPos);
	void setSpriteName(const char* theName);
	void setText(SDL_Renderer* theRenderer, TTF_Font* theFont, SDL_Color theColor, string theMessage);
	bool IsEnabled() { return m_IsEnabled; };
	bool compareColor(SDL_Color theColorToCompare);

	void Disable() { m_IsEnabled = false; };
	void Enable() { m_IsEnabled = true; };

	string getSpriteName();
	SDL_Texture* getTexture();
	SDL_Color getColor();
	string getTextureSource() { return m_SpriteTextureSource; };
	SDL_Rect* getRect();

	void Destroy();

private:
	SDL_Texture* m_Texture = 0;
	SDL_Rect* m_Rect;
	string m_SpriteTextureSource = "";
	SDL_Color m_Color;
	bool m_IsEnabled = false;
	
};
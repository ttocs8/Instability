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
	void setPosition(int x, int y);
	void setYPos(int theYPos);
	void setXPos(int theXPos);
	void setSpriteName(const char* theName);
	void setText(SDL_Renderer* theRenderer, TTF_Font* theFont, SDL_Color theColor, string theMessage);
	bool IsEnabled() { return m_IsEnabled; };
	void Disable() { m_IsEnabled = false; };
	void Enable() { m_IsEnabled = true; };

	void SaveSprite();

	string getSpriteName();
	SDL_Texture* getTexture();
	string getTextureSource() { return m_SpriteTextureSource; };
	SDL_Rect* getRect();

	void Destroy();

	/*friend bool operator>(const Sprite& m1, const Sprite& m2) {
		char left = m1.m_SpriteName[0];
		char right = m2.m_SpriteName[0];
		int(left) > int(right);
	};*/

private:
	SDL_Texture* m_Texture = 0;
	SDL_Rect* m_Rect;
	string m_SpriteTextureSource = "";
	bool m_IsEnabled = false;
	
};
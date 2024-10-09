#include "Sprite.hpp"

Sprite::Sprite()
{
	m_Rect = new SDL_Rect();
}

Sprite::~Sprite(){}

void Sprite::setTexture(SDL_Renderer* theRenderer, const char* theFullPathToImage ) {
	m_SpriteTextureSource = theFullPathToImage;
	SDL_Surface* tempSurface = IMG_Load(theFullPathToImage);
	m_Texture = SDL_CreateTextureFromSurface(theRenderer, tempSurface);
	SDL_FreeSurface(tempSurface);
}
void Sprite::setDimenstions(int theWidth, int theHeight) {
	SDL_assert(m_Texture != NULL);
	m_Rect->w = theWidth;
	m_Rect->h = theHeight;
}

void Sprite::setPosition(int theXPos, int theYPos) {
	SDL_assert(m_Texture != NULL);
	m_Rect->x = theXPos;
	m_Rect->y = theYPos;
}

void Sprite::setSpriteName(const char* theName) {
	m_SpriteName = theName;
}

void Sprite::SaveSprite() {
	//TODO: implement
}

std::string Sprite::getSpriteName()
{
	SDL_assert(strlen( m_SpriteName.c_str() ) != 0);
	return m_SpriteName;
}

SDL_Texture* Sprite::getTexture() {
	return m_Texture;
}

SDL_Rect* Sprite::getRect() {
	return m_Rect;
}

void Sprite::Destroy() {
	SDL_DestroyTexture(m_Texture);
	//LOOK AT THIS LATER FUCKO
	//FIND A WAY TO ACTUALLY DELETE THIS FUCKER
	setDimenstions(0, 0);
}



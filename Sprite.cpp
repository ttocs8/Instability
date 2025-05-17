#include "Sprite.hpp"

Sprite::Sprite()
{
	m_Rect = new SDL_Rect();
}

Sprite::Sprite(SDL_Renderer* theRenderer, const char* theFullPathToImage, int w, int h, int x, int y, const char* theName, bool isScreenCentered, int xOffset, int yOffset)
{
	m_Rect = new SDL_Rect();
	Create(theRenderer, theFullPathToImage, w, h, x, y, theName, isScreenCentered, xOffset, yOffset);
}

Sprite::Sprite(SDL_Renderer* theRenderer, const char* theFullPathToImage, int w, int h, int x, int y, const char* theName)
{
	m_Rect = new SDL_Rect();
	Create(theRenderer, theFullPathToImage, w, h, x, y, theName);
}

Sprite::~Sprite(){}

void Sprite::Create(SDL_Renderer* theRenderer, const char* theFullPathToImage, int w, int h, int x, int y, const char* theName, bool isScreenCentered, int xOffset, int yOffset) {


	setDimenstions(w, h);

	if(theFullPathToImage != NULL)
		setTexture(theRenderer, theFullPathToImage);
	if (isScreenCentered) {
		int centerX = x - ( m_Rect->w / 2 );
		int centerY = y - ( m_Rect->w / 2 );
		setPosition( centerX + xOffset, centerY + yOffset );
	}
	else
		setPosition(x, y);

	
	setSpriteName(theName);
	m_IsEnabled = true;

	
}

void Sprite::setTexture(SDL_Renderer* theRenderer, const char* theFullPathToImage ) {
	m_SpriteTextureSource = theFullPathToImage;
	SDL_Surface* tempSurface = IMG_Load(theFullPathToImage);
	m_Rect->w = tempSurface->w;
	m_Rect->h = tempSurface->h;
	m_Texture = SDL_CreateTextureFromSurface(theRenderer, tempSurface);
	SDL_FreeSurface(tempSurface);
}

void Sprite::setTexture(SDL_Texture* theTexture) {
	m_Texture = theTexture;
}

void Sprite::setDimenstions(int theWidth, int theHeight) {
	//SDL_assert(m_Texture != NULL);
	m_Rect->w = theWidth;
	m_Rect->h = theHeight;
}

void Sprite::setWidth(int theWidth) {
	m_Rect->w = theWidth;
}

void Sprite::setPosition(int theXPos, int theYPos) {
	//SDL_assert(m_Texture != NULL);
	m_Rect->x = theXPos;
	m_Rect->y = theYPos;
}

void Sprite::setYPos(int theYPos) {
	m_Rect->y = theYPos;
}

void Sprite::setXPos(int theXPos) {
	m_Rect->x = theXPos;
}

void Sprite::setSpriteName(const char* theName) {
	m_SpriteName = theName;
}

void Sprite::setText(SDL_Renderer* theRenderer, TTF_Font* theFont, SDL_Color theColor, string theMessage) {
	m_Color = theColor;
	SDL_Surface* tempSurface = TTF_RenderText_Solid(theFont, theMessage.c_str(), theColor);
	m_Rect->w = tempSurface->w;
	m_Rect->h = tempSurface->h;
	m_Texture = SDL_CreateTextureFromSurface(theRenderer, tempSurface);
	SDL_FreeSurface(tempSurface);
}

bool Sprite::compareColor(SDL_Color theColorToCompare)
{
	return theColorToCompare.r == m_Color.r && theColorToCompare.g == m_Color.g && theColorToCompare.b == m_Color.b && theColorToCompare.a == m_Color.a;
}

std::string Sprite::getSpriteName()
{
	SDL_assert(strlen( m_SpriteName.c_str() ) != 0);
	return m_SpriteName;
}

SDL_Texture* Sprite::getTexture() {
	return m_Texture;
}

SDL_Color Sprite::getColor() {
	return m_Color;
}

SDL_Rect* Sprite::getRect() {
	return m_Rect;
}

void Sprite::Destroy() {
	//LOOK AT THIS LATER FUCKO
	//FIND A WAY TO ACTUALLY DELETE THIS FUCKER
	m_IsEnabled = false;
	SDL_DestroyTexture(m_Texture);
	setDimenstions(0, 0);
	SDL_free(&m_Rect);
}



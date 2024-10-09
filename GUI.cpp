#include "GUI.hpp"

GUI::GUI(){}
GUI::~GUI(){}

void GUI::AddSpriteToList( Sprite& theSprite ) {
	m_ListOfSprites.push_back( &theSprite );
}

void GUI::AddSpritesToList(std::vector<Sprite*> theSprites){
	m_ListOfSprites.insert(std::end(m_ListOfSprites), std::begin(theSprites), std::end(theSprites));
}

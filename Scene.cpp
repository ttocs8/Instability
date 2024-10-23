#include "Scene.hpp"

Scene::Scene(){
	m_SceneName = "";
}
Scene::~Scene(){}

void Scene::AddSpriteToList( Sprite* theSprite ) {
	m_ListOfSprites.push_back( theSprite );
}

void Scene::AddSpritesToList(std::vector<Sprite*> theSprites){
	m_ListOfSprites.insert(std::end(m_ListOfSprites), std::begin(theSprites), std::end(theSprites));
}

void Scene::EnableAllSprites() {
	for (Sprite* sprite : m_ListOfSprites)
		sprite->Enable();
}

void Scene::DisableAllSprites() {
	for (Sprite* sprite : m_ListOfSprites)
		sprite->Disable();
}


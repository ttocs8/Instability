#include "Scene.hpp"


Scene::Scene() {
	m_SceneName = "";
}

Scene::Scene(string sceneName, SceneType currentScene) {
	m_SceneName = sceneName;
	m_SceneType = currentScene;
}
Scene::~Scene(){}

void Scene::AddSpriteToRenderList( Sprite* theSprite ) {
	m_ListOfSpritesToRender.push_back( theSprite );
}

void Scene::AddSpritesToRenderList(std::vector<Sprite*> theSprites){
	m_ListOfSpritesToRender.insert(std::end(m_ListOfSpritesToRender), std::begin(theSprites), std::end(theSprites));
}

void Scene::AddClickableSpriteToList(Sprite* theSprite) {
	m_ListOfClickableSprites.push_back(theSprite);
}

void Scene::AddClickableSpritesToList(std::vector<Sprite*> theSprites) {
	m_ListOfClickableSprites.insert(std::end(m_ListOfClickableSprites), std::begin(theSprites), std::end(theSprites));
}
void Scene::EnableAllSprites() {
	for (Sprite* sprite : m_ListOfSpritesToRender)
		sprite->Enable();
}

void Scene::EnableAllSprites_Except(std::string theNameToFilter) {
	for (Sprite* sprite : m_ListOfSpritesToRender)
		if (sprite->getSpriteName().find(theNameToFilter) == std::string::npos)
			sprite->Enable();
		else
			sprite->Disable();
}

void Scene::DisableAllSprites() {
	for (Sprite* sprite : m_ListOfSpritesToRender)
		sprite->Disable();
}

SceneType Scene::GetSceneType()
{
	return (SceneType)m_SceneType;
}

Sprite* Scene::GetSprite(std::string theSpriteName) {
	for (Sprite* sprite : m_ListOfSpritesToRender)
		if (sprite->getSpriteName().find(theSpriteName) != std::string::npos)
			return sprite;

	return 0;
}



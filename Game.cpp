#include "Game.hpp"
//Diable fopen warnings
#pragma warning(disable:4996)

GUI Menu;
Sprite bg;
Sprite tempPlayButton;
Sprite tempQuitButton;
Sprite tempOptionsButton;
SDL_Rect* playrect;
std::string g_SaveStatesFolder;


Game::Game() {
	m_GameDataFolder = SDL_GetPrefPath(ORG_NAME, APP_NAME);
	SDL_assert(strlen(m_GameDataFolder.c_str()) != 0);

	g_SaveStatesFolder = m_GameDataFolder + SAVE_STATE_FOLDER;
}

Game::~Game() {}
void Game::SetCurrentSceneName(std::string theSceneName) {
	m_CurrentSceneName = theSceneName;
}

int Game::init(
	const char* title, 
	int xpos, 
	int ypos, 
	int width, 
	int height, 
	bool isFullscreen
)
{
	int flags = 0;
	if (isFullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		m_mainWindow = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
		if (m_mainWindow)
		{
			std::cout << "Window created." << std::endl;
		}

		m_Renderer = SDL_CreateRenderer(m_mainWindow, -1, 0);
		if (m_Renderer)
		{
			SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
			std::cout << "Renderer created." << std::endl;
		}

		isRunning = true;
	}
	else
	{
		isRunning = false;
	}

	/*Create / Verify game file directories
	* 
	* 
	*Directory will be as follows:
	*	C:\Users\USER\AppData\Roaming\Perpetual Motion Software\Instability\SaveStates
	*	...
	*/
	std::cout << "Initializing game data folders..." << std::endl;
	if (_mkdir(g_SaveStatesFolder.c_str()) == 0) {
		std::cout << "\t>> Created \"" << g_SaveStatesFolder << "\" folder for the first time" << std::endl;
	}
	else {
		//failed to create direcotry (could already be created, which is fine)
		// IGNORE FOR NOW //
	}

	struct stat info;
	//Assert created folders are Directories and is accessible
	SDL_assert(stat(m_GameDataFolder.c_str(), &info) == 0);
	SDL_assert(info.st_mode & S_IFDIR);

	SDL_assert(stat(g_SaveStatesFolder.c_str(), &info) == 0);
	SDL_assert(info.st_mode & S_IFDIR);

	//Create Main Menu sprites
	Menu.AddSpritesToList({ &bg, &tempPlayButton, &tempQuitButton, &tempOptionsButton });

	bg.setTexture(m_Renderer, "Assets/pixelArt.png");
	bg.setDimenstions(800, 600);
	bg.setSpriteName("BG_BACKGROUND");

	tempPlayButton.setTexture(m_Renderer, "Assets/tempplay.png");
	tempPlayButton.setDimenstions(150, 75);
	tempPlayButton.setPosition(200, 200);
	tempPlayButton.setSpriteName("BT_PlayButton");

	tempQuitButton.setTexture(m_Renderer, "Assets/tempquit.png");
	tempQuitButton.setDimenstions(150, 75);
	tempQuitButton.setPosition(500, 200);
	tempQuitButton.setSpriteName("BT_QuitButton");

	tempOptionsButton.setTexture(m_Renderer, "Assets/tempoptions.png");
	tempOptionsButton.setDimenstions(150, 75);
	tempOptionsButton.setPosition(350, 400);
	tempOptionsButton.setSpriteName("BT_OptionsButton");

	SetCurrentSceneName(SCENE_PREFIX "MainMenu");
	SaveState("__InitalGameState");

	return 0;
}

std::vector<Sprite*> Game::GetAllSprites(GUI& theGUI /*, Enemys enemyObjecsts, etc*/)
{
	return theGUI.GetSpriteList();
}


void Game::ClickOnSprite(SDL_Event& theEvent, Sprite* theSprite)
{
	SDL_Point mousePosition;

	mousePosition.x = theEvent.motion.x;
	mousePosition.y = theEvent.motion.y;

	if (SDL_PointInRect(&mousePosition, theSprite->getRect()) && theEvent.button.clicks == 1 && theEvent.button.state == SDL_PRESSED) {

		//destroy sprites besides any sprite with the background tag in its name
		std::vector<Sprite*> allMenuSprites = GetAllSprites(Menu);
		for (int i = 0; i < allMenuSprites.size(); i++)
		{
			std::string currentSpriteName = allMenuSprites.at(i)->getSpriteName();
			if (currentSpriteName.find("BG") == std::string::npos)
			{
				allMenuSprites.at(i)->Destroy();
			}
		}
	}
}



void Game::HandleEvents() {
	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type) {
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_KEYDOWN:
			Reset();
		default:
			break;
	}
	ClickOnSprite(event, &tempPlayButton);
	ClickOnSprite(event, &tempQuitButton);
	ClickOnSprite(event, &tempOptionsButton);
}

void Game::Update() {
	//count++;

	//count / FPS = Timer in seconds
	//std::cout << count << std::endl;
}

void Game::Render() {
	SDL_RenderClear(m_Renderer);
	//////////////////////////

	SDL_RenderCopy(m_Renderer, bg.getTexture(),NULL, bg.getRect());
	SDL_RenderCopy(m_Renderer, tempPlayButton.getTexture(),NULL, tempPlayButton.getRect());
	SDL_RenderCopy(m_Renderer, tempQuitButton.getTexture(),NULL, tempQuitButton.getRect());
	SDL_RenderCopy(m_Renderer, tempOptionsButton.getTexture(),NULL, tempOptionsButton.getRect());

	/////////////////////////
	SDL_RenderPresent(m_Renderer);
}

void Game::Reset() {
	//Read in initial save data and apply
	std::string filePath = g_SaveStatesFolder + GlobalHelpers::GetOSSeparator() + INITIAL_SAVE_STATE_FILE + ".config";
	std::ifstream tempFile;
	tempFile.open(filePath, std::ios::in);
	if (tempFile.is_open())
	{
		std::vector<Sprite*> menuSprites = Menu.GetSpriteList();
		std::vector<std::string> tokensToProcess;
		int index = 0;
		for (std::string line; getline(tempFile, line, '\n'); ) {
	
			char* token = strtok((char*)line.c_str() , ",");

			//tokenize each line
			std::string name = token;
			if (name.find("SS_") != std::string::npos)
				continue;
			
			while (token != NULL)
			{
				tokensToProcess.push_back(token);
				token = strtok(NULL, ",");
			}

			//Process tokens
			menuSprites.at(index)->setSpriteName(tokensToProcess.at(0).c_str());
			menuSprites.at(index)->setPosition(stoi(tokensToProcess.at(1)), stoi(tokensToProcess.at(2)));
			menuSprites.at(index)->setDimenstions(stoi(tokensToProcess.at(3)), stoi(tokensToProcess.at(4)));
			menuSprites.at(index)->setTexture(m_Renderer, tokensToProcess.at(5).c_str());
			
			tokensToProcess.clear();

			index++;
		}

		tempFile.close();
	}
	else
	{
		std::cout << "Could not open save file for writing";
	}
}

void Game::SaveState(std::string theStateName) {

	std::string filePath = g_SaveStatesFolder + GlobalHelpers::GetOSSeparator() + theStateName + ".config";

	std::ofstream tempFile;
	tempFile.open(filePath, std::ios::out);

	if (tempFile.is_open())
	{
		std::vector<Sprite*> menuSprites = Menu.GetSpriteList();
		tempFile << "SS_SPRITES\n";
		for (int i = 0; i < menuSprites.size(); i++)
		{
			Sprite currentSprite = *menuSprites.at(i);
			SDL_Rect spriteRect = *menuSprites.at(i)->getRect();
			std::string spriteName = menuSprites.at(i)->getSpriteName();

			tempFile << spriteName << "," << spriteRect.x << "," << spriteRect.y << "," << spriteRect.w << "," << spriteRect.h << "," << currentSprite.getTextureSource() << ",\n";
		}

		tempFile.close();
	}
	else
	{
		std::cout << "Could not open save file for writing";
	}
}

void Game::Clean(){
	SDL_DestroyWindow(m_mainWindow);
	SDL_DestroyRenderer(m_Renderer);
	SDL_Quit();
	std::cout << "Game shut down successfully" << std::endl;
}


#include "Game.hpp"
//Diable fopen warnings
#pragma warning(disable:4996)

using namespace std;


GUI Menu;
Sprite bg;
Sprite titleCard;

Sprite tempPlayButton;
Sprite tempQuitButton;
Sprite tempOptionsButton;

GUI OptionsMenu;
Sprite tempResolutionButton;

string g_SaveStatesFolder;
map< string, map<string, vector<string> > > g_SettingsFileMap;

Game::Game() {
	m_GameDataFolder = SDL_GetPrefPath(ORG_NAME, APP_NAME);
	SDL_assert(strlen(m_GameDataFolder.c_str()) != 0);

	g_SaveStatesFolder = m_GameDataFolder + SAVE_STATE_FOLDER;
}

Game::~Game() {}
void Game::SetCurrentSceneName(string theSceneName) {
	m_CurrentSceneName = theSceneName;
}

//VERY MESSY, MAY NEED TO REFACTOR
void LoadSettingsFromConfig(map< string, map<string, vector<string> > >& TheSettingsFileMap) {
	//Read in initial save data and apply
	string filePath = g_SaveStatesFolder + GlobalHelpers::GetOSSeparator() + INITIAL_SAVE_STATE_FILE + ".config";
	ifstream tempFile;
	tempFile.open(filePath, ios::in);

	int lineNum = 0;
	if (tempFile.is_open())
	{
		vector<string> tokensToProcess;
		map < string, vector < string > > vectorOfTokensToProcess;
		vector<string> fileLines;
		int index = 0;
		for (string line; getline(tempFile, line, '\n'); ) {
			fileLines.push_back(line);
			char* token = strtok((char*)line.c_str(), ",");
	
			string name = token;
			if (name.find(SAVE_STATE_PREFIX) != string::npos) {
				TheSettingsFileMap.emplace(name, vectorOfTokensToProcess);
				lineNum++;
				continue;
			}

			//tokenize each line
			while (token != NULL)
			{
				int i = 1;
				string settingHeader;
				while (i < 10) {
					settingHeader = fileLines.at(lineNum - i);
					if (settingHeader.find(SAVE_STATE_PREFIX) != string::npos) {
						break;
					}
					i++;
				}
				if (name.find(token) == string::npos) {
					TheSettingsFileMap[settingHeader][name].push_back(token);
				}
				token = strtok(NULL, ",");
			}

			lineNum++;
		}
	}
	else
	{
		cout << "Could not open settings file" << endl;
	}
	tempFile.close();
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


	string firstTimeInitFilePath = g_SaveStatesFolder + GlobalHelpers::GetOSSeparator() + FIRST_TIME_INIT_FILENAME + ".init";
	bool isFirstTimeInit = !GlobalHelpers::FileExists(firstTimeInitFilePath.c_str());

	//used for asserting files/directories exist
	struct stat info;
	/* Create game file directories
	*
	*
	*Directory will be as follows:
	*	C:\Users\USER\AppData\Roaming\Perpetual Motion Software\Instability\SaveStates
	*	...
	*/
	cout << "Initializing game data folders..." << endl;
	if (_mkdir(g_SaveStatesFolder.c_str()) == 0) {
		cout << "\t>> Created directory \"" << g_SaveStatesFolder << "\"" << endl;

		ofstream ftInitFile;
		ftInitFile.open(firstTimeInitFilePath, ios::app);
		ftInitFile << "~First time initialization~\n";
		ftInitFile << "~Deleting this file may break game initialization - only do so if you know what you are doing";
		ftInitFile.close();

		cout << "\t>> Created first time initialization file successfully" << endl;

	}
	else if(!isFirstTimeInit) {
		cout << "\t>> Folder \"" << SAVE_STATE_FOLDER << "\" has already been initialized..." << endl;

		SDL_assert(stat(firstTimeInitFilePath.c_str(), &info) == 0);
		SDL_assert(info.st_mode & S_IFMT);
		cout << "\t>> First time initialization file already exists and is accessible..." << endl;
	}
	else
	{
		//some error happened, failed to create directory
		// IGNORE FOR NOW //
	}

	/* Verify game file directories each are:
	*   1.) a Directory
	*   2.) Accessible
	*/
	SDL_assert(stat(m_GameDataFolder.c_str(), &info) == 0);
	SDL_assert(info.st_mode & S_IFDIR);

	SDL_assert(stat(g_SaveStatesFolder.c_str(), &info) == 0);
	SDL_assert(info.st_mode & S_IFDIR);

	int w = width;
	int h = height;
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		
		if (width == -1 || height == -1) {
			if (isFirstTimeInit || g_SettingsFileMap.empty()) {
				w = DEFAULT_RESOLUTION_W;
				h = DEFAULT_RESOLUTION_H;
			}
			else {
				//Read in settings from file
				for (map< string, map<string, vector<string> > >::iterator iter1 = g_SettingsFileMap.begin(); iter1 != g_SettingsFileMap.end(); iter1++) {

					auto& settingName = (*iter1).first;
					if (settingName.find("RES") != string::npos) {
						map<string, vector<string> >::iterator iter2((*iter1).second.begin());
						for (map<string, vector<string> >::iterator iter2 = (*iter1).second.begin(); iter2 != (*iter1).second.end(); iter2++) {
							auto& settingName = (*iter2).first;
							auto& data = (*iter2).second;
							w = stoi(data.at(0));
							h = stoi(data.at(1));
						}
					}
				}
			}
		}

		int windowFlags = 0;
		if (isFullscreen)
			windowFlags = SDL_WINDOW_FULLSCREEN;
		
		m_mainWindow = SDL_CreateWindow(title, xpos, ypos, w, h, windowFlags);
		SetResolution(w, h);
		//SDL_SetWindowResizable(m_mainWindow,SDL_TRUE);
		if (m_mainWindow) {
			cout << "Window created..." << endl;
		}

		m_Renderer = SDL_CreateRenderer(m_mainWindow, -1, 0);
		if (m_Renderer) {
			SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
			cout << "Renderer created..." << endl;
		}

		std::string pathToIcon = ASSETS_FOLDER + GlobalHelpers::GetOSSeparator() + "icon.png";
		SDL_SetWindowIcon(m_mainWindow,IMG_Load(pathToIcon.c_str()));

		IS_RUNNING_MAIN = true;
	}
	else
	{
		IS_RUNNING_MAIN = false;
	}

		
	//Create Main Menu sprites
	Menu.AddSpritesToList({ &bg, &titleCard, &tempPlayButton, &tempQuitButton, &tempOptionsButton });
	
	bg.Create(m_Renderer, "Assets/menubackground3.png", w, h, 0, 0,"BG_BACKGROUND");
	titleCard.Create(m_Renderer, "Assets/titlecard.png", 500, 200, GetCurrentResolution().W / 2, GetCurrentResolution().H / 2,"TitleCard",true,0,-100);
	tempPlayButton.Create(m_Renderer, "Assets/tempplay.png", 150, 75, GetCurrentResolution().W / 2, GetCurrentResolution().H / 2, "BT_PlayButton", true, 0 ,25);
	tempQuitButton.Create(m_Renderer, "Assets/tempquit.png", 150, 75, GetCurrentResolution().W / 2, GetCurrentResolution().H / 2, "BT_QuitButton", true, 0, 125);
	tempOptionsButton.Create(m_Renderer, "Assets/tempoptions.png", 50, 50, GetCurrentResolution().W - 75, 25,"BT_OptionsButton");

	SetCurrentSceneName(SCENE_PREFIX "MainMenu");

	if (isFirstTimeInit) 
		CreateInitialSaveSate();
	
	//Load all saved settings into map for runtime usage
	LoadSettingsFromConfig(g_SettingsFileMap);
	SDL_assert(!g_SettingsFileMap.empty());

	return 0;
}

vector<Sprite*> Game::GetAllSprites(GUI& theGUI /*, Enemys enemyObjecsts, etc*/)
{
	return theGUI.GetSpriteList();
}


void Game::SetResolution(int w, int h) {
	m_CurrentScreenResolution.W = w;
	m_CurrentScreenResolution.H = h;
}

void Game::ClickOnSprite(SDL_Event& theEvent, vector<Sprite*> theClickableSprites)
{
	SDL_Point mousePosition;

	mousePosition.x = theEvent.motion.x;
	mousePosition.y = theEvent.motion.y;

	for (Sprite* spriteToCheck : theClickableSprites) {

		//If the mouse is INSIDE the sprite rectangle upon clicking
		if (SDL_PointInRect(&mousePosition, spriteToCheck->getRect()))
		{
			if (spriteToCheck->getSpriteName().find(BUTTON_PREFIX "PlayButton") != string::npos) {

				//Destroy All Menu Sprites
				vector<Sprite*> allMenuSprites = GetAllSprites(Menu);
				for (Sprite* menuSprite : allMenuSprites)
					menuSprite->Destroy();

				//Change Scene
				SetCurrentSceneName(SCENE_PREFIX "Game");
				SDL_SetWindowTitle(m_mainWindow, GetCurrentSceneName().c_str());

				//Set up scene
				//	spawn game sprites
				//	spawn UI sprites
			}
			else if (spriteToCheck->getSpriteName().find(BUTTON_PREFIX "OptionsButton") != string::npos) {
				cout << "Clicked on Options button" << endl;

				//Destroy All Menu Sprites
				vector<Sprite*> allMenuSprites = GetAllSprites(Menu);
				for (Sprite* menuSprite : allMenuSprites) {
					if(menuSprite->getSpriteName().find(BACKGROUND_PREFIX) == string::npos)
						menuSprite->Destroy();
				}

				//Change Scene
				SetCurrentSceneName(SCENE_PREFIX "Options");
				SDL_SetWindowTitle(m_mainWindow, GetCurrentSceneName().c_str());
				//Set up scene
				//	spawn UI sprites
				//

				//tempResolutionButton.Create(m_Renderer, );
			}
			else if (spriteToCheck->getSpriteName().find(BUTTON_PREFIX "QuitButton") != string::npos) {
				cout << "Clicked on Quit button" << endl;
				IS_RUNNING_MAIN = false;
			}
		}
	}
}



void Game::HandleEvents() {

	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type) {
		case SDL_QUIT:
			IS_RUNNING_MAIN = false;
			break;
		case SDL_KEYDOWN:  
			switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					IS_RUNNING_MAIN = false;
					break;
				case SDLK_r:
					Reset();
					break;
				default:
					break;//keydown event
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			ClickOnSprite(event, Menu.GetSpriteList());
			break;

		default:
			break;//event type
	}
}

void Game::Update() {
	count++;

	//count / FPS = Timer in seconds
	//cout << count << endl;
}

void Game::Render() {
	SDL_RenderClear(m_Renderer);
	//////////////////////////

	//Render Menu sprites
	for (Sprite* menuSprite : Menu.GetSpriteList()) 
		SDL_RenderCopy(m_Renderer, menuSprite->getTexture(), NULL, menuSprite->getRect());
	

	/////////////////////////
	SDL_RenderPresent(m_Renderer);
}

void Game::Reset() {

	SetCurrentSceneName(SCENE_PREFIX "MainMenu");
	SDL_SetWindowTitle(m_mainWindow, GetCurrentSceneName().c_str());

	vector<Sprite*> menuSprites = Menu.GetSpriteList();
	map< string, map<string, vector<string> > >::iterator iter1(g_SettingsFileMap.begin());
	
	//Iterate through the settings map, then populate the sprite data
	int iterIndex = 0;
	for (map< string, map<string, vector<string> > >::iterator iter1 = g_SettingsFileMap.begin(); iter1 != g_SettingsFileMap.end(); iter1++) {

		auto& settingName = (*iter1).first;
		if (settingName.find("SPRITE") != string::npos) {

			map<string, vector<string> >::iterator iter2((*iter1).second.begin());
			for (map<string, vector<string> >::iterator iter2 = (*iter1).second.begin(); iter2 != (*iter1).second.end(); iter2++) {

				auto& spriteName = (*iter2).first;
				auto& data = (*iter2).second;

				menuSprites.at(iterIndex)->setSpriteName(spriteName.c_str());
				menuSprites.at(iterIndex)->setPosition(stoi(data.at(0)), stoi(data.at(1)));
				menuSprites.at(iterIndex)->setDimenstions(stoi(data.at(2)), stoi(data.at(3)));
				menuSprites.at(iterIndex)->setTexture(m_Renderer, data.at(4).c_str());
				iterIndex++;
			}
		}
		else {
			
		}
	}
}

void Game::CreateInitialSaveSate() {

	string fileName = INITIAL_SAVE_STATE_FILE;
	string filePath = g_SaveStatesFolder + GlobalHelpers::GetOSSeparator() + fileName + ".config";
	
	ofstream tempFile;
	tempFile.open(filePath, ios::out);

	if (tempFile.is_open())
	{
		//Screen Resolution
		tempFile << "SS_RES\n";
		tempFile << "RESOLUTION," << DEFAULT_RESOLUTION_W << "," << DEFAULT_RESOLUTION_H << ",\n";

		//Main Menu Sprites
		tempFile << "SS_SPRITES\n";
		vector<Sprite*> menuSprites = Menu.GetSpriteList();
		for (int i = 0; i < menuSprites.size(); i++)
		{
			Sprite currentSprite = *menuSprites.at(i);
			SDL_Rect spriteRect = *menuSprites.at(i)->getRect();
			string spriteName = menuSprites.at(i)->getSpriteName();

			tempFile << spriteName << "," << spriteRect.x << "," << spriteRect.y << "," << spriteRect.w << "," << spriteRect.h << "," << currentSprite.getTextureSource() << ",\n";
		}

		tempFile.close();
	}
	else
	{
		cout << "Could not open save file for writing";
	}
	
}

void Game::Clean(){
	SDL_DestroyWindow(m_mainWindow);
	SDL_DestroyRenderer(m_Renderer);
	SDL_Quit();
	cout << "Game shut down successfully" << endl;
}


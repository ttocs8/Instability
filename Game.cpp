#include "Game.hpp"
//Diable fopen warnings
#pragma warning(disable:4996)

//using namespace std;


Scene sc_MainMenu;
Sprite bg1;
Sprite titleCard;

Sprite tempPlayButton;
Sprite tempQuitButton;
Sprite tempOptionsButton;
Sprite tempOptionsButton2;

Scene sc_OptionsMenu;
Sprite bg2;
Sprite screenResolutionOptionsText;
Sprite tempResolutionButton2;
Sprite tempResolutionButton3;

Scene sc_GameplayScene;
Sprite bg3;
Sprite backToMenuButton;


TTF_Font* g_Font;

string g_SaveStatesFolder;
map< string, map<string, vector<string> > > g_SettingsFileMap;

Game::Game() {
	m_GameDataFolder = SDL_GetPrefPath(ORG_NAME, APP_NAME);
	SDL_assert(strlen(m_GameDataFolder.c_str()) != 0);

	g_SaveStatesFolder = m_GameDataFolder + SAVE_STATE_FOLDER;
}

Game::~Game() {}


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

	//Load font library 
	//	ALWAYS init this AFTER SDL2 libs
	TTF_Init();

	//Create Main Menu sprites
	bg1.Create(m_Renderer, "Assets/menubackground3.png", w, h, 0, 0, "BG_BACKGROUND");
	titleCard.Create(m_Renderer, "Assets/titlecard.png", 500, 200, GetCurrentResolution().W / 2, GetCurrentResolution().H / 2, "TitleCard", true, 0, -100);
	tempPlayButton.Create(m_Renderer, "Assets/tempplay.png", 150, 75, GetCurrentResolution().W / 2, GetCurrentResolution().H / 2, "BT_PlayButton", true, 0, 25);
	tempQuitButton.Create(m_Renderer, "Assets/tempquit.png", 150, 75, GetCurrentResolution().W / 2, GetCurrentResolution().H / 2, "BT_QuitButton", true, 0, 125);
	tempOptionsButton.Create(m_Renderer, "Assets/tempoptions.png", 50, 50, GetCurrentResolution().W - 75, 25, "BT_OptionsCogwheel");
	sc_MainMenu.AddSpritesToList({ &bg1, &titleCard, &tempPlayButton, &tempQuitButton, &tempOptionsButton });
	sc_MainMenu.SetName(SCENE_PREFIX "MainMenu");


	//Create Options Menu Sprites
	bg2.Create(m_Renderer, "Assets/menubackground3.png", w, h, 0, 0, "BG_BACKGROUND2");
	tempOptionsButton2.Create(m_Renderer, "Assets/tempoptions2.png", 50, 50, GetCurrentResolution().W - 75, 25, "BT_ExitOptns");

	sc_OptionsMenu.SetName(SCENE_PREFIX "OptionsMenu");
	sc_OptionsMenu.AddSpriteToList(&tempOptionsButton2);
	sc_OptionsMenu.AddSpriteToList(&bg2);

	g_Font = TTF_OpenFont("Assets/Stifly.ttf", 24);
	screenResolutionOptionsText.Create(m_Renderer, NULL, 300, 50, 0, 0, "TXT_ScreenResolution");
	screenResolutionOptionsText.setText(m_Renderer, g_Font, FONT_COLOR_WHITE, "Screen Resolution");
	sc_OptionsMenu.AddSpriteToList(&screenResolutionOptionsText);

	//Create Gameplay Sprites
	sc_GameplayScene.SetName(SCENE_PREFIX "INSTABILITY");

	m_CurrentScene = sc_MainMenu;				


	if (isFirstTimeInit) 
		CreateInitialSaveSate();
	
	//Load all saved settings into map for runtime usage
	LoadSettingsFromConfig(g_SettingsFileMap);
	SDL_assert(!g_SettingsFileMap.empty());

	return 0;
}


void Game::GoToNextScene(string theButtonClicked) {

	m_CurrentScene.DisableAllSprites();
	string currentSceneName = m_CurrentScene.GetName();

	//MainMenu --> OptionsMenu
	//    or
	//MainMenu --> Gameplay
	if (currentSceneName.find(SCENE_PREFIX "MainMenu") != string::npos){
		if (theButtonClicked.find(BUTTON_PREFIX "OptionsCogwheel") != string::npos) {
			m_CurrentScene = sc_OptionsMenu;
		}
		else if (theButtonClicked.find(BUTTON_PREFIX "PlayButton") != string::npos) {
			m_CurrentScene = sc_GameplayScene;
		}
	}
	//OptionsMenu --> MainMenu
	else if (currentSceneName.find(SCENE_PREFIX "OptionsMenu") != string::npos && theButtonClicked.find(BUTTON_PREFIX "ExitOptns") != string::npos) {
		m_CurrentScene = sc_MainMenu;
	}
	else if (currentSceneName.find(SCENE_PREFIX "INSTABILITY") != string::npos && theButtonClicked.find(BUTTON_PREFIX "BackToMenu") != string::npos) {
		m_CurrentScene = sc_MainMenu;
	}

	m_CurrentScene.EnableAllSprites();

	SortSpritesForRendering();
}


void Game::SetResolution(int w, int h) {
	m_CurrentScreenResolution.W = w;
	m_CurrentScreenResolution.H = h;
}

void Game::SetUpGameplayGrid()
{
	const int gridRows = 4;
	const int gridCols = 9;

	int hexWidth = 95;
	int hexHeight = 90; 

	//CENTERED
	int gridXPos = (GetCurrentResolution().W / 2) - ( (gridCols * hexWidth) / 2);
	int gridYPos = (GetCurrentResolution().H / 2) - ( (gridRows * hexHeight) / 2);;

	Sprite* hexGrid[gridRows][gridCols]{};

	for (int i = 0; i < gridRows; i++) {
		for (int j = 0; j < gridCols; j++) {
			string nodeName = "Hexagon [" + to_string(i) + "," + to_string(j) + "]";
			hexGrid[i][j] = new Sprite(m_Renderer, "Assets/temphex.png", hexWidth, hexHeight, gridXPos + (j * hexWidth), gridYPos + (i *  (hexHeight + 5)), nodeName.c_str());

			//offset the Y of every other column
			if (j % 2 == 0) {
				hexGrid[i][j]->setYPos(hexGrid[i][j]->getRect()->y + (hexGrid[i][j]->getRect()->h) / 2);
			}

			sc_GameplayScene.AddSpriteToList(hexGrid[i][j]);
		}
	}

	m_IsGameplayGridSetUp = true;
}

void Game::ClickOnSprite(SDL_Event& theEvent, vector<Sprite*> theClickableSprites)
{
	SDL_Point mousePosition;

	mousePosition.x = theEvent.motion.x;
	mousePosition.y = theEvent.motion.y;

	for (Sprite* spriteToCheck : theClickableSprites) {

		//If the mouse is INSIDE the sprite rectangle upon clicking
		if (SDL_PointInRect(&mousePosition, spriteToCheck->getRect()) && theEvent.button.button == SDL_BUTTON_LEFT)
		{
			string currentSpriteName = spriteToCheck->getSpriteName();
			if (currentSpriteName.find(BUTTON_PREFIX "PlayButton") != string::npos && spriteToCheck->IsEnabled()) {

				cout << "Clicked on Play Button" << endl;

				if (!m_IsGameplayGridSetUp) {
					backToMenuButton.Create(m_Renderer, "Assets/tempoptions2.png", 50, 50, GetCurrentResolution().W - 75, 25, "BT_BackToMenu");
					sc_GameplayScene.AddSpriteToList(&backToMenuButton);

					//TEMPORARY BACKGROUND IMAGE - PLS REPLACE SOON ISH THANKS
					bg3.Create(m_Renderer, "Assets/menubackground3.png", GetCurrentResolution().W, GetCurrentResolution().H, 0, 0, "BG_BACKGROUND2");
					sc_GameplayScene.AddSpriteToList(&bg3);

					//Set up the grid
					// FOR NOW it is a 9x4 grid of hexagons
					//	In the future, read the grid wanted in from a file that describes how the grid should be set up
					SetUpGameplayGrid();
				}

				GoToNextScene(currentSpriteName);				
			}
			else if (currentSpriteName.find(BUTTON_PREFIX "OptionsCogwheel") != string::npos && spriteToCheck->IsEnabled()) {
				cout << "Clicked on Options button" << endl;
				
				GoToNextScene(currentSpriteName);

			}
			else if (currentSpriteName.find(BUTTON_PREFIX "ExitOptns") != string::npos && spriteToCheck->IsEnabled()) {
				cout << "Clicked on Exit Options Button" << endl;

				GoToNextScene(currentSpriteName);
			}
			else if (currentSpriteName.find(BUTTON_PREFIX "QuitButton") != string::npos && spriteToCheck->IsEnabled()) {
				cout << "Clicked on Quit button" << endl;
				IS_RUNNING_MAIN = false;
			}
			else if (currentSpriteName.find("Hexagon") != string::npos && spriteToCheck->IsEnabled()) {
				cout << "Clicked on " << currentSpriteName << endl;
			}
			else if (currentSpriteName.find(BUTTON_PREFIX "BackToMenu") != string::npos && spriteToCheck->IsEnabled()) {
				cout << "Clicked on Leave Game" << endl;

				GoToNextScene(currentSpriteName);
			}
		}
	}
}

void Game::HoverOverSprite(SDL_Event& theEvent, vector<Sprite*> theHoverableSprites) {
	SDL_Point mousePosition;

	mousePosition.x = theEvent.motion.x;
	mousePosition.y = theEvent.motion.y;

	for (Sprite* spriteToCheck : theHoverableSprites) {

		if (spriteToCheck->getSpriteName().find(BUTTON_PREFIX "PlayButton") != string::npos) {
			//Hovering over Play Button
			if (SDL_PointInRect(&mousePosition, spriteToCheck->getRect())) {
				//cout << "Hovering over play button" << endl;
				spriteToCheck->setTexture(m_Renderer, "Assets/tempplay_hover.png");
			}
			else //NOT hovering over Play Button
			{			
				spriteToCheck->setTexture(m_Renderer, "Assets/tempplay.png");
			}
		}

		if (spriteToCheck->getSpriteName().find(BUTTON_PREFIX "QuitButton") != string::npos) {
			//Hovering over Quit Button
			if (SDL_PointInRect(&mousePosition, spriteToCheck->getRect())) {
				spriteToCheck->setTexture(m_Renderer, "Assets/tempquit_hover.png");
			}
			else //NOT hovering over Quit Button
			{
				spriteToCheck->setTexture(m_Renderer, "Assets/tempquit.png");
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
					//Reset();
					break;
				default:
					break;//keydown event
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			ClickOnSprite(event, m_CurrentScene.m_ListOfSprites);
			break;
		case SDL_MOUSEMOTION:
			HoverOverSprite(event, m_CurrentScene.m_ListOfSprites);
			break;
		default:
			break;//event type
	}
}

void Game::Update() {
	count++;
	SDL_SetWindowTitle(m_mainWindow, m_CurrentScene.GetName().c_str());

    //count / FPS = Timer in seconds
	//cout << count / FPS << endl;
}

void Game::SortSpritesForRendering() {
	/**
	* BG  0
	* *   1
	* BT  2
	* TXT 3
	*/

	//replace the prefixes with numbers for sorting
	for (Sprite* sprite : m_CurrentScene.m_ListOfSprites) {
		string newName = sprite->getSpriteName();
		if (sprite->getSpriteName().find(BACKGROUND_PREFIX) != string::npos) {
			newName = "0" + sprite->getSpriteName().substr(2);
		}
		else if (sprite->getSpriteName().find(BUTTON_PREFIX) != string::npos) {
			newName = "2" + sprite->getSpriteName().substr(2);
		}
		else if (sprite->getSpriteName().find(TEXT_PREFIX) != string::npos) {
			newName = "3" + sprite->getSpriteName().substr(3);
		}
		else {
			newName = "1" + newName;
		}

		sprite->setSpriteName(newName.c_str());
	}


	//Sort
	int n = m_CurrentScene.m_ListOfSprites.size();
	for (int j = 0; j < n - 1; j++) {
		
		// Comparing adjacent elements
		char currentChar = m_CurrentScene.m_ListOfSprites[j]->m_SpriteName[0];
		char nextChar = m_CurrentScene.m_ListOfSprites[j + 1]->m_SpriteName[0];

		if (int(currentChar) > int(nextChar))

			// Swapping if in the wrong order
			swap(m_CurrentScene.m_ListOfSprites[j], m_CurrentScene.m_ListOfSprites[j + 1]);
	}

	//revert to original names
	for (Sprite* sprite : m_CurrentScene.GetSpriteList()) {
		string newName = sprite->getSpriteName();
		if (sprite->getSpriteName().substr(0,2).find("0") != string::npos) {
			newName = "BG_" + sprite->getSpriteName().substr(2);
		}
		else if (sprite->getSpriteName().substr(0, 2).find("2") != string::npos) {
			newName = "BT_" + sprite->getSpriteName().substr(2);
		}
		else if (sprite->getSpriteName().substr(0, 2).find("3") != string::npos) {
			newName = "TXT_" + sprite->getSpriteName().substr(3);
		}
		else if (sprite->getSpriteName().substr(0, 2).find("1") != string::npos) {
			newName = newName.substr(1);
		}

		sprite->setSpriteName(newName.c_str());
	}

}

void Game::Render() {
	SDL_RenderClear(m_Renderer);
	//////////////////////////	

	for (Sprite* spriteInScene : m_CurrentScene.GetSpriteList())
		if (spriteInScene->IsEnabled()) 
			SDL_RenderCopy(m_Renderer, spriteInScene->getTexture(), NULL, spriteInScene->getRect());

	/////////////////////////
	SDL_RenderPresent(m_Renderer);
}

//void Game::Reset() {
//
//	//SetCurrentSceneName(SCENE_PREFIX "MainMenu");
//	m_CurrentScene.DisableAllSprites();
//	m_CurrentScene = sc_MainMenu;
//	m_CurrentScene.EnableAllSprites();
//
//	vector<Sprite*> menuSprites = sc_MainMenu.GetSpriteList();
//	map< string, map<string, vector<string> > >::iterator iter1(g_SettingsFileMap.begin());
//	
//	//Iterate through the settings map, then populate the sprite data
//	int iterIndex = 0;
//	for (map< string, map<string, vector<string> > >::iterator iter1 = g_SettingsFileMap.begin(); iter1 != g_SettingsFileMap.end(); iter1++) {
//
//		auto& settingName = (*iter1).first;
//		if (settingName.find("SPRITE") != string::npos) {
//
//			map<string, vector<string> >::iterator iter2((*iter1).second.begin());
//			for (map<string, vector<string> >::iterator iter2 = (*iter1).second.begin(); iter2 != (*iter1).second.end(); iter2++) {
//
//				auto& spriteName = (*iter2).first;
//				auto& data = (*iter2).second;
//
//				menuSprites.at(iterIndex)->setSpriteName(spriteName.c_str());
//				menuSprites.at(iterIndex)->setPosition(stoi(data.at(0)), stoi(data.at(1)));
//				menuSprites.at(iterIndex)->setDimenstions(stoi(data.at(2)), stoi(data.at(3)));
//				menuSprites.at(iterIndex)->setTexture(m_Renderer, data.at(4).c_str());
//				menuSprites.at(iterIndex)->Enable();
//				iterIndex++;
//			}
//		}
//	}
//}

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
		vector<Sprite*> menuSprites = sc_MainMenu.GetSpriteList();
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
	TTF_CloseFont(g_Font);
	TTF_Quit();
	cout << "Game shut down successfully" << endl;
}


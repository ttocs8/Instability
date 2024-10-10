#include "Game.hpp"
//Diable fopen warnings
#pragma warning(disable:4996)

using namespace std;


GUI Menu;
Sprite bg;
Sprite tempPlayButton;
Sprite tempQuitButton;
Sprite tempOptionsButton;
SDL_Rect* playrect;
string g_SaveStatesFolder;


Game::Game() {
	m_GameDataFolder = SDL_GetPrefPath(ORG_NAME, APP_NAME);
	SDL_assert(strlen(m_GameDataFolder.c_str()) != 0);

	g_SaveStatesFolder = m_GameDataFolder + SAVE_STATE_FOLDER;
}

Game::~Game() {}
void Game::SetCurrentSceneName(string theSceneName) {
	m_CurrentSceneName = theSceneName;
}

void LoadFromSaveState(map< string, map<string, vector<string> > >& TheSettingsFileMap) {
	//Read in initial save data and apply
	string filePath = g_SaveStatesFolder + GlobalHelpers::GetOSSeparator() + INITIAL_SAVE_STATE_FILE + ".config";
	ifstream tempFile;
	tempFile.open(filePath, ios::in);

	//map< string, map<string, vector<string> > > settingsMap;

	int lineNum = 0;
	if (tempFile.is_open())
	{
		vector<string> tokensToProcess;
		map < string, vector < string > > vectorOfTokensToProcess;
		vector<string> lines;
		int index = 0;
		for (string line; getline(tempFile, line, '\n'); ) {
			lines.push_back(line);
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
				string mainLine;
				while (i < 10) {
					mainLine = lines.at(lineNum - i);
					if (mainLine.find(SAVE_STATE_PREFIX) != string::npos) {
						break;
					}
					i++;
				}
				if (name.find(token) == string::npos) {
					TheSettingsFileMap[mainLine][name].push_back(token);
				}
				token = strtok(NULL, ",");
			}

			lineNum++;
		}
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
			int flags = 0;
			if (isFullscreen)
			{
				flags = SDL_WINDOW_FULLSCREEN;
			}

			if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
			{
				//Read in settings from file
				map< string, map<string, vector<string> > > TheSettingsFileMap;
				LoadFromSaveState(TheSettingsFileMap);

				m_mainWindow = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
				//SDL_SetWindowResizable(m_mainWindow,SDL_TRUE);
				if (m_mainWindow)
				{
					cout << "Window created." << endl;
				}

				m_Renderer = SDL_CreateRenderer(m_mainWindow, -1, 0);
				if (m_Renderer)
				{
					SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
					cout << "Renderer created." << endl;
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
			cout << "Initializing game data folders..." << endl;
			if (_mkdir(g_SaveStatesFolder.c_str()) == 0) {
				cout << "\t>> Created \"" << g_SaveStatesFolder << "\" folder for the first time" << endl;
			}
			else {
				//failed to create direcotry (could already be created, which is fine)
				// IGNORE FOR NOW //
			}

			struct stat info;
			//Assert created folders are Directories and is accessible
			SDL_assert(stat(m_GameDataFolder.c_str(), &info) == 0);
			SDL_assert(info.st_mode& S_IFDIR);

			SDL_assert(stat(g_SaveStatesFolder.c_str(), &info) == 0);
			SDL_assert(info.st_mode& S_IFDIR);

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
			CreateInitialSaveSate();

			return 0;
}

vector<Sprite*> Game::GetAllSprites(GUI& theGUI /*, Enemys enemyObjecsts, etc*/)
{
	return theGUI.GetSpriteList();
}


void Game::ClickOnSprite(SDL_Event& theEvent, Sprite* theSprite)
{
	SDL_Point mousePosition;

	mousePosition.x = theEvent.motion.x;
	mousePosition.y = theEvent.motion.y;

	//If the mouse is INSIDE the sprite rectangle, the user left clicks
	if (SDL_PointInRect(&mousePosition, theSprite->getRect()) && theEvent.button.clicks == 1 && theEvent.button.state == SDL_PRESSED) {

		if (theSprite->getSpriteName().find(BUTTON_PREFIX "PlayButton") != string::npos) {

			//Destroy All Menu Sprites
			vector<Sprite*> allMenuSprites = GetAllSprites(Menu);
			for (int i = 0; i < allMenuSprites.size(); i++)
				allMenuSprites.at(i)->Destroy();

			//Change Scene
			SetCurrentSceneName(SCENE_PREFIX "Game");
		}
		else if (theSprite->getSpriteName().find(BUTTON_PREFIX "OptionsButton") != string::npos) {
			cout << "Clicked on Options button" << endl;
		}
		else if (theSprite->getSpriteName().find(BUTTON_PREFIX "QuitButton") != string::npos) {
			cout << "Clicked on Quit button" << endl;
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
	count++;

	//count / FPS = Timer in seconds
	//cout << count << endl;
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

	SetCurrentSceneName(SCENE_PREFIX "MainMenu");

	//Read in initial save data and apply
	map< string, map<string, vector<string> > > TheSettingsFileMap;
	LoadFromSaveState(TheSettingsFileMap);

	vector<Sprite*> menuSprites = Menu.GetSpriteList();

	map< string, map<string, vector<string> > >::iterator iter1(TheSettingsFileMap.begin());
	
	int iterIndex = 0;
	for (map< string, map<string, vector<string> > >::iterator iter1 = TheSettingsFileMap.begin(); iter1 != TheSettingsFileMap.end(); iter1++) {

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
		tempFile << "RESOLUTION,1280,720\n";

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


#include "Game.hpp"
//Diable fopen warnings
#pragma warning(disable:4996)
#pragma warning(disable:6385)
//using namespace std;



//INITIALIZATION//////////////////////////////
TTF_Font* g_Font;
string g_SaveStatesFolder;
map< string, map<string, vector<string> > > g_SettingsFileMap;



///GAMEPLAY///////////////////////////////////
const int gridRows = 4;
const int gridCols = 9;
struct CoordPair {
	int X;
	int Y;
};
Sprite* hexGridSprites[gridRows][gridCols]{};
CoordPair defaultGridDataCoords[gridRows][gridCols];
int defaultGridData[gridRows][gridCols]
{
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0}
};

bool g_BuyTurretToggle = false;
bool g_BuyWallToggle = false;
bool g_BuyFactoryToggle = false;
bool g_UpgradeToggle = false;


SDL_TimerID wiggle_timerID = 0; //timer for hexagon click wiggle feedback effect

float g_Energy = 0.0f;           // Current total energy
float g_EnergyPerSecond = 1.0f;  // How much energy you gain per second, 1 by default
float g_EnergyTimer = 0.0f;      // Engergy accumulator (timer)

//SPRITES//////////////////////////////////
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
Sprite buyTurretButton;
Sprite buyWallButton;
Sprite buyFactoryButton;
Sprite upgradeButton;

Sprite resourceCounterText;
Sprite resourceCounterText2;

std::vector<std::unique_ptr<Sprite>> bulletList; //used for sprite rendering and cleanup

///////////////////////////////////
///////////////////////////////////

Game::Game() {
	m_GameDataFolder = SDL_GetPrefPath(ORG_NAME, APP_NAME);
	SDL_assert(strlen(m_GameDataFolder.c_str()) != 0);

	g_SaveStatesFolder = m_GameDataFolder + SAVE_STATE_FOLDER;
}

Game::~Game() {}


//VERY BAD 
// TODO: REDO ENTIRE DESIGN OF CONFIG FROM SCRATCH, THIS SUCKS
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
		//something happened, failed to create directory
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
	if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_TIMER) == 0) {
		
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
			windowFlags = SDL_WINDOW_FULLSCREEN_DESKTOP;
		
		SetResolution(w, h);
		m_mainWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, windowFlags);
		SDL_SetWindowResizable(m_mainWindow,SDL_TRUE);

		if (m_mainWindow) {
			cout << "Window created..." << endl;
		}

		m_Renderer = SDL_CreateRenderer(m_mainWindow, -1, 0);
		if (m_Renderer) {
			SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
			SDL_RenderSetLogicalSize(m_Renderer, RENDER_RESOLUTION_W, RENDER_RESOLUTION_H);
			cout << "Renderer created..." << endl;
		}

		std::string pathToIcon = ASSETS_FOLDER + GlobalHelpers::GetOSSeparator() + "turret_upgraded.png";
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
	bg1.Create(m_Renderer, "Assets/menubackground5.png", -1, -1, 0, 0, "BG_BACKGROUND");
	titleCard.Create(m_Renderer, "Assets/titlecard2.png", -1, -1, RENDER_RESOLUTION_W / 2, RENDER_RESOLUTION_H / 2, "TitleCard", true);
	tempPlayButton.Create(m_Renderer, "Assets/tempplay.png", -1, -1, RENDER_RESOLUTION_W / 2, RENDER_RESOLUTION_H / 2, "BT_PlayButton", true, 0, 25);
	tempQuitButton.Create(m_Renderer, "Assets/tempquit.png", -1, -1, RENDER_RESOLUTION_W / 2, RENDER_RESOLUTION_H / 2, "BT_QuitButton", true, 0, 75);
	tempOptionsButton.Create(m_Renderer, "Assets/tempoptions.png", -1, -1, RENDER_RESOLUTION_W - 75, 25, "BT_OptionsCogwheel");
	sc_MainMenu.AddSpritesToList({ &bg1, &titleCard, &tempPlayButton, &tempQuitButton, &tempOptionsButton });
	sc_MainMenu.SetName(SCENE_PREFIX "MainMenu");


	//Create Options Menu Sprites
	bg2.Create(m_Renderer, "Assets/menubackground5.png", -1, -1, 0, 0, "BG_BACKGROUND2");
	tempOptionsButton2.Create(m_Renderer, "Assets/tempoptions2.png", -1, -1, RENDER_RESOLUTION_W - 75, 25, "BT_ExitOptns");

	sc_OptionsMenu.SetName(SCENE_PREFIX "OptionsMenu");
	sc_OptionsMenu.AddSpriteToList(&tempOptionsButton2);
	sc_OptionsMenu.AddSpriteToList(&bg2);

	g_Font = TTF_OpenFont("Assets/Stifly.ttf", 24);
	screenResolutionOptionsText.Create(m_Renderer, NULL, 300, 50, 0, 0, "TXT_ScreenResolution");
	screenResolutionOptionsText.setText(m_Renderer, g_Font, COLOR_WHITE, "Screen Resolution");
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

void Game::UpdateGrid()
{
	for (int i = 0; i < gridRows; i++) {
		for (int j = 0; j < gridCols; j++) {
			switch (defaultGridData[i][j]) {
			
				case TURRET_DEFAULT:
					hexGridSprites[i][j]->setTexture(m_Renderer, "Assets/turret.png");
					break;
				case TURRET_UPGRADE1:
					hexGridSprites[i][j]->setTexture(m_Renderer, "Assets/turret_upgraded.png");
					break;

				case WALL_DEFAULT:
					hexGridSprites[i][j]->setTexture(m_Renderer, "Assets/wall.png");
					break;
				case WALL_UPGRADE1:
					hexGridSprites[i][j]->setTexture(m_Renderer, "Assets/wall_upgraded.png");
					break;

				case FACTORY_DEFAULT:
					hexGridSprites[i][j]->setTexture(m_Renderer, "Assets/factory.png");
					break;
				case FACTORY_UPGRADE1:
					hexGridSprites[i][j]->setTexture(m_Renderer, "Assets/factory_upgraded.png");
					break;

				default:
					hexGridSprites[i][j]->setTexture(m_Renderer, "Assets/temphex.png");
			}
			
		}
	}
}

void Game::ResetGridPosition()
{
	for (int i = 0; i < gridRows; i++) {
		for (int j = 0; j < gridCols; j++) {
			int currentX = hexGridSprites[i][j]->getRect()->x;
			int currentY = hexGridSprites[i][j]->getRect()->y;
			int defaultX = defaultGridDataCoords[i][j].X;
			int defaultY = defaultGridDataCoords[i][j].Y;

			if (currentY != defaultY )
				hexGridSprites[i][j]->setYPos(defaultY);

			if (currentX != defaultX)
				hexGridSprites[i][j]->setXPos(defaultX);
		}
	}

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
			m_CurrentScene.m_SceneEnum = 1;
		}
		else if (theButtonClicked.find(BUTTON_PREFIX "PlayButton") != string::npos) {
			m_CurrentScene = sc_GameplayScene;
			g_BuyTurretToggle = false;
			g_BuyWallToggle = false;
			g_BuyFactoryToggle = false;
			g_UpgradeToggle = false;
			UpdateGrid();
			m_CurrentScene.m_SceneEnum = 2;
		}
	}
	//OptionsMenu --> MainMenu
	else if (currentSceneName.find(SCENE_PREFIX "OptionsMenu") != string::npos && theButtonClicked.find(BUTTON_PREFIX "ExitOptns") != string::npos) {
		m_CurrentScene = sc_MainMenu;
		m_CurrentScene.m_SceneEnum = 0;
	}
	else if (currentSceneName.find(SCENE_PREFIX "INSTABILITY") != string::npos && theButtonClicked.find(BUTTON_PREFIX "BackToMenu") != string::npos) {
		g_Energy = 0;
		for (int i = 0; i < gridRows; i++) {
			for (int j = 0; j < gridCols; j++) {
				defaultGridData[i][j] = 0; 
			}
		}
		m_CurrentScene = sc_MainMenu;
		m_CurrentScene.m_SceneEnum = 0;
		
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
	
	//CENTERED
	int gridXPos = (RENDER_RESOLUTION_W / 2) - ( (gridCols * hexWidth) / 2);
	int gridYPos = (RENDER_RESOLUTION_H / 2) - ( (gridRows * hexHeight) / 2);


	for (int i = 0; i < gridRows; i++) {
		for (int j = 0; j < gridCols; j++) {
			string nodeName = "Hexagon [ " + to_string(i) + " , " + to_string(j) + " ]";
			hexGridSprites[i][j] = new Sprite(m_Renderer, "Assets/temphex.png", hexWidth, hexHeight, gridXPos + (j * (hexWidth - (hexHeight/6) + 12)) - 50, gridYPos + (i *  (hexHeight + 14)), nodeName.c_str());
			
			//offset the Y of every other column
			if (j % 2 == 0) {
				hexGridSprites[i][j]->setYPos((hexGridSprites[i][j]->getRect()->y + (hexGridSprites[i][j]->getRect()->h) / 2)+2);
			}
			defaultGridDataCoords[i][j].X = hexGridSprites[i][j]->getRect()->x;
			defaultGridDataCoords[i][j].Y = hexGridSprites[i][j]->getRect()->y;
			sc_GameplayScene.AddSpriteToList(hexGridSprites[i][j]);			
		}
	}

	m_IsGameplayGridSetUp = true;
}

vector<int> getHexGridIndex(string str)
{
	stringstream ss;
	vector<int> foundInds;
	ss << str;

	string temp;
	int found;
	while (!ss.eof()) {

		// extracting word by word
		ss >> temp;

		// Checking the word is an int or not 
		if (stringstream(temp) >> found)
			foundInds.push_back(found);

		// To save from space at the end of string 
		temp = "";
	}

	return foundInds;
}
mutex mtx;
//POSSIBLE MEMORY LEAK HERE
//MEMORY INCREASING WITH EACH CLICK
Uint32 hexagon_wiggle_feedback(Uint32 interval, void* param)
{
	mtx.lock();
	Sprite* param1 = reinterpret_cast<Sprite*>(param);

	int currentYpos = param1->getRect()->y;
	param1->setYPos(currentYpos - 1);

	mtx.unlock();
	return 0;
}

void Game::ClickOnSprite(SDL_Event& theEvent, vector<Sprite*> theClickableSprites)
{
	SDL_Point mousePosition;

	mousePosition.x = theEvent.motion.x;
	mousePosition.y = theEvent.motion.y;

	for (Sprite* spriteToCheck : theClickableSprites) {

		//If the mouse is INSIDE the sprite rectangle upon clicking
		if (SDL_PointInRect(&mousePosition, spriteToCheck->getRect()) && theEvent.type == SDL_MOUSEBUTTONDOWN && theEvent.button.button == SDL_BUTTON_LEFT)
		{
			string currentSpriteName = spriteToCheck->getSpriteName();
			if (currentSpriteName.find(BUTTON_PREFIX "PlayButton") != string::npos && spriteToCheck->IsEnabled()) {

				cout << "Clicked on Play Button" << endl;

				if (!m_IsGameplayGridSetUp) {
					backToMenuButton.Create(m_Renderer, "Assets/tempoptions2.png", -1, -1, RENDER_RESOLUTION_W - 75, 25, "BT_BackToMenu");
					sc_GameplayScene.AddSpriteToList(&backToMenuButton);

					//TEMPORARY BACKGROUND IMAGE - PLS REPLACE SOON ISH THANKS
					bg3.Create(m_Renderer, "Assets/stars.png", RENDER_RESOLUTION_W, RENDER_RESOLUTION_H, 0, 0, "BG_BACKGROUND2");
					sc_GameplayScene.AddSpriteToList(&bg3);

					resourceCounterText.Create(m_Renderer, NULL, 100, 25, RENDER_RESOLUTION_W - 150, 75, "TXT_ResourceCounter");
					resourceCounterText.setText(m_Renderer, g_Font, COLOR_WHITE, "ENERGY: ");

					resourceCounterText2.Create(m_Renderer, NULL, 15, 25, RENDER_RESOLUTION_W - 50, 75, "TXT_ResourceCounter2");

					buyTurretButton.Create(m_Renderer, NULL, 110, 25, RENDER_RESOLUTION_W - 150, 105, "TXT_BuyTurret");
					buyWallButton.Create(m_Renderer, NULL, 85, 25, RENDER_RESOLUTION_W - 150, 135, "TXT_BuyWall");
					buyFactoryButton.Create(m_Renderer, NULL, 120, 25, RENDER_RESOLUTION_W - 150, 165, "TXT_BuyFactory");
					upgradeButton.Create(m_Renderer, NULL, 110, 25, RENDER_RESOLUTION_W - 150, 205, "TXT_Upgrade");

					sc_GameplayScene.AddSpriteToList(&resourceCounterText);
					sc_GameplayScene.AddSpriteToList(&resourceCounterText2);
					sc_GameplayScene.AddSpriteToList(&buyTurretButton);
					sc_GameplayScene.AddSpriteToList(&buyWallButton);
					sc_GameplayScene.AddSpriteToList(&buyFactoryButton);
					sc_GameplayScene.AddSpriteToList(&upgradeButton);

					//Set up the grid
					// FOR NOW it is a 9x4 grid of hexagons
					//	In the future, read the grid wanted in from a file that describes how the grid should be set up
					SetUpGameplayGrid();
				}

				buyTurretButton.setText(m_Renderer, g_Font, COLOR_WHITE, "TURRET: " + to_string(TURRET_COST));
				buyWallButton.setText(m_Renderer, g_Font, COLOR_WHITE, "WALL: " + to_string(WALL_COST));
				buyFactoryButton.setText(m_Renderer, g_Font, COLOR_WHITE, "FACTORY: " + to_string(FACTORY_COST));
				upgradeButton.setText(m_Renderer, g_Font, COLOR_WHITE, "UPGRADE");

				


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
				SDL_RemoveTimer(wiggle_timerID);


				buyTurretButton.setText(m_Renderer, g_Font, COLOR_WHITE, "TURRET: " + to_string(TURRET_COST));
				buyWallButton.setText(m_Renderer, g_Font, COLOR_WHITE, "WALL: " + to_string(WALL_COST));
				buyFactoryButton.setText(m_Renderer, g_Font, COLOR_WHITE, "FACTORY: " + to_string(FACTORY_COST));
				upgradeButton.setText(m_Renderer, g_Font, COLOR_WHITE, "UPGRADE");

				cout << "Clicked on " << currentSpriteName << endl;
				vector<int> inds = getHexGridIndex(currentSpriteName);
				int row = inds.at(0);
				int col = inds.at(1);

				int currentYpos = spriteToCheck->getRect()->y;
				spriteToCheck->setYPos(currentYpos + 1);
				
				wiggle_timerID = SDL_AddTimer(105, hexagon_wiggle_feedback, spriteToCheck);

				//Turret buying
				if (g_BuyTurretToggle) {
					if (g_Energy >= TURRET_COST && defaultGridData[row][col] == 0) {
						g_Energy -= TURRET_COST;
						defaultGridData[row][col] = TURRET_DEFAULT;
					}

					g_BuyTurretToggle = false;
				}
				//Wall Buying
				else if (g_BuyWallToggle) {
					if (g_Energy >= WALL_COST && defaultGridData[row][col] == 0) {
						g_Energy -= WALL_COST;
						defaultGridData[row][col] = WALL_DEFAULT;
					}

					g_BuyWallToggle = false;
				}
				//Factory Buying
				else if (g_BuyFactoryToggle) {
					if (g_Energy >= FACTORY_COST && defaultGridData[row][col] == 0) {
						g_Energy -= FACTORY_COST;
						defaultGridData[row][col] = FACTORY_DEFAULT;
						g_EnergyPerSecond += 2;
					}

					g_BuyFactoryToggle = false;
				}
				//Upgrading anything
				else if (g_UpgradeToggle) {

					if (g_Energy >= UPGRADE_COST) {

						g_Energy -= UPGRADE_COST;

						switch (defaultGridData[row][col]) {
							case TURRET_DEFAULT:
								defaultGridData[row][col] = TURRET_UPGRADE1;
								break;
							case WALL_DEFAULT:
								defaultGridData[row][col] = WALL_UPGRADE1;
								break;
							case FACTORY_DEFAULT:
								defaultGridData[row][col] = FACTORY_UPGRADE1;
								g_EnergyPerSecond += 3;
								break;
							default:
								break;
						}
					}
				}

				UpdateGrid();
			}
			else if (currentSpriteName.find(BUTTON_PREFIX "BackToMenu") != string::npos && spriteToCheck->IsEnabled()) {
				cout << "Clicked on Leave Game" << endl;

				GoToNextScene(currentSpriteName);
			}
			else if (currentSpriteName.find(TEXT_PREFIX "BuyTurret") != string::npos && spriteToCheck->IsEnabled()) {
				cout << "Clicked on BUY TURRET" << endl;
				g_BuyWallToggle = false;
				g_BuyFactoryToggle = false;
				g_UpgradeToggle = false;
				buyWallButton.setText(m_Renderer, g_Font, COLOR_WHITE, "WALL: " + to_string(WALL_COST));
				buyFactoryButton.setText(m_Renderer, g_Font, COLOR_WHITE, "FACTORY: " + to_string(FACTORY_COST));
				upgradeButton.setText(m_Renderer, g_Font, COLOR_WHITE, "UPGRADE");


				if (g_BuyTurretToggle) {
					g_BuyTurretToggle = false;
					buyTurretButton.setText(m_Renderer, g_Font, COLOR_WHITE, "TURRET: " + to_string(TURRET_COST));
				}
				else {
					g_BuyTurretToggle = true;
					buyTurretButton.setText(m_Renderer, g_Font, COLOR_RED, "TURRET: " + to_string(TURRET_COST));
				}
				
			}
			else if (currentSpriteName.find(TEXT_PREFIX "BuyWall") != string::npos && spriteToCheck->IsEnabled()) {
				cout << "Clicked on BUY WALL" << endl;
				g_BuyTurretToggle = false;
				g_BuyFactoryToggle = false;
				g_UpgradeToggle = false;
				buyTurretButton.setText(m_Renderer, g_Font, COLOR_WHITE, "TURRET: " + to_string(TURRET_COST));
				buyFactoryButton.setText(m_Renderer, g_Font, COLOR_WHITE, "FACTORY: " + to_string(FACTORY_COST));
				upgradeButton.setText(m_Renderer, g_Font, COLOR_WHITE, "UPGRADE");


				if (g_BuyWallToggle) {
					g_BuyWallToggle = false;
					buyWallButton.setText(m_Renderer, g_Font, COLOR_WHITE, "WALL: " + to_string(WALL_COST));
				}
				else {
					g_BuyWallToggle = true;
					buyWallButton.setText(m_Renderer, g_Font, COLOR_RED, "WALL: " + to_string(WALL_COST));
				}
			}
			else if (currentSpriteName.find(TEXT_PREFIX "BuyFactory") != string::npos && spriteToCheck->IsEnabled()) {
				cout << "Clicked on BUY FACTORY" << endl;
				g_BuyWallToggle = false;
				g_BuyTurretToggle = false;
				g_UpgradeToggle = false;
				buyTurretButton.setText(m_Renderer, g_Font, COLOR_WHITE, "TURRET: " + to_string(TURRET_COST));
				buyWallButton.setText(m_Renderer, g_Font, COLOR_WHITE, "WALL: " + to_string(WALL_COST));
				upgradeButton.setText(m_Renderer, g_Font, COLOR_WHITE, "UPGRADE");


				if (g_BuyFactoryToggle) {
					g_BuyFactoryToggle = false;
					buyFactoryButton.setText(m_Renderer, g_Font, COLOR_WHITE, "FACTORY: " + to_string(FACTORY_COST));
				}
				else {
					g_BuyFactoryToggle = true;
					buyFactoryButton.setText(m_Renderer, g_Font, COLOR_RED, "FACTORY: " + to_string(FACTORY_COST));
				}

			}
			else if (currentSpriteName.find(TEXT_PREFIX "Upgrade") != string::npos && spriteToCheck->IsEnabled()) {
				cout << "Clicked on UPGRADE" << endl;
				g_BuyWallToggle = false;
				g_BuyTurretToggle = false;
				g_BuyFactoryToggle = false;
				buyTurretButton.setText(m_Renderer, g_Font, COLOR_WHITE, "TURRET: " + to_string(TURRET_COST));
				buyFactoryButton.setText(m_Renderer, g_Font, COLOR_WHITE, "FACTORY: " + to_string(FACTORY_COST));
				buyWallButton.setText(m_Renderer, g_Font, COLOR_WHITE, "WALL: " + to_string(WALL_COST));


				if (g_UpgradeToggle) {
					g_UpgradeToggle = false;
					upgradeButton.setText(m_Renderer, g_Font, COLOR_WHITE, "UPGRADE");
				}
				else {
					g_UpgradeToggle = true;
					upgradeButton.setText(m_Renderer, g_Font, COLOR_PURPLE, "UPGRADE");
				}

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


		if (spriteToCheck->getSpriteName().find(BUTTON_PREFIX "OptionsCogwheel") != string::npos) {
			//Hovering over Options Cogwheel Button

			//play anymation
			if (SDL_PointInRect(&mousePosition, spriteToCheck->getRect())) {
				
				spriteToCheck->setTexture(m_Renderer, "Assets/tempquit_hover.png");
				
				spriteToCheck->setTexture(m_Renderer, "Assets/tempoptions.png");

			}
			else //NOT hovering over Quit Button
			{
				spriteToCheck->setTexture(m_Renderer, "Assets/tempoptions.png");
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
				case SDLK_1:
					SDL_SetWindowSize(m_mainWindow, 1920, 1080);
					SDL_SetWindowFullscreen(m_mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
					break;
				case SDLK_2:
					SDL_SetWindowSize(m_mainWindow, 1280, 720);
					SDL_SetWindowFullscreen(m_mainWindow, 0);
					break;
				case SDLK_3:
					SDL_SetWindowSize(m_mainWindow, 960, 540);
					SDL_SetWindowFullscreen(m_mainWindow, 0);
					break;
				default:
					break;//keydown event
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			ClickOnSprite(event, m_CurrentScene.m_ListOfSprites);
			break;
		case SDL_MOUSEBUTTONUP:
			ClickOnSprite(event, m_CurrentScene.m_ListOfSprites);
			break;
		case SDL_MOUSEMOTION:
			HoverOverSprite(event, m_CurrentScene.m_ListOfSprites);
			break;
		default:
			break;//event type
	}
}

void Game::Update(float deltaTime) {
	//0 - main menu
	//1 - options menu
	//2 - gameplay
	if (m_CurrentScene.m_SceneEnum == 2) {
		if (g_Energy < 0) {
			g_Energy = 0;
		}
		if ((int)g_Energy != m_LastDisplayedEnergy) {
			resourceCounterText2.setText(m_Renderer, g_Font, COLOR_RED, to_string((int)g_Energy));
			m_LastDisplayedEnergy = (int)g_Energy;
		}


		//Turret Bullets
		static float defaultTurretTimer[gridRows][gridCols] = { 0 };   // One timer per turret
		static float upgradedTurretTimer[gridRows][gridCols] = { 0 };   // One timer per turret
		

		for (int i = 0; i < gridRows; i++) {
			for (int j = 0; j < gridCols; j++) {
				if (defaultGridData[i][j] == TURRET_DEFAULT) {

					defaultTurretTimer[i][j] -= deltaTime;

					if (defaultTurretTimer[i][j] <= 0.0f) {
						std::string name = "friendlyTurretLaser_" + std::to_string(i) + "_" + std::to_string(j);

						bulletList.push_back(
							std::make_unique<Sprite>(m_Renderer, "Assets/friendly_laser.png",-1, -1,
							defaultGridDataCoords[i][j].X + 20,
							defaultGridDataCoords[i][j].Y,
							name.c_str())
						);

						m_CurrentScene.AddSpriteToList(bulletList.back().get());

						defaultTurretTimer[i][j] = 1.0f;   // 1 shot per second
					}
				}
				if (defaultGridData[i][j] == TURRET_UPGRADE1) {

					upgradedTurretTimer[i][j] -= deltaTime;

					if (upgradedTurretTimer[i][j] <= 0.0f) {
						std::string name = "friendlyTurretLaser_upgrade1_" + std::to_string(i) + "_" + std::to_string(j);

						bulletList.push_back(
							std::make_unique<Sprite>(m_Renderer, "Assets/friendly_laser_upgraded.png", -1, -1,
								defaultGridDataCoords[i][j].X + 20,
								defaultGridDataCoords[i][j].Y,
								name.c_str())
						);

						m_CurrentScene.AddSpriteToList(bulletList.back().get());

						upgradedTurretTimer[i][j] = 0.5f;   // 2 shot per second
					}
				}
			}
		}

		//Doubles as the game timer - always counts up, so extra processing is done here
		g_EnergyTimer += deltaTime;

	

		std::vector<Sprite*> spritesToRemove;

		// Move + check off-screen EVERY FRAME
		for (auto it = bulletList.begin(); it != bulletList.end(); ) {
			auto* s = it->get();

			float newY = s->getRect()->y - 400.0f * deltaTime;
			s->setYPos(static_cast<int>(newY));

			if (newY < -50.0f) {
				//remove from render list + delete from memory
				RemoveSpriteFromRenderingList(s);    
				it = bulletList.erase(it);  
			}
			else {
				++it;
			}
		}


		if (g_EnergyTimer >= 1.0f)
		{
			g_Energy += g_EnergyPerSecond;
			g_EnergyTimer -= 1.0f;


			//Reset every second, probably need a better way of doing this
			////////////////////
			ResetGridPosition();
			//////////////////////


		}
	}
}

void Game::SortSpritesForRendering() {
	/**
	* Higher number = topmost layer
	* 
	* BG (background)   0
	* default			1
	* BT (buttons)		2
	* TXT (text)		3
	*/

	//replace the prefixes with numbers for sorting, following the above comments rules
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
			newName = "TXT_" + sprite->getSpriteName().substr(2);
		}
		else if (sprite->getSpriteName().substr(0, 2).find("1") != string::npos) {
			newName = newName.substr(1);
		}

		sprite->setSpriteName(newName.c_str());
	}

}
void Game::RemoveSpriteFromRenderingList(Sprite* spriteToRemove)
{
	auto& list = m_CurrentScene.m_ListOfSprites;
	list.erase(
		std::remove(list.begin(), list.end(), spriteToRemove),
		list.end()
	);
}

void Game::Render() {
	SDL_RenderClear(m_Renderer);
	//////////////////////////	

	//Loop through all sprites in the current scene, if its enabled, render to screen
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


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

//SCENES & SPRITES//////////////////////////////////
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
	//Unused for now, will be where config files are stored
	m_GameDataFolder = SDL_GetPrefPath(ORG_NAME, APP_NAME);
}

Game::~Game() {}


//Windows does a funny bit where it freezes the main thread when moving a nonfullscreen window around
//This function is basically a copy of main() and ensures the main thread keeps running. Added at the end of init() as an SDL EventWatch
//		-not perfect, if you SLOWLY drag the window, it still pauses and then catches up, so idk man
static int GameLoopEventWatch(void* userdata, SDL_Event* event)
{
	if (event->type == SDL_WINDOWEVENT && (event->window.event == SDL_WINDOWEVENT_MOVED || event->window.event == SDL_WINDOWEVENT_RESIZED)) {
		Game* game = reinterpret_cast<Game*>(userdata);

		Uint32 now = SDL_GetTicks64();
		if (game->m_LastFrameTime == 0) game->m_LastFrameTime = now;
		game->m_DeltaTime = (now - game->m_LastFrameTime) / 1000.0f;
		game->m_LastFrameTime = now;

		game->HandleEvents();
		game->Update(game->m_DeltaTime);
		game->Render();
	}
	return 0;
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

	//used for asserting files/directories exist
	struct stat info;

	int w = DEFAULT_RESOLUTION_W;
	int h = DEFAULT_RESOLUTION_H;

	if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_TIMER) == 0) {
		
		int windowFlags = 0;
		if (isFullscreen)
			windowFlags = SDL_WINDOW_FULLSCREEN_DESKTOP;
		
		SetResolution(w, h);
		m_mainWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, windowFlags);
		//SDL_SetWindowResizable(m_mainWindow,SDL_TRUE);

		if (m_mainWindow) {
			cout << "Window created..." << endl;
		}

		m_Renderer = SDL_CreateRenderer(m_mainWindow, -1, 0);
		if (m_Renderer) {
			SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
			SDL_RenderSetLogicalSize(m_Renderer, RENDER_RESOLUTION_W, RENDER_RESOLUTION_H);
			cout << "Renderer created..." << endl;
		}

		//set application icon
		std::string pathToIcon = ASSETS_FOLDER + GlobalHelpers::GetOSSeparator() + "icon.png";
		SDL_Surface* iconSurface = IMG_Load(pathToIcon.c_str());
		SDL_SetWindowIcon(m_mainWindow, iconSurface);
		SDL_FreeSurface(iconSurface);

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
	sc_MainMenu = Scene("MainMenu", MENU);
	bg1.Create(m_Renderer, "Assets/menubackground6.png", -1, -1, 0, 0, "BG_BACKGROUND", false);
	titleCard.Create(m_Renderer, "Assets/titlecard2.png", -1, -1, RENDER_RESOLUTION_W / 2, RENDER_RESOLUTION_H / 2, "TitleCard", false, true);
	tempPlayButton.Create(m_Renderer, "Assets/tempplay.png", -1, -1, RENDER_RESOLUTION_W / 2, RENDER_RESOLUTION_H / 2, "BT_PlayButton", true, true, 0, 25);
	tempQuitButton.Create(m_Renderer, "Assets/tempquit.png", -1, -1, RENDER_RESOLUTION_W / 2, RENDER_RESOLUTION_H / 2, "BT_QuitButton", true, true, 0, 75);
	tempOptionsButton.Create(m_Renderer, "Assets/tempoptions.png", -1, -1, RENDER_RESOLUTION_W - 75, 25, "BT_OptionsCogwheel",true);
	sc_MainMenu.AddSpritesToRenderList({ &bg1, &titleCard, &tempPlayButton, &tempQuitButton, &tempOptionsButton });
	sc_MainMenu.AddClickableSpritesToList({ &bg1, &titleCard, &tempPlayButton, &tempQuitButton, &tempOptionsButton });
	//sc_MainMenu.SetName(SCENE_PREFIX "MainMenu");


	//Create Options Menu Sprites
	tempOptionsButton2.Create(m_Renderer, "Assets/tempoptions2.png", -1, -1, RENDER_RESOLUTION_W - 75, 25, "BT_ExitOptns", true);
	sc_OptionsMenu = Scene("OptionsMenu", OPTIONS);
	sc_OptionsMenu.AddSpritesToRenderList({ &tempOptionsButton2, &bg1 });
	sc_OptionsMenu.AddClickableSpriteToList(&tempOptionsButton2);

	g_Font = TTF_OpenFont("Assets/Stifly.ttf", 24);
	screenResolutionOptionsText.Create(m_Renderer, NULL, 300, 50, 0, 0, "TXT_ScreenResolution", false);
	screenResolutionOptionsText.setText(m_Renderer, g_Font, COLOR_WHITE, "Screen Resolution");
	sc_OptionsMenu.AddSpriteToRenderList(&screenResolutionOptionsText);
	sc_OptionsMenu.AddClickableSpriteToList(&screenResolutionOptionsText);

	//Create Gameplay Scene and set it as current
	sc_GameplayScene = Scene("Gameplay", GAMEPLAY);
	m_CurrentScene = sc_MainMenu;				

	SDL_AddEventWatch(GameLoopEventWatch, this);
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
	int currentScene = m_CurrentScene.GetSceneType();

	//MainMenu --> OptionsMenu
	//    or
	//MainMenu --> Gameplay
	if (currentScene == MENU){
		if (theButtonClicked.find(BUTTON_PREFIX "OptionsCogwheel") != string::npos) {
			m_CurrentScene = sc_OptionsMenu;
		}
		else if (theButtonClicked.find(BUTTON_PREFIX "PlayButton") != string::npos) {
			m_CurrentScene = sc_GameplayScene;
			g_BuyTurretToggle = false;
			g_BuyWallToggle = false;
			g_BuyFactoryToggle = false;
			g_UpgradeToggle = false;
			UpdateGrid();
		}
	}
	//OptionsMenu --> MainMenu
	else if (currentScene == OPTIONS && theButtonClicked.find(BUTTON_PREFIX "ExitOptns") != string::npos) {
		m_CurrentScene = sc_MainMenu;
	}
	//Gameplay --> MainMenu
	else if (currentScene == GAMEPLAY && theButtonClicked.find(BUTTON_PREFIX "BackToMenu") != string::npos) {
		g_Energy = 0;
		for (int i = 0; i < gridRows; i++) {
			for (int j = 0; j < gridCols; j++) {
				defaultGridData[i][j] = 0; 
			}
		}

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
	
	//CENTERED
	int gridXPos = (RENDER_RESOLUTION_W / 2) - ( (gridCols * hexWidth) / 2);
	int gridYPos = (RENDER_RESOLUTION_H / 2) - ( (gridRows * hexHeight) / 2);


	for (int i = 0; i < gridRows; i++) {
		for (int j = 0; j < gridCols; j++) {
			string nodeName = "Hexagon [ " + to_string(i) + " , " + to_string(j) + " ]";
			hexGridSprites[i][j] = new Sprite(m_Renderer, "Assets/temphex.png", hexWidth, hexHeight, gridXPos + (j * (hexWidth - (hexHeight/6) + 12)) - 50, gridYPos + (i *  (hexHeight + 14)), nodeName.c_str(), true);
			
			//offset the Y of every other column
			if (j % 2 == 0) {
				hexGridSprites[i][j]->setYPos((hexGridSprites[i][j]->getRect()->y + (hexGridSprites[i][j]->getRect()->h) / 2)+2);
			}
			defaultGridDataCoords[i][j].X = hexGridSprites[i][j]->getRect()->x;
			defaultGridDataCoords[i][j].Y = hexGridSprites[i][j]->getRect()->y;
			sc_GameplayScene.AddSpriteToRenderList(hexGridSprites[i][j]);			
			sc_GameplayScene.AddClickableSpriteToList(hexGridSprites[i][j]);
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
Uint32 hexagon_wiggle_feedback(Uint32 interval, void* param)
{
	mtx.lock();
	Sprite* param1 = reinterpret_cast<Sprite*>(param);

	int currentYpos = param1->getRect()->y;
	param1->setYPos(currentYpos - 1);

	mtx.unlock();
	return 0;
}

void Game::ClickOnSprite(SDL_Event& theEvent, const vector<Sprite*> theClickableSprites)
{
	SDL_Point mousePosition;

	mousePosition.x = theEvent.motion.x;
	mousePosition.y = theEvent.motion.y;

	for (Sprite* spriteToCheck : theClickableSprites) {
		
		//If the mouse is INSIDE the sprite rectangle upon clicking
		if (spriteToCheck->IsClickable() && ( SDL_PointInRect(&mousePosition, spriteToCheck->getRect()) && theEvent.type == SDL_MOUSEBUTTONDOWN && theEvent.button.button == SDL_BUTTON_LEFT) )
		{
			string currentSpriteName = spriteToCheck->getSpriteName();
			if (currentSpriteName.find(BUTTON_PREFIX "PlayButton") != string::npos && spriteToCheck->IsEnabled()) {

				cout << "Clicked on Play Button" << endl;

				if (!m_IsGameplayGridSetUp) {
					backToMenuButton.Create(m_Renderer, "Assets/tempoptions2.png", -1, -1, RENDER_RESOLUTION_W - 75, 25, "BT_BackToMenu",true);
					sc_GameplayScene.AddSpriteToRenderList(&backToMenuButton);
					sc_GameplayScene.AddClickableSpriteToList(&backToMenuButton);

					//TEMPORARY BACKGROUND IMAGE - PLS REPLACE SOON ISH THANKS
					bg3.Create(m_Renderer, "Assets/stars.png", RENDER_RESOLUTION_W, RENDER_RESOLUTION_H, 0, 0, "BG_BACKGROUND2", false);
					sc_GameplayScene.AddSpriteToRenderList(&bg3);

					resourceCounterText.Create(m_Renderer, NULL, 100, 25, RENDER_RESOLUTION_W - 150, 75, "TXT_ResourceCounter",false);
					resourceCounterText.setText(m_Renderer, g_Font, COLOR_WHITE, "ENERGY: ");

					resourceCounterText2.Create(m_Renderer, NULL, 15, 25, RENDER_RESOLUTION_W - 50, 75, "TXT_ResourceCounter2", false);
					resourceCounterText2.setText(m_Renderer, g_Font, COLOR_RED, "0");

					buyTurretButton.Create(m_Renderer, NULL, 110, 25, RENDER_RESOLUTION_W - 150, 105, "TXT_BuyTurret", true);
					buyWallButton.Create(m_Renderer, NULL, 85, 25, RENDER_RESOLUTION_W - 150, 135, "TXT_BuyWall", true);
					buyFactoryButton.Create(m_Renderer, NULL, 120, 25, RENDER_RESOLUTION_W - 150, 165, "TXT_BuyFactory", true);
					upgradeButton.Create(m_Renderer, NULL, 110, 25, RENDER_RESOLUTION_W - 150, 205, "TXT_Upgrade", true);

					sc_GameplayScene.AddSpritesToRenderList({ &resourceCounterText, &resourceCounterText2, &buyTurretButton, &buyWallButton, &buyFactoryButton, &upgradeButton });
					sc_GameplayScene.AddClickableSpritesToList({ &buyTurretButton, &buyWallButton, &buyFactoryButton, &upgradeButton });
				

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

void Game::HoverOverSprite(SDL_Event& theEvent, const vector<Sprite*> theHoverableSprites) {
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
			if (SDL_PointInRect(&mousePosition, spriteToCheck->getRect())) {				
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
			ClickOnSprite(event, m_CurrentScene.m_ListOfClickableSprites);
			break;
		case SDL_MOUSEBUTTONUP:
			ClickOnSprite(event, m_CurrentScene.m_ListOfClickableSprites);
			break;
		case SDL_MOUSEMOTION:
			HoverOverSprite(event, m_CurrentScene.m_ListOfClickableSprites);
			break;
		default:
			break;//event type
	}
}

void Game::Update(float deltaTime) {
	
	if (m_CurrentScene.GetSceneType() == GAMEPLAY) {
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
							name.c_str(), 
							false)
						);

						m_CurrentScene.AddSpriteToRenderList(bulletList.back().get());

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
								name.c_str(),
								false)
						);

						m_CurrentScene.AddSpriteToRenderList(bulletList.back().get());

						upgradedTurretTimer[i][j] = 0.5f;   // 2 shot per second
					}
				}
			}
		}

		//Doubles as the game timer - always counts up, so extra processing is done here
		g_EnergyTimer += deltaTime;

	
		// Move bullet 
		for (auto it = bulletList.begin(); it != bulletList.end(); ) {
			auto* s = it->get();

			float newY = s->getRect()->y - 400.0f * deltaTime;
			s->setYPos(static_cast<int>(newY));

			//check off screen (this is happening every frame)
			if (newY < -25.0f) {
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
	for (Sprite* sprite : m_CurrentScene.m_ListOfSpritesToRender) {
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
	int n = m_CurrentScene.m_ListOfSpritesToRender.size();
	for (int pass = 0; pass < n - 1; pass++)
		for (int j = 0; j < n - 1 - pass; j++)
			if (int(m_CurrentScene.m_ListOfSpritesToRender[j]->m_SpriteName[0]) >
				int(m_CurrentScene.m_ListOfSpritesToRender[j + 1]->m_SpriteName[0]))
				swap(m_CurrentScene.m_ListOfSpritesToRender[j], m_CurrentScene.m_ListOfSpritesToRender[j + 1]);

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
	//auto& list = m_CurrentScene.m_ListOfSprites;
	m_CurrentScene.m_ListOfSpritesToRender.erase(
		std::remove(m_CurrentScene.m_ListOfSpritesToRender.begin(), m_CurrentScene.m_ListOfSpritesToRender.end(), spriteToRemove),
		m_CurrentScene.m_ListOfSpritesToRender.end()
	);

	m_CurrentScene.m_ListOfClickableSprites.erase(
		std::remove(m_CurrentScene.m_ListOfClickableSprites.begin(), m_CurrentScene.m_ListOfClickableSprites.end(), spriteToRemove),
		m_CurrentScene.m_ListOfClickableSprites.end()
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

void Game::Clean(){
	SDL_DestroyWindow(m_mainWindow);
	SDL_DestroyRenderer(m_Renderer);
	SDL_Quit();
	TTF_CloseFont(g_Font);
	TTF_Quit();
	cout << "Game shut down successfully" << endl;
}
#include <SDL_platform.h>
#include "SDL_ttf.h"
#include <string>
#include <iostream>
#ifdef _WIN32
#include <io.h> 
#define access    _access_s
#else
#include <unistd.h>
#endif

//Game Management Constants
#define ASSETS_FOLDER "Assets"
#define ORG_NAME "Perpetual Motion Software"
#define APP_NAME "Instability"
#define SAVE_STATE_FOLDER "SaveStates"
#define INITIAL_SAVE_STATE_FILE "__InitalGameState"
#define FIRST_TIME_INIT_FILENAME "INIT"
#define SEPARATOR_WIN "\\"
#define SEPARATOR_LNX "/"
#define WINDOWS "Windows"
#define LINUX "Linux"
#define MACOSX "Mac OS X"
#define IOS "iOS"
#define ANDROID "Android"
#define FPS 60


//Sprite / Naming Constants
#define SCENE_PREFIX "SC_"
#define SAVE_STATE_PREFIX "SS_"
#define BACKGROUND_PREFIX "BG_"
#define BUTTON_PREFIX "BT_"
#define TEXT_PREFIX "TXT_"

//Settings Constants
#define DEFAULT_RESOLUTION_W 1280
#define DEFAULT_RESOLUTION_H 720
#define RENDER_RESOLUTION_W 640
#define RENDER_RESOLUTION_H 360


#define COLOR_WHITE { 255, 255, 255 }
#define COLOR_RED { 236, 33, 40 }


//Gameplay Constants
#define TURRET_COST 20
#define WALL_COST 10
#define FACTORY_COST 30

typedef Uint32(*SDL_NewTimerCallback) (Uint32 interval, void* param);

class GlobalHelpers {
public:
	static std::string GetOSSeparator();
	static bool FileExists(const std::string& Filename); 
};
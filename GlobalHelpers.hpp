#include <SDL_platform.h>
#include <string>
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

//Settings Constants
#define DEFAULT_RESOLUTION_W 960
#define DEFAULT_RESOLUTION_H 540


class GlobalHelpers {
public:
	static std::string GetOSSeparator();
	static bool FileExists(const std::string& Filename);
};
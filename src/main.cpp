#include "F2ClickerApp.h"

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int show_command) {
    F2ClickerApp app(instance);
    return app.Run(show_command);
}

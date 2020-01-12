#ifndef MAIN_H
#define MAIN_H

#include <sp2/window.h>
#include <sp2/io/keybinding.h>

#include "controller.h"

extern sp::P<sp::Window> window;
extern sp::io::Keybinding escape_key;
extern Controller controller;

void openMainMenu();

#endif//MAIN_H
/*
Music from https://filmmusic.io
"Mining by Moonlight" by Kevin MacLeod (https://incompetech.com)
"Walking Along" by Kevin MacLeod (https://incompetech.com)
License: CC BY (http://creativecommons.org/licenses/by/4.0/)
 */
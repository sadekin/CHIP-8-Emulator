#pragma once

#include "chip8.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

#include <TGUI/TGUI.hpp>
#include <TGUI/Core.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/Widgets/Button.hpp>
#include <TGUI/Widgets/CheckBox.hpp>

class Emulator {
public:
    Emulator();
    void Run();

private:
    Chip8 chip8;

    void SetupGUI();
    void Render();
    void HandleInput();

    sf::RenderWindow window;
    tgui::Gui gui;
    tgui::ComboBox::Ptr romSelector;  // The dropdown menu (ComboBox) for ROM selection
};



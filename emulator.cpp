#include "emulator.h"

Emulator::Emulator()
        : chip8(), window(sf::VideoMode(DISPLAY_WIDTH * 15, DISPLAY_HEIGHT * 10), "CHIP-8") {
    chip8.LoadROM("../roms/Chip8 emulator Logo [Garstyciuks].ch8");
    SetupGUI();
}

void Emulator::Run() {
    while (window.isOpen()) {
        HandleInput();
        chip8.Cycle();

        if (chip8.drawFlag) Render();

        // Add a delay or limit the frame rate, so it doesn't Run too fast
        sf::sleep(sf::milliseconds(1));
    }
}

void Emulator::SetupGUI() {
    gui.setTarget(window);

    // Create ComboBox for ROM selection
    romSelector = tgui::ComboBox::create();
    romSelector->setSize(200, 30);
    romSelector->setPosition(700, 10);

    // Set the number of items to display before scrollbar is needed
    romSelector->setItemsToDisplay(9);

    // Populate the ComboBox with the names of available ROMs
    romSelector->addItem("PLEASE SELECT A GAME");
    romSelector->addItem("15PUZZLE");
    romSelector->addItem("BLINKY");
    romSelector->addItem("BLITZ");
    romSelector->addItem("BRIX");
    romSelector->addItem("CONNECT4");
    romSelector->addItem("GUESS");
    romSelector->addItem("HIDDEN");
    romSelector->addItem("INVADERS");
    romSelector->addItem("KALEID");
    romSelector->addItem("MAZE");
    romSelector->addItem("MERLIN");
    romSelector->addItem("MISSILE");
    romSelector->addItem("PONG");
    romSelector->addItem("PONG2");
    romSelector->addItem("PUZZLE");
    romSelector->addItem("SYZYGY");
    romSelector->addItem("TANK");
    romSelector->addItem("TETRIS");
    romSelector->addItem("TICTAC");
    romSelector->addItem("UFO");
    romSelector->addItem("VBRIX");
    romSelector->addItem("VERS");
    romSelector->addItem("WIPEOFF");

    // Setup the onItemSelect callback
    romSelector->onItemSelect([this](const tgui::String& item) {
        if (item.toStdString() == "PLEASE SELECT A GAME") return;
        std::string romPath = "../roms/" + item.toStdString();
        chip8.LoadROM(romPath);

        // Set the window title to the name of the ROM
        window.setTitle(item.toStdString());
    });

    gui.add(romSelector);
}

void Emulator::Render() {
    window.clear(sf::Color::Black);

    float pixelSize = 10.0; // Define the size of a pixel on the window

    for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
        for (int x = 0; x < DISPLAY_WIDTH; ++x) {
            if (chip8.display[y * DISPLAY_WIDTH + x]) {
                sf::RectangleShape pixel(sf::Vector2f(pixelSize, pixelSize));
                pixel.setPosition(x * pixelSize, y * pixelSize);
                pixel.setFillColor(sf::Color::White);
                window.draw(pixel);
            }
        }
    }

    gui.draw();
    window.display();
}


// =====================
// Chip8 Keypad Mapping
// =====================
//  1 2 3 C  ->  1 2 3 4
//  4 5 6 D  ->  Q W E R
//  7 8 9 E  ->  A S D F
//  A 0 B F  ->  Z X C V

void Emulator::HandleInput() {
    sf::Event event;
    while (window.pollEvent(event)) {
        gui.handleEvent(event);
        switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code) {
                    case sf::Keyboard::Num1: chip8.key[0x1] = 1; break;
                    case sf::Keyboard::Num2: chip8.key[0x2] = 1; break;
                    case sf::Keyboard::Num3: chip8.key[0x3] = 1; break;
                    case sf::Keyboard::Num4: chip8.key[0xC] = 1; break;

                    case sf::Keyboard::Q: chip8.key[0x4] = 1; break;
                    case sf::Keyboard::W: chip8.key[0x5] = 1; break;
                    case sf::Keyboard::E: chip8.key[0x6] = 1; break;
                    case sf::Keyboard::R: chip8.key[0xD] = 1; break;

                    case sf::Keyboard::A: chip8.key[0x7] = 1; break;
                    case sf::Keyboard::S: chip8.key[0x8] = 1; break;
                    case sf::Keyboard::D: chip8.key[0x9] = 1; break;
                    case sf::Keyboard::F: chip8.key[0xE] = 1; break;

                    case sf::Keyboard::Z: chip8.key[0xA] = 1; break;
                    case sf::Keyboard::X: chip8.key[0x0] = 1; break;
                    case sf::Keyboard::C: chip8.key[0xB] = 1; break;
                    case sf::Keyboard::V: chip8.key[0xF] = 1; break;

                    default: break;
                }
                break;
            case sf::Event::KeyReleased:
                switch (event.key.code) {
                    case sf::Keyboard::Num1: chip8.key[0x1] = 0; break;
                    case sf::Keyboard::Num2: chip8.key[0x2] = 0; break;
                    case sf::Keyboard::Num3: chip8.key[0x3] = 0; break;
                    case sf::Keyboard::Num4: chip8.key[0xC] = 0; break;

                    case sf::Keyboard::Q: chip8.key[0x4] = 0; break;
                    case sf::Keyboard::W: chip8.key[0x5] = 0; break;
                    case sf::Keyboard::E: chip8.key[0x6] = 0; break;
                    case sf::Keyboard::R: chip8.key[0xD] = 0; break;

                    case sf::Keyboard::A: chip8.key[0x7] = 0; break;
                    case sf::Keyboard::S: chip8.key[0x8] = 0; break;
                    case sf::Keyboard::D: chip8.key[0x9] = 0; break;
                    case sf::Keyboard::F: chip8.key[0xE] = 0; break;

                    case sf::Keyboard::Z: chip8.key[0xA] = 0; break;
                    case sf::Keyboard::X: chip8.key[0x0] = 0; break;
                    case sf::Keyboard::C: chip8.key[0xB] = 0; break;
                    case sf::Keyboard::V: chip8.key[0xF] = 0; break;

                    default: break;
                }
                break;
        }
    }
}





#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

using namespace sf;
using namespace std;

// Constants
const int windoww = 1000;
const int windowh = 700;
const int buttonw = 200;
const int buttonh = 50;

// Function to create a button
RectangleShape createButton(float x, float y, const Color& color) {
    RectangleShape button(Vector2f(buttonw, buttonh));
    button.setFillColor(color);
    button.setOutlineColor(Color::Black);
    button.setOutlineThickness(4);
    button.setPosition(x, y);
    return button;
}

// Function to handle the main menu
void handleMainMenu(RenderWindow& window, Font& font, int& currentState, Sound& click) {
    // Load background texture for main menu
    Texture texture;
    if (!texture.loadFromFile("battleship4.jpg")) {
        cout << "Error loading background texture!" << endl;
        return;
    }

    // Create and scale sprite
    Sprite background(texture);
    Vector2u textureSize = texture.getSize();
    background.setScale(static_cast<float>(windoww) / textureSize.x, static_cast<float>(windowh) / textureSize.y);

    // Create buttons and text
    RectangleShape playButton = createButton(400, 350, Color::Green);
    RectangleShape exitButton = createButton(400, 450, Color::Red);

    Text playText("PLAY", font, 30);
    playText.setFillColor(Color::Black);
    playText.setPosition(470, 355);

    Text exitText("EXIT", font, 30);
    exitText.setFillColor(Color::Black);
    exitText.setPosition(470, 455);

    // Main menu loop
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
                return;
            }

            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

                if (playButton.getGlobalBounds().contains(mousePos)) {
                    // soubd
                    click.play();
                    currentState = 1;//loading
                    return;
                }
                else if (exitButton.getGlobalBounds().contains(mousePos)) {
                    //s
                    window.close();  // Exit the program
                    click.play();
                    return;
                }
            }
        }

        // Change button colors based on mouse position
        Vector2i mousePos = Mouse::getPosition(window);
        Vector2f mousePosf(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        if (playButton.getGlobalBounds().contains(mousePosf)) {
            playButton.setFillColor(Color::Yellow);
        }
        else {
            playButton.setFillColor(Color::Green);
        }

        if (exitButton.getGlobalBounds().contains(mousePosf)) {
            exitButton.setFillColor(Color::Yellow);
        }
        else {
            exitButton.setFillColor(Color::Red);
        }

        // Render
        window.clear();
        window.draw(background);
        window.draw(playButton);
        window.draw(exitButton);
        window.draw(playText);
        window.draw(exitText);
        window.display();
    }
}

bool isHunting = false;
Vector2i lastHit(-1, -1);
vector<Vector2i> huntTargets;



// Function to handle the loading screen
void handleLoadingScreen(RenderWindow& window, Font& font, int& currentState) {
    // Set up the black screen with loading text
    Text loadingText("Loading...", font, 40);
    loadingText.setFillColor(Color::Green);
    loadingText.setPosition(windoww / 2 - loadingText.getLocalBounds().width / 2, windowh / 2 - 100);

    // Set up the progress bar rectangle
    RectangleShape progressBarOutline(Vector2f(600, 30));
    progressBarOutline.setFillColor(Color::Black);
    progressBarOutline.setPosition(windoww / 2 - 300, windowh / 2);

    RectangleShape progressBar(Vector2f(0, 30)); // Initial width 0
    progressBar.setFillColor(Color::Green);
    progressBar.setPosition(windoww / 2 - 300, windowh / 2);

    Clock clock;
    float duration = 3.0f; // 3 seconds to fill the progress bar

    // Loading screen loop
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
                return;
            }
        }

        // Update progress bar width based on time passed
        float elapsedTime = clock.getElapsedTime().asSeconds();
        if (elapsedTime <= duration) {
            float progressWidth = (elapsedTime / duration) * 600;  // Fill the bar over 3 seconds
            progressBar.setSize(Vector2f(progressWidth, 30));
        }
        else {
            // Once the bar is filled, transition to the next screen (e.g., the score screen)
            currentState = 2;
            return;
        }

        // Render
        window.clear(Color::Black);
        window.draw(loadingText);
        window.draw(progressBarOutline);
        window.draw(progressBar);
        window.display();
    }
}

const int GRID_SIZE = 10;
const int CELL_SIZE = 40;

// Ship structure
struct Ship {
    string name;
    int size;
    int hits = 0;
};

// Game board
struct Board {
    char grid[GRID_SIZE][GRID_SIZE];

    Board() {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                grid[i][j] = '.';
            }
        }
    }
};
// function to get adjacent cells
vector<Vector2i> getAdjacentCells(int x, int y) {
    vector<Vector2i> neighbors;
    if (x > 0) neighbors.push_back(Vector2i(x - 1, y));
    if (x < GRID_SIZE - 1) neighbors.push_back(Vector2i(x + 1, y));
    if (y > 0) neighbors.push_back(Vector2i(x, y - 1));
    if (y < GRID_SIZE - 1) neighbors.push_back(Vector2i(x, y + 1));
    return neighbors;
}
vector<Ship> ships = {
    {"Aircraft Carrier", 5},
    {"Battleship", 4},
    {"Submarine", 3},
    {"Cruiser", 3},
    {"Destroyer", 2}
};

// place ships on the board randomly axis by user
bool placeShip(Board& board, int x, int y, int size, bool isHorizontal) {
    if (isHorizontal) {
        if (x + size > GRID_SIZE) return false; // ship would be out of size
        for (int i = 0; i < size; i++) {
            if (board.grid[y][x + i] != '.') return false; // space already occupied
        }
        for (int i = 0; i < size; i++) {
            board.grid[y][x + i] = 'S'; // place ship
        }
    }
    else {
        if (y + size > GRID_SIZE) return false; // ship would be out of size
        for (int i = 0; i < size; i++) {
            if (board.grid[y + i][x] != '.') return false; // space already occupied
        }
        for (int i = 0; i < size; i++) {
            board.grid[y + i][x] = 'S'; // place ship
        }
    }
    return true;
}

//function of the game board
void renderBoard(RenderWindow& window, Board& board, int X, int Y, bool hideShips) {
    RectangleShape cell(Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
    cell.setOutlineThickness(1);
    cell.setOutlineColor(Color::Blue);

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            cell.setPosition(X + j * CELL_SIZE, Y + i * CELL_SIZE);

            if (board.grid[i][j] == 'S' && !hideShips) {
                cell.setFillColor(Color::Blue); // ship
            }
            else if (board.grid[i][j] == 'X') {
                cell.setFillColor(Color::Red); // hit
            }
            else if (board.grid[i][j] == 'O') {
                cell.setFillColor(Color::White); // miss
            }
            else {
                cell.setFillColor(Color(173, 216, 230)); // water
            }

            window.draw(cell);
        }
    }
}


bool attack(Board& board, int x, int y) {
    if (board.grid[y][x] == 'S') {
        board.grid[y][x] = 'X'; //hit
        return true;
    }
    else if (board.grid[y][x] == '.') {
        board.grid[y][x] = 'O'; //miss
    }
    return false;
}

// sunk ships
bool allShipsSunk(Board& board) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (board.grid[i][j] == 'S') return false;
        }
    }
    return true;
}

void playBackgroundMusic(Music& music) {
    if (!music.openFromFile("music.mp3")) {
        cout << "Error loading background music!" << endl;
        return;
    }
    music.setLoop(true);
    music.play();
}
int playerScore = 100;
string winner;
void handleGameScreen(RenderWindow& window, Font& font, int& currentState, Sound& select) {
    Texture playerBackground, computerBackground;
    playerBackground.loadFromFile("back.jpg");
    computerBackground.loadFromFile("back.jpg");

    Sprite playerBackgroundSprite(playerBackground);
    Sprite computerBackgroundSprite(computerBackground);

    playerBackgroundSprite.setScale(static_cast<float>(windoww) / playerBackground.getSize().x,
        static_cast<float>(windowh) / playerBackground.getSize().y);
    computerBackgroundSprite.setScale(static_cast<float>(windoww) / computerBackground.getSize().x,
        static_cast<float>(windowh) / computerBackground.getSize().y);

    Board playerBoard, computerBoard;

    // track player moves (whether they have clicked a cell or not)
    bool playerMoves[GRID_SIZE][GRID_SIZE] = { false };
    int placedShips = 0;



    // randomly place computer ships
    for (Ship& ship : ships) {
        bool placed = false;
        while (!placed) {
            int x = rand() % GRID_SIZE;
            int y = rand() % GRID_SIZE;
            bool isHorizontal = rand() % 2;

            if (placeShip(computerBoard, x, y, ship.size, isHorizontal)) {
                placed = true;
            }
        }
    }

    // player ship placement
    while (placedShips < ships.size()) {
        int x = -1, y = -1;
        bool isHorizontal = false;
        bool validPlacement = false;

        while (!validPlacement) {
            window.clear(Color::Black);
            window.draw(playerBackgroundSprite);
            renderBoard(window, playerBoard, 50, 100, false); // render player board
            renderBoard(window, computerBoard, 140 + GRID_SIZE * CELL_SIZE + 20, 100, true); // render computer board

            Font font;
            font.loadFromFile("arial.ttf");
            Text instruction("Click to place your ships.", font, 20);
            instruction.setFillColor(Color::White);
            instruction.setPosition(55, 65);
            window.draw(instruction);

            Text labelP("PLAYER BOARD", font, 20);
            labelP.setFillColor(Color::White);
            labelP.setPosition(180, 510);
            window.draw(labelP);

            Text labelc("COMPUTER BOARD", font, 20);
            labelc.setFillColor(Color::White);
            labelc.setPosition(660, 510);
            window.draw(labelc);

            window.display();

            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed) {
                    window.close();
                }
                else if (event.type == Event::MouseButtonPressed) {
                    x = (event.mouseButton.x - 50) / CELL_SIZE;
                    y = (event.mouseButton.y - 100) / CELL_SIZE;

                    if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
                        select.play();
                        isHorizontal = rand() % 2; // randomly determine orientation
                        if (placeShip(playerBoard, x, y, ships[placedShips].size, isHorizontal)) {
                            placedShips++;
                            validPlacement = true;
                        }
                    }
                }

            }
        }
    }

    bool isPlayerTurn = true;
    bool gameOver = false;
    //string winner;



    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }

            if (event.type == Event::MouseButtonPressed && isPlayerTurn && !gameOver) {
                int x = (event.mouseButton.x - 140 - GRID_SIZE * CELL_SIZE - 20) / CELL_SIZE;
                int y = (event.mouseButton.y - 100) / CELL_SIZE;

                if (x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE) {
                    select.play();
                    if (!playerMoves[y][x]) {
                        if (!attack(computerBoard, x, y)) {
                            playerScore -= 1;
                        }
                        else {
                            if (allShipsSunk(computerBoard)) {
                                gameOver = true;
                                winner = "PLAYER";
                                currentState = 3;
                                return;
                            }
                        }
                        playerMoves[y][x] = true;
                        isPlayerTurn = false;
                    }
                }
            }
        }

        if (!isPlayerTurn && !gameOver) {
            int x, y;
            if (isHunting && !huntTargets.empty()) {
                //target the next cell in the hunt list
                Vector2i target = huntTargets.back();
                huntTargets.pop_back();
                x = target.x;
                y = target.y;
            }
            else {
                //random attack
                do {
                    x = rand() % GRID_SIZE;
                    y = rand() % GRID_SIZE;
                } while (playerBoard.grid[y][x] == 'X' || playerBoard.grid[y][x] == 'O');
            }

            if (attack(playerBoard, x, y)) {
                // successful hit
                if (!isHunting) {
                    isHunting = true;
                    lastHit = Vector2i(x, y);
                    huntTargets = getAdjacentCells(x, y);
                }
                else {
                    // add new targets for hunting
                    vector<Vector2i> newTargets = getAdjacentCells(x, y);
                    huntTargets.insert(huntTargets.end(), newTargets.begin(), newTargets.end());
                }
                //remove invalid cells from huntTargets
                huntTargets.erase(remove_if(huntTargets.begin(), huntTargets.end(),
                    [&playerBoard](Vector2i cell) {
                        return playerBoard.grid[cell.y][cell.x] == 'X' ||
                            playerBoard.grid[cell.y][cell.x] == 'O';
                    }),
                    huntTargets.end());

                if (allShipsSunk(playerBoard)) {
                    huntTargets.clear();
                    gameOver = true;
                    winner = "Computer";
                    currentState = 3;
                    return;
                }
            }
            else {
                // If no hit, change to random attacks when no targets remain
                if (huntTargets.empty()) {
                    isHunting = false;
                    huntTargets.clear();
                }
            }
            isPlayerTurn = true;
        }

        window.clear(Color::Black);
        window.draw(playerBackgroundSprite);
        window.draw(computerBackgroundSprite);
        renderBoard(window, playerBoard, 50, 100, false);
        renderBoard(window, computerBoard, 140 + GRID_SIZE * CELL_SIZE + 20, 100, true);

        Font font;
        font.loadFromFile("GILLUBCD.ttf");
        Text scoreText("Score: " + to_string(playerScore), font, 30);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(420, 550);
        window.draw(scoreText);

        Text labelb("BATTLE BEGINS", font, 55);
        labelb.setFillColor(Color::Black);
        labelb.setPosition(310, 30);
        window.draw(labelb);

        if (gameOver) {
            Text result(winner + " WINS!", font, 30);
            result.setFillColor(Color::White);
            result.setPosition(400, 590);
            window.draw(result);
            currentState = 3;
            return;
        }

        window.display();
    }
}


//score window
void handleScoreScreen(RenderWindow& window, Font& font, int& currentState, Sound& click) {

    Texture texture;
    if (!texture.loadFromFile("winner2.jpg")) {
        cout << "Error loading background texture!" << endl;
        return;
    }

    //background image
    Sprite background(texture);
    Vector2u textureSize = texture.getSize();
    background.setScale(static_cast<float>(windoww) / textureSize.x, static_cast<float>(windowh) / textureSize.y);

    RectangleShape playButton = createButton(400, 370, Color::Green);
    RectangleShape exitButton = createButton(400, 570, Color::Red);
    RectangleShape mainButton = createButton(400, 470, Color::Red);

    Text playText("PLAY AGAIN", font, 30);
    playText.setFillColor(Color::Black);
    playText.setPosition(410, 375);

    Text mainText("MAIN MENU", font, 30);
    mainText.setFillColor(Color::Black);
    mainText.setPosition(410, 475);

    Text exitText("EXIT", font, 30);
    exitText.setFillColor(Color::Black);
    exitText.setPosition(470, 575);


    Text result(winner + " WINS!", font, 50);
    result.setFillColor(Color::Black);
    result.setPosition(320, 30);
    window.draw(result);

    Text scoreText("Score: " + to_string(playerScore), font, 50);
    scoreText.setFillColor(Color::Black);
    scoreText.setPosition(400, 100);
    window.draw(scoreText);

    // score screen loop
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
                return;
            }

            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

                if (playButton.getGlobalBounds().contains(mousePos)) {
                    click.play();
                    playerScore = 100;
                    currentState = 1;
                    return;
                }
                else if (mainButton.getGlobalBounds().contains(mousePos)) {
                    click.play();
                    currentState = 0;
                    return;
                }
                else if (exitButton.getGlobalBounds().contains(mousePos)) {
                    click.play();
                    window.close();
                    return;
                }
            }
        }

        // change button colors with mouse position
        Vector2i mousePos = Mouse::getPosition(window);
        Vector2f mousePosf(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        if (playButton.getGlobalBounds().contains(mousePosf)) {
            playButton.setFillColor(Color::Yellow);
        }
        else {
            playButton.setFillColor(Color::Green);
        }

        if (exitButton.getGlobalBounds().contains(mousePosf)) {
            exitButton.setFillColor(Color::Yellow);
        }
        else {
            exitButton.setFillColor(Color::Red);
        }
        if (mainButton.getGlobalBounds().contains(mousePosf)) {
            mainButton.setFillColor(Color::Yellow);
        }
        else {
            mainButton.setFillColor(Color(128, 0, 128));
        }

        // render
        window.clear();
        window.draw(background);
        window.draw(result);
        window.draw(scoreText);
        window.draw(playButton);
        window.draw(mainButton);
        window.draw(exitButton);
        window.draw(playText);
        window.draw(mainText);
        window.draw(exitText);
        window.display();
    }
}

int main() {
    RenderWindow window(VideoMode(windoww, windowh), "Battleship Menu");
    Font font;
    if (!font.loadFromFile("Beauty Sunny.otf")) {
        cout << "Error loading font!" << endl;
        return -1;
    }
    //click sound
    SoundBuffer clickBuffer;
    if (!clickBuffer.loadFromFile("select.mp3.mp3")) {
        cout << "Error loading click sound!" << endl;
        return -1;
    }

    SoundBuffer selectBuffer;
    if (!selectBuffer.loadFromFile("select.mp3.mp3")) {
        cout << "Error loading click sound!" << endl;
        return -1;
    }
    Music backgroundMusic;
    playBackgroundMusic(backgroundMusic);

    Sound click, select;
    click.setBuffer(clickBuffer);
    select.setBuffer(selectBuffer);


    int currentState = 0;

    // main loop
    while (window.isOpen())
    {
        switch (currentState)
        {
        case 0:  // main Menu
            handleMainMenu(window, font, currentState, click);
            break;
        case 1:  // loading Screen
            handleLoadingScreen(window, font, currentState);
            break;
        case 2:  // game Screen
            handleGameScreen(window, font, currentState, select);
            break;
        case 3:  // score Screen
            handleScoreScreen(window, font, currentState, click);
            break;
        }
    }

    return 0;
}

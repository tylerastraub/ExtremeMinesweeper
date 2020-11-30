#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include "Utility/SpriteSheet.hpp"

#include "SFML/Graphics.hpp"
#include <vector>

class GameState
{
public:
	GameState(int scaleIndex);
	~GameState() = default;
	void render(sf::RenderWindow* window);
	void handleEvent(sf::Window* window);
	void restart();

	void collisionCheck(sf::Vector2i mousePos, sf::Mouse::Button buttonPressed);
	void resetButtonCollisionCheck(sf::Vector2i mousePos, sf::Mouse::Button buttonPressed);
	void handleCollision(sf::Vector2i cellIndex, sf::Mouse::Button buttonPressed);
	int getCoordinate(sf::Vector2i cellIndex);
	void generateMap(int firstClickIndex);
	bool checkForFirstClick(int firstClickIndex, int index);
	int getSurroundingMines(int index);
	std::vector<int> getSurroundingTiles(int index);

	void handleClick(int index);
	void revealMines();
	void revealEmptySquares(int index);

	bool getRestartFlag();
	bool getScaleChangedFlag();
	sf::Vector2f getScale();
	int getScaleIndex();

	float getTimeAndResetPaddingSpace();
	float getWidth();
	float getHeight();

private:
	SpriteSheet _tileSheet;
	SpriteSheet _gameResetSprite;

	sf::Vector2i _tileSize;
	sf::Vector2f _scale;
	sf::Vector2f _scales[3] = {sf::Vector2f(1.f, 1.f), sf::Vector2f(2.f, 2.f), sf::Vector2f(3.f, 3.f)};
	int _scaleIndex = 0;

	std::vector<TileType> _map; // what is drawn to the screen
	std::vector<TileType> _cells; // the actual level code
	int _xCells = 30;
	int _yCells = 16;
	int _mines = 99;
	int _clearedTiles = 0;

	sf::Clock _gameTime;

	bool _clicking = false;
	bool _firstClick = true;
	bool _gameOver = false;
	bool _restartFlag = false;
	bool _scaleChangedFlag = false;
	bool _scaleKeyDown = false;
	bool _gameStart = false;
	bool _collidingWithResetButton = false;

	float _timeAndResetPaddingSpace = 40.f;
	const float SCREEN_WIDTH = 480.f;
	const float SCREEN_HEIGHT = 256.f + _timeAndResetPaddingSpace;
};

#endif
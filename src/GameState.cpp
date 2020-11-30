#include "GameState.hpp"

#include <iostream>
#include <cstdlib>
#include <cmath>

GameState::GameState(int scaleIndex) {
	srand(time(NULL));

	_scaleIndex = scaleIndex;
	_scale = _scales[scaleIndex];
	_tileSize = sf::Vector2i(16, 16);

	for(int i = 0; i < _xCells * _yCells; ++i) {
		_map.push_back(TileType::NEUTRAL);
		_cells.push_back(TileType::EMPTY);
	}

	if(!_tileSheet.load("content/minesweeper_tilesheet.png", sf::Vector2u(16, 16), _map, _xCells, _yCells))
		std::cout << "Failed to load content/minesweeper_tilesheet.png" << std::endl;

	if(!_gameResetSprite.load("content/resetbutton_tilesheet.png", sf::Vector2u(24, 24), ResetButtonType::DEFAULT, 1, 1))
		std::cout << "Failed to load content/minesweeper_tilesheet.png" << std::endl;

	_gameResetSprite.setOrigin(12, 12);
}

void GameState::render(sf::RenderWindow* window) {
	_tileSheet.setPosition(0, _timeAndResetPaddingSpace);
	window->draw(_tileSheet);
	_gameResetSprite.setPosition(SCREEN_WIDTH / 2.f, _timeAndResetPaddingSpace / 2.f);
	window->draw(_gameResetSprite);
}

void GameState::handleEvent(sf::Window* window) {
	if(!_clicking &&
	  (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) ||
	  sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
	  ) {
		_clicking = true;
		if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !_gameOver) {
			sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
			collisionCheck(mousePos, sf::Mouse::Button::Left);
			resetButtonCollisionCheck(mousePos, sf::Mouse::Button::Left);
		} 
		else if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) && !_gameOver) {
			sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
			collisionCheck(mousePos, sf::Mouse::Button::Right);
			resetButtonCollisionCheck(mousePos, sf::Mouse::Button::Left);
		}
		else if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
			sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
			resetButtonCollisionCheck(mousePos, sf::Mouse::Button::Left);
		}
	}
	else if(!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) &&
			!sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
		_clicking = false;
		_restartFlag = _collidingWithResetButton;
	}
	
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::S) && !_scaleKeyDown) {
		++_scaleIndex;
		if(_scaleIndex > 2) _scaleIndex = 0;
		_scale = _scales[_scaleIndex];
		_scaleChangedFlag = true;
		_scaleKeyDown = true;
	}
	else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
		_scaleKeyDown = false;
	}

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
		restart();
	}
}

void GameState::restart() {
	_restartFlag = true;
}

void GameState::collisionCheck(sf::Vector2i mousePos, sf::Mouse::Button buttonPressed) {
	if((mousePos.x > 0 && mousePos.x < _xCells * _tileSize.x * _scale.x) &&
		mousePos.y > _timeAndResetPaddingSpace * _scale.y &&
		mousePos.y < (_yCells * _tileSize.y + _timeAndResetPaddingSpace) * _scale.y) {
		handleCollision(sf::Vector2i(mousePos.x / (_tileSize.x * _scale.x),
			(mousePos.y - _timeAndResetPaddingSpace * _scale.y) / (_tileSize.y * _scale.y)),
			buttonPressed);
	}
}

void GameState::resetButtonCollisionCheck(sf::Vector2i mousePos, sf::Mouse::Button buttonPressed) {
	if(mousePos.x > (SCREEN_WIDTH / 2 - 12) * _scale.x &&
	   mousePos.x < (SCREEN_WIDTH / 2 + 12) * _scale.x &&
	   mousePos.y > (_timeAndResetPaddingSpace / 2 - 12) * _scale.y &&
	   mousePos.y < (_timeAndResetPaddingSpace / 2 + 12) * _scale.y &&
	   buttonPressed == sf::Mouse::Button::Left) {
		_collidingWithResetButton = true;
		_gameResetSprite.load("content/resetbutton_tilesheet.png", sf::Vector2u(24, 24), ResetButtonType::DEFAULT_PRESSED, 1, 1);
	}
	else if(!_gameOver) {
		_gameResetSprite.load("content/resetbutton_tilesheet.png", sf::Vector2u(24, 24), ResetButtonType::DEFAULT, 1, 1);
	}
	else {
		_gameResetSprite.load("content/resetbutton_tilesheet.png", sf::Vector2u(24, 24), ResetButtonType::GAME_OVER, 1, 1);
	}
}

void GameState::handleCollision(sf::Vector2i cellIndex, sf::Mouse::Button buttonPressed) {
	int absoluteIndex = getCoordinate(cellIndex);

	switch(_map[absoluteIndex]) {
		case TileType::NEUTRAL:
			if(!_firstClick) {
				if(buttonPressed == sf::Mouse::Button::Left) {
					handleClick(absoluteIndex);
				}
				else if(buttonPressed == sf::Mouse::Button::Right) {
					_map[absoluteIndex] = TileType::FLAGGED;
				}
			}
			else {
				if(buttonPressed == sf::Mouse::Button::Left) {
					generateMap(absoluteIndex);
					handleClick(absoluteIndex);
				}
			}
			break;
		case TileType::FLAGGED:
			if(buttonPressed == sf::Mouse::Button::Right) {
				_map[absoluteIndex] = TileType::QUESTION;
			}
			break;
		case TileType::QUESTION:
			if(buttonPressed == sf::Mouse::Button::Right) {
				_map[absoluteIndex] = TileType::NEUTRAL;
			}
			break;
		default:
			break;
	}

	// this is so unoptimized it's hilarious. but we can live with it for now
	_tileSheet.load("content/minesweeper_tilesheet.png", sf::Vector2u(16, 16), _map, _xCells, _yCells);
}

int GameState::getCoordinate(sf::Vector2i cellIndex) {
	return cellIndex.y * _xCells + cellIndex.x;
}

void GameState::generateMap(int firstClickIndex) {
	sf::Clock clock;
	std::cout << "Generating map..." << std::endl;
	int minesGenerated = 0;
	unsigned int currentIndex = 0;

	// lay out mines
	while(minesGenerated < _mines) {
		if(rand() % 10 == 0
		   && _cells[currentIndex] != TileType::MINE
		   && !checkForFirstClick(firstClickIndex, currentIndex)) {
			_cells[currentIndex] = TileType::MINE;
			++minesGenerated;
		}

		++currentIndex;
		if(currentIndex == _cells.size())
			currentIndex = 0;
	}

	// assign numbers
	for(int i = 0; i < _xCells * _yCells; ++i) {
		if(_cells[i] != TileType::MINE) {
			int surroundingMines = getSurroundingMines(i);
			switch(surroundingMines) {
				case 1:
					_cells[i] = TileType::ONE;
					break;
				case 2:
					_cells[i] = TileType::TWO;
					break;
				case 3:
					_cells[i] = TileType::THREE;
					break;
				case 4:
					_cells[i] = TileType::FOUR;
					break;
				case 5:
					_cells[i] = TileType::FIVE;
					break;
				case 6:
					_cells[i] = TileType::SIX;
					break;
				case 7:
					_cells[i] = TileType::SEVEN;
					break;
				case 8:
					_cells[i] = TileType::EIGHT;
					break;
				default:
					break;
			}
		}
	}

#if defined(_DEBUG)
	sf::Time time = clock.getElapsedTime();
	std::cout << "Map generated in ";
	if(time.asMilliseconds() < 1) {
		std::cout << "<1 ms";
	}
	else {
		std::cout << time.asMilliseconds() << " ms";
	}
	std::cout << std::endl;
#endif

	_firstClick = false;
	_gameTime.restart();
	_gameStart = true;
}

// makes sure we have a nice 3x3 square (at least) surrounding the first click
bool GameState::checkForFirstClick(int firstClickIndex, int index) {
	if(index == firstClickIndex ||
	   index == firstClickIndex - 1 ||
	   index == firstClickIndex + 1 ||
	   index - _xCells == firstClickIndex ||
	   index - _xCells == firstClickIndex - 1 ||
	   index - _xCells == firstClickIndex + 1 ||
	   index + _xCells == firstClickIndex ||
	   index + _xCells == firstClickIndex - 1 ||
	   index + _xCells == firstClickIndex + 1) {
		   return true;
	   }

	return false;
}

int GameState::getSurroundingMines(int index) {
	int mineCount = 0;
	std::vector<int> surroundingTiles = getSurroundingTiles(index);

	for(int tile : surroundingTiles) {
		if(_cells[tile] == TileType::MINE) ++mineCount;
	}

	return mineCount;
}

std::vector<int> GameState::getSurroundingTiles(int index) {
	std::vector<int> tiles;
	bool north, east, south, west = false;

	// north
	if(index >= _xCells) {tiles.push_back(index - _xCells); north = true;}
	// east
	if(index % _xCells != _xCells - 1) {tiles.push_back(index + 1); east = true;}
	// south
	if(index <= _xCells * _yCells - _xCells) {tiles.push_back(index + _xCells); south = true;}
	// west
	if(index % _xCells > 0) {tiles.push_back(index - 1); west = true;}

	if(north && west) tiles.push_back(index - _xCells - 1);
	if(north && east) tiles.push_back(index - _xCells + 1);
	if(south && west) tiles.push_back(index + _xCells - 1);
	if(south && east) tiles.push_back(index + _xCells + 1);

	return tiles;
}

void GameState::handleClick(int index) {
	if(_cells[index] == TileType::MINE) {
		_map[index] = TileType::DETONATED_MINE;
		_gameOver = true;
		_gameResetSprite.load("content/resetbutton_tilesheet.png", sf::Vector2u(24, 24), ResetButtonType::GAME_OVER, 1, 1);
		revealMines();
	}
	else if(_cells[index] == TileType::EMPTY) {
		_map[index] = _cells[index];
		revealEmptySquares(index);
	}
	else {
		_map[index] = _cells[index];
	}
}

void GameState::revealMines() {
	for(int i = 0; i < _xCells * _yCells; ++i) {
		if(_cells[i] == TileType::MINE && _map[i] == TileType::NEUTRAL) {
			_map[i] = TileType::MINE;
		}
		else if(_cells[i] != TileType::MINE && _map[i] == TileType::FLAGGED) {
			_map[i] = TileType::INCORRECT_MINE;
		}
	}
}

void GameState::revealEmptySquares(int index) {
	std::vector<int> destinations = getSurroundingTiles(index);

	for(int i : destinations) {
		if(_cells[i] == TileType::EMPTY && _map[i] == TileType::NEUTRAL) {
			_map[i] = _cells[i];
			revealEmptySquares(i);
		}
		else {
			_map[i] = _cells[i];
		}
	}
}

bool GameState::getRestartFlag() {
	return _restartFlag;
}

bool GameState::getScaleChangedFlag() {
	return _scaleChangedFlag;
}

sf::Vector2f GameState::getScale() {
	return _scale;
}

int GameState::getScaleIndex() {
	return _scaleIndex;
}

float GameState::getTimeAndResetPaddingSpace() {
	return _timeAndResetPaddingSpace;
}

float GameState::getWidth() {
	return SCREEN_WIDTH;
}

float GameState::getHeight() {
	return SCREEN_HEIGHT;
}

#include "GameState.hpp"
#include "Platform/Platform.hpp"

int main()
{
	util::Platform platform;
	GameState* gameState = new GameState(0);

#if defined(_DEBUG)
	std::cout << "Debugging enabled" << std::endl;
#endif

	sf::RenderWindow* window = new sf::RenderWindow();
	// in Windows at least, this must be called before creating the window
	float screenScalingFactor = platform.getScreenScalingFactor(window->getSystemHandle());
	// Use the screenScalingFactor
	window->create(sf::VideoMode(gameState->getWidth() * screenScalingFactor,
				   gameState->getHeight() * screenScalingFactor),
				   "Extreme Minesweeper",
				   sf::Style::Close);
	platform.setIcon(window->getSystemHandle());

	sf::Event event;

	while (window->isOpen())
	{
		if(gameState->getRestartFlag()) gameState = new GameState(gameState->getScaleIndex());
		if(gameState->getScaleChangedFlag())
			window->setSize(sf::Vector2u(gameState->getWidth() * gameState->getScale().x,
										 gameState->getHeight() * gameState->getScale().y));

		while (window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window->close();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				window->close();
			
			gameState->handleEvent(window);
		}

		window->clear(sf::Color(192, 192, 192));
		gameState->render(window);
		window->display();
	}

	return 0;
}

// TODO: implement clock and reset button, then get EXTREME
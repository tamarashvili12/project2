#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

class ObstacleCar
{
public:
    sf::Sprite sprite;
    sf::Vector2f velocity;

    ObstacleCar(sf::Texture& texture, float roadWidth, float borderLeft, float carSpeed)
    {
        sprite.setTexture(texture);
        float obstacleX = static_cast<float>(rand() % static_cast<int>(roadWidth - sprite.getGlobalBounds().width)) + borderLeft;
        float obstacleY = -sprite.getGlobalBounds().height;
        sprite.setPosition(obstacleX, obstacleY);
        velocity.x = 0.0f;
        velocity.y = carSpeed;

        // Scale the obstacle car sprite to a smaller size
        sf::Vector2f scaleFactor(0.3f, 0.3f);
        sprite.setScale(scaleFactor);
    }

    void update(float deltaTime)
    {
        sprite.move(velocity * deltaTime);
    }
};

class Game
{
private:
    sf::RenderWindow window;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Texture carTexture;
    sf::Sprite carSprite;
    std::vector<sf::Texture> obstacleTextures;
    sf::Texture explosionTexture;
    sf::Sprite explosionSprite;
    std::vector<ObstacleCar> obstacles;
    sf::Clock clock;
    sf::Music backgroundMusic;
    sf::SoundBuffer crashBuffer;
    sf::Sound crashSound;
    sf::Font font;
    sf::Text scoreText;
    int score;
    bool gameOver;

    const float CAR_SPEED = 200.0f;
    const float BORDER_LEFT = 150.0f;
    const float BORDER_RIGHT = 620.0f;

public:
    Game() : window(sf::VideoMode(800, 600), "SFML Car Game"), score(0), gameOver(false)
    {
    }

    void run()
    {
        setup();
        while (window.isOpen())
        {
            handleEvents();
            update();
            render();
        }
    }

private:
    void setup()
    {
        setupTextures();
        setupSprites();
        setupAudio();
        setupText();
        srand(static_cast<unsigned int>(time(NULL)));
    }

    void handleEvents()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
    }

    void update()
    {
        if (gameOver)
            return;

        float deltaTime = clock.restart().asSeconds();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            if (carSprite.getPosition().x > 0 && carSprite.getPosition().x > BORDER_LEFT)
                carSprite.move(-CAR_SPEED * deltaTime, 0);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            if (carSprite.getPosition().x + carSprite.getGlobalBounds().width < window.getSize().x &&
                carSprite.getPosition().x + carSprite.getGlobalBounds().width < BORDER_RIGHT)
            {
                carSprite.move(CAR_SPEED * deltaTime, 0);
            }
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            carSprite.move(0, -CAR_SPEED * deltaTime);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            carSprite.move(0, CAR_SPEED * deltaTime);
        }

        for (auto& obstacle : obstacles)
        {
            obstacle.update(deltaTime);

            // Calculate the distance between the car and obstacle centers
            sf::Vector2f carCenter = carSprite.getPosition() + sf::Vector2f(carSprite.getGlobalBounds().width / 2, carSprite.getGlobalBounds().height / 2);
            sf::Vector2f obstacleCenter = obstacle.sprite.getPosition() + sf::Vector2f(obstacle.sprite.getGlobalBounds().width / 2, obstacle.sprite.getGlobalBounds().height / 2);
            sf::Vector2f distance = carCenter - obstacleCenter;

            // Define a threshold distance for collision
            float collisionThreshold = 30.0f;

            // Check if the distance is less than the collision threshold
            if (std::abs(distance.x) < collisionThreshold && std::abs(distance.y) < collisionThreshold)
            {
                sf::Vector2f explosionPosition = obstacle.sprite.getPosition();
                float explosionScale = 0.15f; // Adjust the scale factor for a smaller explosion
                explosionSprite.setScale(explosionScale, explosionScale);
                explosionSprite.setPosition(explosionPosition.x + obstacle.sprite.getGlobalBounds().width / 2 - explosionSprite.getGlobalBounds().width / 2,
                                            explosionPosition.y + obstacle.sprite.getGlobalBounds().height / 2 - explosionSprite.getGlobalBounds().height / 2);

                crashSound.play();

                gameOver = true;
                backgroundMusic.stop();
                // Add game over logic here
            }
        }

        obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(), [&](const ObstacleCar& obstacle) {
            return obstacle.sprite.getPosition().y > window.getSize().y;
        }), obstacles.end());

        if (obstacles.empty() || obstacles.back().sprite.getPosition().y > 100.0f)
        {
            spawnObstacle();
            score++; // Increment the score
        }

        scoreText.setString("Score: " + std::to_string(score));
    }

    void render()
    {
        window.clear();
        window.draw(backgroundSprite);

        for (const auto& obstacle : obstacles)
        {
            window.draw(obstacle.sprite);
        }

        window.draw(carSprite);

        if (gameOver)
        {
            window.draw(explosionSprite);
        }

        window.draw(scoreText);

        window.display();
    }

    void setupTextures()
    {
        if (!backgroundTexture.loadFromFile("background.png"))
        {
            // Handle error loading the texture
            std::exit(1);
        }

        if (!carTexture.loadFromFile("car.png"))
        {
            // Handle error loading the texture
            std::exit(1);
        }

        obstacleTextures.resize(2);
        if (!obstacleTextures[0].loadFromFile("obstacle1.png") || !obstacleTextures[1].loadFromFile("obstacle2.png"))
        {
            // Handle error loading the obstacle textures
            std::exit(1);
        }

        if (!explosionTexture.loadFromFile("explosion.png"))
        {
            // Handle error loading the explosion texture
            std::exit(1);
        }
    }

    void setupSprites()
    {
        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setScale(
            static_cast<float>(window.getSize().x) / backgroundSprite.getTexture()->getSize().x,
            static_cast<float>(window.getSize().y) / backgroundSprite.getTexture()->getSize().y
        );

        carSprite.setTexture(carTexture);
        carSprite.setPosition(
            window.getSize().x / 2 - carSprite.getGlobalBounds().width / 2,
            window.getSize().y - carSprite.getGlobalBounds().height - 20
        );

        explosionSprite.setTexture(explosionTexture);
    }

    void setupAudio()
    {
        if (!backgroundMusic.openFromFile("background_music.ogg"))
        {
            // Handle error loading the music
            std::exit(1);
        }

        backgroundMusic.setLoop(true);
        backgroundMusic.play();

        if (!crashBuffer.loadFromFile("crash_sound.wav"))
        {
            // Handle error loading the sound effect
            std::exit(1);
        }

        crashSound.setBuffer(crashBuffer);
    }

    void setupText()
    {
        if (!font.loadFromFile("Oswald.ttf"))
        {
            // Handle error loading the font
            std::exit(1);
        }

        scoreText.setFont(font);
        scoreText.setCharacterSize(30); // Increase the character size to 30
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10.0f, 10.0f);
        scoreText.setString("Score: " + std::to_string(score));
    }


    void spawnObstacle()
    {
        int textureIndex = rand() % obstacleTextures.size();

        // Adjust the range of random X positions slightly inward from the borders
        float minObstacleX = BORDER_LEFT + 50.0f;
        float maxObstacleX = BORDER_RIGHT - 50.0f;
        float obstacleX = static_cast<float>(rand() % static_cast<int>(maxObstacleX - minObstacleX)) + minObstacleX;

        ObstacleCar newObstacle(obstacleTextures[textureIndex], window.getSize().x, BORDER_LEFT, CAR_SPEED);
        newObstacle.sprite.setPosition(obstacleX, -newObstacle.sprite.getGlobalBounds().height);

        obstacles.push_back(newObstacle);
    }


    bool isCollision(const sf::Sprite& sprite1, const sf::Sprite& sprite2)
    {
        return sprite1.getGlobalBounds().intersects(sprite2.getGlobalBounds());
    }
};

int main()
{
    Game game;
    game.run();

    return 0;
}

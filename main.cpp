#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <omp.h>

struct Particle {
    float x, y;
    float vx, vy;
    int lifetime;
    int max_lifetime;
    sf::Color color;
};

int main() {
    const int NUM_PARTICLES = 2000; 
    std::vector<Particle> particles(NUM_PARTICLES);
    
    std::srand(std::time(nullptr));

    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].x = 400.0f + (std::rand() % 40 - 20);
        particles[i].y = 550.0f;
        particles[i].vx = (std::rand() % 100 / 50.0f) - 1.0f;
        particles[i].vy = -(std::rand() % 100 / 25.0f + 2.0f);
        particles[i].max_lifetime = std::rand() % 90 + 60;
        particles[i].lifetime = particles[i].max_lifetime;
        particles[i].color = sf::Color(255, std::rand() % 100 + 100, 0);
    }

    sf::RenderWindow window(sf::VideoMode(800, 600), "Fire Particle System - OpenMP");
    
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")) {
        font.loadFromFile("/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf");
    }

    sf::Text fpsText;
    fpsText.setFont(font);
    fpsText.setCharacterSize(24);
    fpsText.setFillColor(sf::Color::Blue);
    fpsText.setPosition(15.0f, 15.0f);

    sf::Text startText;
    startText.setFont(font);
    startText.setString("CLICK TO START");
    startText.setCharacterSize(30);
    startText.setFillColor(sf::Color::Cyan);
    sf::FloatRect textBounds = startText.getLocalBounds();
    startText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    startText.setPosition(400.0f, 300.0f);

    bool isStarted = false;
    sf::Clock clock;
    float smoothedFps = 60.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            
            if (!isStarted && event.type == sf::Event::MouseButtonPressed) {
                isStarted = true;
                clock.restart();
            }
        }

        float currentTime = clock.restart().asSeconds();
        float currentFps = 1.0f / (currentTime > 0 ? currentTime : 0.001f);
        smoothedFps = smoothedFps + 0.05f * (currentFps - smoothedFps);
        
        fpsText.setString("FPS: " + std::to_string(static_cast<int>(smoothedFps)) + " | Partikel: " + std::to_string(NUM_PARTICLES));

        if (isStarted) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);

            #pragma omp parallel
            {
                unsigned int seed = std::time(nullptr) ^ omp_get_thread_num();

                #pragma omp for
                for (int i = 0; i < NUM_PARTICLES; i++) {
                    
                    // 1. EFEK DORONGAN KE KIRI/KANAN BIAR APINYA NGGAK KAKU
                    float turbulence = ((rand_r(&seed) % 100) / 100.0f) - 0.5f;
                    particles[i].vx += turbulence * 0.15f;

                    // 2. INTERAKSI HEMBUSAN ANGIN MOUSE
                    if (mousePressed) {
                        float dx = particles[i].x - mousePos.x;
                        float dy = particles[i].y - mousePos.y;
                        float distance = std::sqrt(dx * dx + dy * dy) + 0.1f;
                        
                        if (distance < 250.0f) {
                            float force = (250.0f - distance) / 250.0f;
                            particles[i].vx += (dx / distance) * force * 0.8f; 
                            particles[i].vy -= force * 0.4f; 
                        }
                    }

                    // Update Posisi
                    particles[i].x += particles[i].vx;
                    particles[i].y += particles[i].vy;

                    particles[i].vx *= 0.95f; 

                    particles[i].lifetime--;

                    // Gradasi warna api
                    float lifeRatio = static_cast<float>(particles[i].lifetime) / particles[i].max_lifetime;
                    if (lifeRatio > 0.5f) {
                        particles[i].color.g = static_cast<sf::Uint8>(100 + 155 * ((lifeRatio - 0.5f) * 2.0f));
                        particles[i].color.r = 255;
                    } else {
                        particles[i].color.g = static_cast<sf::Uint8>(100 * (lifeRatio * 2.0f));
                        particles[i].color.r = static_cast<sf::Uint8>(155 + 100 * (lifeRatio * 2.0f));
                    }

                    // Respawn Partikel
                    if (particles[i].lifetime <= 0 || particles[i].x < 0 || particles[i].x > 800 || particles[i].y < 0) {
                        // Area spawn dipersempit sedikit agar sumbu api lebih fokus di tengah sebelum meliuk
                        particles[i].x = 400.0f + (rand_r(&seed) % 20 - 10);
                        particles[i].y = 550.0f;
                        particles[i].vx = (rand_r(&seed) % 100 / 100.0f) - 0.5f; // Kecepatan horizontal awal lebih rapat
                        particles[i].vy = -(rand_r(&seed) % 100 / 25.0f + 2.5f);
                        particles[i].max_lifetime = rand_r(&seed) % 90 + 60;
                        particles[i].lifetime = particles[i].max_lifetime;
                        particles[i].color = sf::Color(255, rand_r(&seed) % 100 + 100, 0);
                    }
                }
            }
        }

        // RENDERING
        window.clear(sf::Color(5, 5, 10)); 
        
        if (isStarted) {
            for (int i = 0; i < NUM_PARTICLES; i++) {
                float lifeRatio = static_cast<float>(particles[i].lifetime) / particles[i].max_lifetime;
                
                // Ukuran partikel mengecil secara smooth di ujung atas lidah api
                sf::CircleShape shape(2.0f + 6.0f * lifeRatio); 
                
                shape.setPosition(particles[i].x, particles[i].y);
                shape.setFillColor(particles[i].color);
                window.draw(shape);
            }
            window.draw(fpsText);
        } else {
            window.draw(startText);
        }

        window.display();
    }
    return 0;
}

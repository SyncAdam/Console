#ifndef AGAR_H
#define AGAR_H

#include "GinaESP.h"
#include <Arduino.h>

#define MAX_PLAYERS 1
#define MAX_FOOD 50
#define MAX_ENEMIES 5
#define ENEMY_FREQUENCY 5000  // Fréquence d'apparition des ennemis en millisecondes
#define MAX_PLAYER_SIZE 30    // Taille maximale pour rester bien normal

struct Player {
    float x, y;     // Position du joueur
    float radius;   // Rayon du joueur
    uint16_t color; // Couleur du joueur
    float speed;    // Vitesse de déplacement du joueur
};

struct Food {
    float x, y;       // Position de la nourriture
    uint16_t color;   // Couleur de la nourriture
    float radius;     // Rayon de la nourriture
    bool isConsumed;  // État de consommation de la nourriture
};

struct Enemy {
    float x, y;        // Position de l'ennemi
    uint16_t color;    // Couleur de l'ennemi
    float radius;      // Rayon de l'ennemi
    float vx, vy;      // Vitesse de déplacement en X et Y
    bool active;       // Si l'ennemi est actif
};

Player player;
Food foods[MAX_FOOD];
Enemy enemies[MAX_ENEMIES];
unsigned long lastEnemyTime = 0;  // Dernier moment où un ennemi a été introduit
unsigned long gameStartTime = 0;  // Heure de début de jeu
int score = 0;  // Score du joueur

void initGame(TFT_eSPI &screen);
void introduceEnemy();
void updateGame(TFT_eSPI &screen, bool withEnemies);
void playAgar(TFT_eSPI &screen);
void playAgarWithEnemies(TFT_eSPI &screen);
void endGame(TFT_eSPI &screen, bool withEnemies, bool win);
void returnToMainMenu();

void initGame(TFT_eSPI &screen) {
    GinaESP::clearBuffer();
    player.x = WIDTH / 2;
    player.y = HEIGHT / 2;
    player.radius = 10;
    player.color = TFT_WHITE;
    player.speed = 4;  // Vitesse élevée pour esquiver facilement

    // Initialisation des foods
    for (int i = 0; i < MAX_FOOD; i++) {
        foods[i].x = random(WIDTH);
        foods[i].y = random(HEIGHT);
        foods[i].radius = random(1, 4);
        foods[i].color = GinaESP::colors[random(24)];
        foods[i].isConsumed = false;
    }

    // Initialisation des ennemis
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }

    gameStartTime = millis();  // Début du comptage du temps
    score = 0;  // Réinitialisation du score

    GinaESP::drawScreen(screen);
}

void introduceEnemy() {
    if (millis() - lastEnemyTime > ENEMY_FREQUENCY) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) {
                int edge = random(4);  // Choisir un bord pour l'apparition de l'ennemi
                switch (edge) {
                    case 0: enemies[i].x = random(WIDTH); enemies[i].y = -10; break;
                    case 1: enemies[i].x = WIDTH + 10; enemies[i].y = random(HEIGHT); break;
                    case 2: enemies[i].x = random(WIDTH); enemies[i].y = HEIGHT + 10; break;
                    case 3: enemies[i].x = -10; enemies[i].y = random(HEIGHT); break;
                }
                enemies[i].radius = random(10, 20);  // Taille de l'ennemi (plus grande pour être plus visible)
                enemies[i].color = TFT_RED;
                enemies[i].vx = (random(200) / 100.0 - 1);  // Vitesse initiale aléatoire
                enemies[i].vy = (random(200) / 100.0 - 1);
                enemies[i].active = true;
                lastEnemyTime = millis();
                break;
            }
        }
    }
}

void updateGame(TFT_eSPI &screen, bool withEnemies) {
    GinaESP::clearBuffer();

    // Mise à jour de la position du joueur
    int joyX = analogRead(JOYX);
    int joyY = analogRead(JOYY);
    float ax = map(joyX, 0, 4095, -player.speed, player.speed);
    float ay = map(joyY, 0, 4095, -player.speed, player.speed);
    player.x += ax;
    player.y += ay;
    player.x = constrain(player.x, player.radius, WIDTH - player.radius);
    player.y = constrain(player.y, player.radius, HEIGHT - player.radius);

    // Dessin du joueur
    GinaESP::fillCircle(screen, player.x, player.y, player.radius, player.color);

    // Mise à jour et dessin de la nourriture
    bool allFoodsConsumed = true;
    for (int i = 0; i < MAX_FOOD; i++) {
        if (!foods[i].isConsumed) {
            float dx = player.x - foods[i].x;
            float dy = player.y - foods[i].y;
            float distance = sqrt(dx * dx + dy * dy);
            if (distance < player.radius + foods[i].radius) {
                player.radius += 0.5 * foods[i].radius; // Augmenter la taille du joueur
                player.radius = min(player.radius, static_cast<float>(MAX_PLAYER_SIZE));// Limite maximale de la taille du joueur
                foods[i].isConsumed = true;
                foods[i].x = -100; // Déplacer hors de l'écran
                foods[i].y = -100;
                score++; // Augmenter le score
            } else {
                GinaESP::fillCircle(screen, foods[i].x, foods[i].y, foods[i].radius, foods[i].color);
                allFoodsConsumed = false;
            }
        }
    }

    // Mise à jour et dessin des ennemis si activé
    if (withEnemies) {
        introduceEnemy();
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                // Mouvement aléatoire des ennemis
                enemies[i].vx += (random(200) / 100.0 - 1) * 0.1;
                enemies[i].vy += (random(200) / 100.0 - 1) * 0.1;
                enemies[i].x += enemies[i].vx;
                enemies[i].y += enemies[i].vy;

                // Garder les ennemis à l'intérieur de l'écran
                if (enemies[i].x < 0) {
                    enemies[i].x = 0;
                    enemies[i].vx = abs(enemies[i].vx);
                } else if (enemies[i].x > WIDTH) {
                    enemies[i].x = WIDTH;
                    enemies[i].vx = -abs(enemies[i].vx);
                }
                if (enemies[i].y < 0) {
                    enemies[i].y = 0;
                    enemies[i].vy = abs(enemies[i].vy);
                } else if (enemies[i].y > HEIGHT) {
                    enemies[i].y = HEIGHT;
                    enemies[i].vy = -abs(enemies[i].vy);
                }

                float dist = sqrt(pow(player.x - enemies[i].x, 2) + pow(player.y - enemies[i].y, 2));
                if (dist < player.radius + enemies[i].radius) {
                    endGame(screen, withEnemies, false);
                    return;
                }
                GinaESP::drawStar(screen, enemies[i].x, enemies[i].y, enemies[i].radius, enemies[i].color);
            }
        }
    }

    GinaESP::drawScreen(screen);

    // Afficher le score et le temps
    screen.setTextColor(TFT_WHITE, TFT_BLACK);
    screen.setCursor(0, HEIGHT - 20);
    screen.print("Score: ");
    screen.print(score);
    screen.print(" Time: ");
    screen.print((millis() - gameStartTime) / 1000);
    screen.println("s");

    // Vérifier si tous les aliments sont consommés
    if (allFoodsConsumed) {
        endGame(screen, withEnemies, true);
    }
}

void playAgar(TFT_eSPI &screen) {
    initGame(screen);
    while (true) {
        if (digitalRead(BUTTON2)) {
            returnToMainMenu();
            return;
        }
        updateGame(screen, false);
        delay(30);
    }
}

void playAgarWithEnemies(TFT_eSPI &screen) {
    initGame(screen);
    while (true) {
        if (digitalRead(BUTTON2)) {
            returnToMainMenu();
            return;
        }
        updateGame(screen, true);
        delay(30);
    }
}

void endGame(TFT_eSPI &screen, bool withEnemies, bool win) {
    if (win) {
        screen.fillScreen(TFT_GREEN);
        screen.setCursor(10, 10);
        screen.setTextColor(TFT_WHITE);
        screen.setTextSize(2);
        screen.println("You Win!");
    } else {
        screen.fillScreen(TFT_RED);
        screen.setCursor(10, 10);
        screen.setTextColor(TFT_WHITE);
        screen.setTextSize(2);
        screen.println("Game Over!");
    }

    screen.setCursor(10, 40);
    screen.println("Next Level: Button1");
    screen.setCursor(10, 60);
    screen.println("Main Menu: Button2");

    while (true) {
        bool button1 = digitalRead(BUTTON1);
        bool button2 = digitalRead(BUTTON2);
        if (button1) {
            delay(200);  // Debounce delay
            button1 = digitalRead(BUTTON1);
            if (button1) {
                if (withEnemies) {
                    playAgarWithEnemies(screen);  // Start next level
                } else {
                    playAgar(screen);  // Retry current level
                }
            }
        } else if (button2) {
            delay(200);  // Debounce delay
            button2 = digitalRead(BUTTON2);
            if (button2) {
                returnToMainMenu();
            }
        }
    }
}


void returnToMainMenu() {
    setup();
    loop();
}

#endif // AGAR_H

#ifndef AGAR_H
#define AGAR_H

#include "GinaESP.h"
#include <Arduino.h>

#define MAX_PLAYERS 1
#define MAX_FOOD 50
#define MAX_ATTACKERS 5
#define ATTACKER_FREQUENCY 20000  // Fréquence attaquants en millisecondes

enum Shape { CIRCLE, SQUARE, TRIANGLE };  //formes pour les attaquants

struct Player {
    float x, y;     
    float radius;   
    uint16_t color; 
    float speed;    
};

struct Food {
    float x, y;       
    uint16_t color;   
    float radius;     
    bool isConsumed;  
};

struct Attacker {
    float x, y;        
    uint16_t color;    
    float size;        
    float speedX, speedY; 
    bool isActive;     
    Shape shape;      
};

Player player;
Food foods[MAX_FOOD];
Attacker attackers[MAX_ATTACKERS];
unsigned long lastAttackerTime = 0;  // Dernier moment où un attaquant a été introduit

void initGame(TFT_eSPI &screen) {
    GinaESP::clearBuffer();
    player.x = WIDTH / 2;
    player.y = HEIGHT / 2;
    player.radius = 10;
    player.color = TFT_WHITE;
    player.speed = 4;  // Vitesse pour esquiver 

    // 
    for (int i = 0; i < MAX_FOOD; i++) {
        foods[i].x = random(WIDTH);
        foods[i].y = random(HEIGHT);
        foods[i].radius = random(1, 4);
        foods[i].color = GinaESP::colors[random(24)];
        foods[i].isConsumed = false;
    }

    // 
    for (int i = 0; i < MAX_ATTACKERS; i++) {
        attackers[i].isActive = false;
    }

    GinaESP::drawScreen(screen);
}

void introduceAttacker() {
    int idx = random(MAX_ATTACKERS);
    if (!attackers[idx].isActive) {
        int edge = random(4);  
        switch (edge) {
            case 0: attackers[idx].x = random(WIDTH); attackers[idx].y = -10; break;
            case 1: attackers[idx].x = WIDTH + 10; attackers[idx].y = random(HEIGHT); break;
            case 2: attackers[idx].x = random(WIDTH); attackers[idx].y = HEIGHT + 10; break;
            case 3: attackers[idx].x = -10; attackers[idx].y = random(HEIGHT); break;
        }
        attackers[idx].size = random(10, 20);  
        attackers[idx].color = TFT_RED;
        float angle = atan2(player.y - attackers[idx].y, player.x - attackers[idx].x);
        attackers[idx].speedX = 0.8 * cos(angle); // Vitesse modérée pour  l'esquive
        attackers[idx].speedY = 0.8 * sin(angle);
        attackers[idx].isActive = true;
        attackers[idx].shape = static_cast<Shape>(random(0, 3));  // Forme aléatoire
        lastAttackerTime = millis();
    }
}

void drawAttacker(TFT_eSPI &screen, const Attacker &attacker) {
    switch (attacker.shape) {
        case CIRCLE:
            GinaESP::fillCircle(screen, attacker.x, attacker.y, attacker.size, attacker.color);
            break;
        case SQUARE:
            GinaESP::fillRect(screen, attacker.x - attacker.size, attacker.y - attacker.size,
                              attacker.x + attacker.size, attacker.y + attacker.size, attacker.color);
            break;
        case TRIANGLE:
            //
            break;
    }
}

void updateGameLogic(TFT_eSPI &screen) {
    unsigned long currentTime = millis();
    if (currentTime - lastAttackerTime > ATTACKER_FREQUENCY) {
        introduceAttacker();
    }

    int joyX = analogRead(JOYX);
    int joyY = analogRead(JOYY);
    float ax = map(joyX, 0, 4095, -player.speed, player.speed);
    float ay = map(joyY, 0, 4095, -player.speed, player.speed);

    player.x += ax;
    player.y += ay;
    player.x = constrain(player.x, player.radius, WIDTH - player.radius);
    player.y = constrain(player.y, player.radius, HEIGHT - player.radius);

    GinaESP::clearBuffer();
    for (int i = 0; i < MAX_FOOD; i++) {
        if (!foods[i].isConsumed) {
            float dx = player.x - foods[i].x;
            float dy = player.y - foods[i].y;
            float distance = sqrt(dx * dx + dy * dy);
            if (distance < player.radius + foods[i].radius) {
                player.radius += foods[i].radius / 2;
                foods[i].isConsumed = true;
                foods[i].x = -100; // Move out of screen
                foods[i].y = -100;
            } else {
                GinaESP::fillCircle(screen, foods[i].x, foods[i].y, foods[i].radius, foods[i].color);
            }
        }
    }
    for (int i = 0; i < MAX_ATTACKERS; i++) {
        if (attackers[i].isActive) {
            attackers[i].x += attackers[i].speedX;
            attackers[i].y += attackers[i].speedY;
            if (sqrt(pow(player.x - attackers[i].x, 2) + pow(player.y - attackers[i].y, 2)) < player.radius + attackers[i].size) {
                screen.fillScreen(TFT_BLACK);
                screen.setCursor(10, 10);
                screen.setTextColor(TFT_RED);
                screen.setTextSize(2);
                screen.println("Game Over!");
                delay(5000);
                return;  // Stop the game if hit by an attacker
            }
            if (attackers[i].x < -20 || attackers[i].x > WIDTH + 20 || attackers[i].y < -20 || attackers[i].y > HEIGHT + 20) {
                attackers[i].isActive = false;  // Deactivate if out of screen
            } else {
                drawAttacker(screen, attackers[i]);
            }
        }
    }
    GinaESP::fillCircle(screen, player.x, player.y, player.radius, player.color);
    GinaESP::drawScreen(screen);
}

void playAgar(TFT_eSPI &screen) {
    initGame(screen);
    while (true) {
        updateGameLogic(screen);
        delay(30);  // Lower delay for more frequent updates
    }
}

#endif // AGAR_H

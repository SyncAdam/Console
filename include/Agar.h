#ifndef AGAR_H
#define AGAR_H

#include "GinaESP.h"

#define MAX_PLAYERS 1
#define MAX_FOOD 50

struct Player {
    float x, y;
    float radius;
    uint16_t color;
};

struct Food {
    float x, y;
    uint16_t color;
};

Player players[MAX_PLAYERS];
Food foods[MAX_FOOD];

void initGame(TFT_eSPI &screen) {
    GinaESP::clearBuffer();
    players[0].x = WIDTH / 2;
    players[0].y = HEIGHT / 2;
    players[0].radius = 10;
    players[0].color = GinaESP::colors[random(24)];

    for (int i = 0; i < MAX_FOOD; i++) {
        foods[i].x = random(WIDTH);
        foods[i].y = random(HEIGHT);
        foods[i].color = GinaESP::colors[random(24)];
    }
    GinaESP::drawScreen(screen);
}

void updateGameLogic(TFT_eSPI &screen) {
    int joyX = analogRead(JOYX);
    int joyY = analogRead(JOYY);

    float ax = map(joyX, 0, 4095, -1, 1);
    float ay = map(joyY, 0, 4095, -1, 1);

    players[0].x += ax;
    players[0].y += ay;

    players[0].x = constrain(players[0].x, players[0].radius, WIDTH - players[0].radius);
    players[0].y = constrain(players[0].y, players[0].radius, HEIGHT - players[0].radius);

    GinaESP::clearBuffer();
    for (int i = 0; i < MAX_FOOD; i++) {
        float dx = players[0].x - foods[i].x;
        float dy = players[0].y - foods[i].y;
        float distance = sqrt(dx*dx + dy*dy);
        if (distance < (players[0].radius + 2)) { // petit buffer pour la détect collision
            players[0].radius += 0.5;
            foods[i].x = random(WIDTH); // food new position
            foods[i].y = random(HEIGHT);
        }
        GinaESP::fillCircle(screen, foods[i].x, foods[i].y, 2, foods[i].color);
    }
    GinaESP::fillCircle(screen, players[0].x, players[0].y, players[0].radius, players[0].color);
    GinaESP::drawScreen(screen);
}

void playAgar(TFT_eSPI &screen) {
    initGame(screen);
    while (true) {
        updateGameLogic(screen);
        delay(100);
    }
}

#endif // AGAR_H

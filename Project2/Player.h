#pragma once
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>

#include "Position.h"

class Player {
public:
    Player(const std::string& name, const Position& startPosition, int totalWalls = 10);

    Position previewMove(char direction) const;
    void move(char direction, int steps = 1);
    void showStatus() const;
    bool isDead() const;

    const std::string& getName() const;
    Position getPosition() const;
    int getWallsRemaining() const;
    bool hasWallsRemaining() const;

    bool placeWall();
    void eliminate();

private:
    static constexpr int kBoardSize = 8;

    std::string name_;
    Position position_;
    int wallsRemaining_;
    bool eliminated_;

    bool isWithinBoard(int row, int col) const;
};

#endif  // PLAYER_HPP

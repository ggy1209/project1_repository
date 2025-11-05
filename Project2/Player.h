#pragma once
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>

#include "Position.h"

class Player {
public:
    Player(const std::string& name, const Position& startPosition, int totalWalls = 10);

    Position previewMove(int direction) const;
    void move(int direction);
    void showStatus() const;
    bool isDead() const;

    const std::string& getName() const;
    Position getPosition() const;
    int getWallsRemaining() const;
    bool hasWallsRemaining() const;

    bool placeWall();
    void eliminate();

private:
    static constexpr int kBoardSize = 9;

    std::string name_;
    Position position_;
    int wallsRemaining_;
    bool eliminated_;

    bool isWithinBoard(int row, int col) const;
};

#endif  // PLAYER_HPP

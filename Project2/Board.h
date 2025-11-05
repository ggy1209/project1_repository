#pragma once
#ifndef BOARD_HPP
#define BOARD_HPP

#include <vector>

#include "Position.h"

class Player;

class Board {
public:
    static constexpr int kSize = 9;

    Board();

    void reset();
    void drawBoard(const std::vector<Player>& players) const;

    bool isWithinBounds(const Position& position) const;
    bool placeWall(const Position& position, bool horizontal);
    bool hasWall(const Position& position, bool horizontal) const;
    bool isMoveBlocked(const Position& from, const Position& to) const;

private:
    struct WallPlacement {
        Position position;
        bool horizontal;
    };

    int countHorizontalInRow(int row) const;
    int countVerticalInCol(int col) const;
    bool overlapsExistingWall(const Position& position, bool horizontal) const;

    std::vector<WallPlacement> walls_;
};

#endif  // BOARD_HPP

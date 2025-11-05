#include "Board.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "Player.h"

Board::Board() = default;

void Board::reset() {
    walls_.clear();
}

void Board::drawBoard(const std::vector<Player>& players) const {
    const int N = kSize;
    const int gridSize = 2 * N - 1;
    const std::string kCellSymbol = u8"□";
    const std::string kWallSymbol = u8"■";

    std::vector<std::vector<std::string>> screen(
        gridSize, std::vector<std::string>(gridSize, " "));

    // Fill base cells.
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            screen[2 * row][2 * col] = kCellSymbol;
        }
    }

    // Place players on their cells.
    for (std::size_t index = 0; index < players.size(); ++index) {
        if (players[index].isDead()) {
            continue;
        }
        Position position = players[index].getPosition();
        if (!isWithinBounds(position)) {
            continue;
        }

        screen[2 * position.row][2 * position.col] =
            std::string(1, static_cast<char>('1' + static_cast<int>(index)));
    }

    // Mark stored walls.
    auto markWall = [&](int row, int col) {
        if (row >= 0 && row < gridSize && col >= 0 && col < gridSize) {
            screen[row][col] = kWallSymbol;
        }
    };

    for (const auto& wall : walls_) {
        if (wall.horizontal) {
            int rowIndex = 2 * wall.position.row;
            int colIndex = 2 * wall.position.col + 1;
            markWall(rowIndex, colIndex);
            markWall(rowIndex, colIndex - 2);
            markWall(rowIndex, colIndex + 2);
        } else {
            int rowIndex = 2 * wall.position.row + 1;
            int colIndex = 2 * wall.position.col;
            markWall(rowIndex, colIndex);
            markWall(rowIndex - 2, colIndex);
            markWall(rowIndex + 2, colIndex);
        }
    }

    // Insert column letters in vertical slots between cells.
    for (int col = 1; col < gridSize; col += 2) {
        char label = static_cast<char>('A' + (col - 1) / 2);
        for (int row = 0; row < gridSize; row += 2) {
            if (screen[row][col] == " ") {
                screen[row][col] = std::string(1, label);
            }
        }
    }

    // Insert row numbers in horizontal slots between cells.
    for (int row = 1; row < gridSize; row += 2) {
        std::string label = std::to_string((row + 1) / 2);
        for (int col = 0; col < gridSize; col += 2) {
            if (screen[row][col] == " ") {
                screen[row][col] = label;
            }
        }
    }

    // Print the composed screen.
    for (int row = 0; row < gridSize; ++row) {
        for (int col = 0; col < gridSize; ++col) {
            std::cout << screen[row][col];
            if (col < gridSize - 1) {
                std::cout << ' ';
            }
        }
        std::cout << '\n';
    }
}

bool Board::isWithinBounds(const Position& position) const {
    return position.row >= 0 && position.row < kSize &&
           position.col >= 0 && position.col < kSize;
}

int Board::countHorizontalInRow(int row) const {
    return std::count_if(walls_.begin(), walls_.end(),
                         [&](const WallPlacement& wall) {
                             return wall.horizontal && wall.position.row == row;
                         });
}

int Board::countVerticalInCol(int col) const {
    return std::count_if(walls_.begin(), walls_.end(),
                         [&](const WallPlacement& wall) {
                             return !wall.horizontal && wall.position.col == col;
                         });
}

bool Board::placeWall(const Position& position, bool horizontal) {
    if (!isWithinBounds(position)) {
        return false;
    }

    if (position.row >= kSize - 1 || position.col >= kSize - 1) {
        return false;
    }

    if (hasWall(position, horizontal)) {
        return false;
    }

    if (horizontal) {
        if (countHorizontalInRow(position.row) >= 3) {
            return false;
        }

        Position left{position.row, position.col - 1};
        Position right{position.row, position.col + 1};
        if (position.col - 1 >= 0 && hasWall(left, true)) {
            return false;
        }
        if (position.col + 1 < kSize - 1 && hasWall(right, true)) {
            return false;
        }
    } else {
        if (countVerticalInCol(position.col) >= 3) {
            return false;
        }

        Position up{position.row - 1, position.col};
        Position down{position.row + 1, position.col};
        if (position.row - 1 >= 0 && hasWall(up, false)) {
            return false;
        }
        if (position.row + 1 < kSize - 1 && hasWall(down, false)) {
            return false;
        }
    }

    walls_.push_back({position, horizontal});
    return true;
}

bool Board::hasWall(const Position& position, bool horizontal) const {
    return std::any_of(walls_.begin(), walls_.end(),
                       [&](const WallPlacement& wall) {
                           return wall.horizontal == horizontal &&
                                  wall.position.row == position.row &&
                                  wall.position.col == position.col;
                       });
}

bool Board::isMoveBlocked(const Position& from, const Position& to) const {
    int rowDelta = to.row - from.row;
    int colDelta = to.col - from.col;

    if (rowDelta == 1 && colDelta == 0) {
        return hasWall({from.row, from.col}, true);
    }
    if (rowDelta == -1 && colDelta == 0) {
        return hasWall({to.row, to.col}, true);
    }
    if (rowDelta == 0 && colDelta == 1) {
        return hasWall({from.row, from.col}, false);
    }
    if (rowDelta == 0 && colDelta == -1) {
        return hasWall({to.row, to.col}, false);
    }

    return false;
}

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
    const int gridRows = 2 * N + 2;   // 18 rows for N == 8
    const int gridCols = 4 * N + 2;   // 34 columns for N == 8
    const std::string kCellSymbol = u8"□";
    const std::string kWallSymbol = u8"■";

    std::vector<std::vector<std::string>> screen(
        gridRows, std::vector<std::string>(gridCols, " "));

    // Row labels on even-numbered rows (1-based): rows 2,4,...,16.
    for (int row = 1, label = 1; row < gridRows && label <= N; row += 2, ++label) {
        screen[row][0] = std::to_string(label);
    }

    // Column labels on multiples of 3 (1-based) in the first row.
    for (int col = 0, label = 0; col < gridCols && label < N; ++col) {
        if ((col + 1) % 3 == 0) {
            screen[0][col] = std::string(1, static_cast<char>('A' + label));
            ++label;
        }
    }

    // Fill base cells.
    for (int row = 0; row < N; ++row) {
        for (int col = 0; col < N; ++col) {
            int rowIndex = 2 * row + 1;
            int colIndex = 3 * col + 2;
            if (rowIndex < gridRows && colIndex < gridCols) {
                screen[rowIndex][colIndex] = kCellSymbol;
            }
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

        int rowIndex = 2 * position.row + 1;
        int colIndex = 3 * position.col + 2;
        if (rowIndex < gridRows && colIndex < gridCols) {
            screen[rowIndex][colIndex] =
                std::string(1, static_cast<char>('1' + static_cast<int>(index)));
        }
    }

    // Mark stored walls.
    auto markWall = [&](int row, int col) {
        if (row >= 0 && row < gridRows && col >= 0 && col < gridCols) {
            screen[row][col] = kWallSymbol;
        }
    };

    for (const auto& wall : walls_) {
        if (wall.horizontal) {
            int rowIndex = 2 * wall.position.row + 2;
            int colIndex = 3 * wall.position.col + 2;
            for (int delta = 0; delta < 3; ++delta) {
                markWall(rowIndex, colIndex + delta);
            }
        } else {
            int rowIndex = 2 * wall.position.row + 1;
            int colIndex = 3 * wall.position.col + 3;
            for (int delta = 0; delta < 3; ++delta) {
                markWall(rowIndex + delta, colIndex);
            }
        }
    }

    // Print the composed board.
    for (int row = 0; row < gridRows; ++row) {
        for (int col = 0; col < gridCols; ++col) {
            std::cout << screen[row][col];
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

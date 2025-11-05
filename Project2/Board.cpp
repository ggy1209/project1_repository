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
    const int N = kSize;                     // 예: 9
    const int rows = 2 * N;                  // 0행 알파벳 + (셀/숫자) * (N-1) + 마지막 셀
    const int cols = 2 + N + (N - 1) * 3;    // "  " + N칸 + (N-1)*3

    // 화면 버퍼
    std::vector<std::vector<std::string>> screen(
        rows, std::vector<std::string>(cols, " ")
    );

    // 1) 맨 위 알파벳 줄
    // N=9면 gap은 0..7 → A..H까지만 찍힘
    for (int g = 0; g < N - 1; ++g) {
        int col = 4 + 4 * g; // 네모 사이 가운데
        if (col < cols) {
            screen[0][col] = std::string(1, static_cast<char>('A' + g)); // A~H
        }
    }

    // 2) 기본 보드 (셀행 + 숫자행)
    for (int r = 0; r < N; ++r) {
        // 셀 있는 행: 1,3,5,... = 1 + 2*r
        int cellRow = 1 + 2 * r;

        // 앞 두 칸
        screen[cellRow][0] = " ";
        screen[cellRow][1] = " ";

        // 셀 N개 찍기
        for (int c = 0; c < N; ++c) {
            int cellCol = 2 + 4 * c;  // 셀 c 위치
            if (cellCol < cols) {
                screen[cellRow][cellCol] = u8"□";
            }
        }

        // 숫자행은 위에서 말한대로 1~(N-1)까지만
        if (r < N - 1) {
            int numRow = cellRow + 1;
            screen[numRow][0] = std::to_string(r + 1); // 1,2,...,8
        }
    }

    // 3) 플레이어 덮어쓰기
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i].isDead()) continue;
        Position p = players[i].getPosition();
        if (!isWithinBounds(p)) continue;

        int cellRow = 1 + 2 * p.row;
        int cellCol = 2 + 4 * p.col;

        if (cellRow >= 0 && cellRow < rows &&
            cellCol >= 0 && cellCol < cols) {
            screen[cellRow][cellCol] =
                std::string(1, static_cast<char>('1' + static_cast<int>(i)));
        }
    }

    // 4) 출력
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            std::cout << screen[r][c];
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

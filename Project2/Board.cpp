#include "Board.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "Player.h"

namespace {
std::vector<std::pair<int, int>> wallPixels(const Position& position, bool horizontal) {
    std::vector<std::pair<int, int>> pixels;

    const int centerRow = 2 + 2 * position.row;
    const int centerCol = 4 + 4 * position.col;

    pixels.emplace_back(centerRow, centerCol);
    if (horizontal) {
        pixels.emplace_back(centerRow, centerCol - 2);
        pixels.emplace_back(centerRow, centerCol + 2);
    } else {
        pixels.emplace_back(centerRow - 1, centerCol);
        pixels.emplace_back(centerRow + 1, centerCol);
    }

    return pixels;
}
}  // namespace

Board::Board() = default;

void Board::reset() {
    walls_.clear();
}

void Board::drawBoard(const std::vector<Player>& players) const {
    const int N = kSize;                     // 예: 9
    const int rows = 2 * N;                  // 0행 알파벳 + (셀/숫자)*반복
    const int cols = 2 + N + (N - 1) * 3;    // "  " + N칸 + (N-1)*3

    // 화면 버퍼 (전부 공백으로 초기화)
    std::vector<std::vector<std::string>> screen(
        rows, std::vector<std::string>(cols, " ")
    );

    // 1) 맨 위 알파벳 줄 (A ~ H)
    for (int g = 0; g < N - 1; ++g) {
        int col = 4 + 4 * g; // 네모 사이 가운데 위치
        if (col < cols) {
            screen[0][col] = std::string(1, static_cast<char>('A' + g)); // A~H
        }
    }

    // 2) 기본 보드 (셀행 + 숫자행)
    for (int r = 0; r < N; ++r) {
        int cellRow = 1 + 2 * r;   // 셀행 (1,3,5,...)

        // 셀행 앞 두 칸은 공백
        screen[cellRow][0] = " ";
        screen[cellRow][1] = " ";

        // 셀 N개 출력
        for (int c = 0; c < N; ++c) {
            int cellCol = 2 + 4 * c; // 각 셀의 열 위치
            if (cellCol < cols) {
                screen[cellRow][cellCol] = u8"□";
            }
        }

        // 숫자행 (1~N-1까지만)
        if (r < N - 1) {
            int numRow = cellRow + 1;
            screen[numRow][0] = std::to_string(r + 1); // 1,2,...,8
        }
    }

    // 3) 플레이어 표시
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i].isDead()) continue;
        Position p = players[i].getPosition();
        if (!isWithinBounds(p)) continue;

        int cellRow = 1 + 2 * p.row;  // 셀행
        int cellCol = 2 + 4 * p.col;  // 셀열

        if (cellRow >= 0 && cellRow < rows &&
            cellCol >= 0 && cellCol < cols) {
            screen[cellRow][cellCol] =
                std::string(1, static_cast<char>('1' + static_cast<int>(i)));
        }
    }

    // 4) 벽 표시 (■)
    for (const auto& wall : walls_) {
        // 화면상 중심 좌표 계산
        int centerRow = 2 + 2 * wall.position.row;  // 숫자 있는 줄 (2,4,6,...)
        int centerCol = 4 + 4 * wall.position.col;  // 알파벳 있는 열 (4,8,12,...)

        if (centerRow < 0 || centerRow >= rows ||
            centerCol < 0 || centerCol >= cols)
            continue;

        // 중심 네모
        screen[centerRow][centerCol] = u8"■";

        if (wall.horizontal) {
            // 수평(h): 같은 행에서 양옆 셀 열에 찍어야 함
            // 가운데(4+4c) 기준으로 ±2 하면 셀 열(2+4c, 6+4c)이 됨
            int leftCol  = centerCol - 2;
            int rightCol = centerCol + 2;
            if (leftCol >= 0)    screen[centerRow][leftCol] = u8"■";
            if (rightCol < cols) screen[centerRow][rightCol] = u8"■";
        } else {
            // 수직(v): 같은 열에서 위/아래 한 줄씩 (셀 줄)
            // 가운데 숫자줄(2+2r) 기준으로 ±1 하면 위아래 셀줄(1+2r, 3+2r)
            int upRow   = centerRow - 1;
            int downRow = centerRow + 1;
            if (upRow >= 0)    screen[upRow][centerCol] = u8"■";
            if (downRow < rows) screen[downRow][centerCol] = u8"■";
        }
    }

    // 5) 화면 출력
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

bool Board::overlapsExistingWall(const Position& position, bool horizontal) const {
    const auto newPixels = wallPixels(position, horizontal);

    for (const auto& wall : walls_) {
        const auto existingPixels = wallPixels(wall.position, wall.horizontal);
        for (const auto& pixel : newPixels) {
            if (std::find(existingPixels.begin(), existingPixels.end(), pixel) != existingPixels.end()) {
                return true;
            }
        }
    }

    return false;
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

    if (overlapsExistingWall(position, horizontal)) {
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

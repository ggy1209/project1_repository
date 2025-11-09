#include "Board.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "Player.h"

namespace {
inline Position makePos(int r, int c){
    Position p;
    p.row=r;
    p.col=c;
    return p;
}
constexpr const char* kResetColor = "\033[0m";
constexpr const char* kRedColor = "\033[31m";
constexpr const char* kGreenColor = "\033[32m";
constexpr const char* kYellowColor = "\033[33m";
constexpr const char* kBlueColor = "\033[34m";

std::string colorize(const std::string& text, const char* color) {
    return std::string(color) + text + kResetColor;
}

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

    const std::vector<std::pair<int, int>> highlightedCells = {
        {2, 2}, {2, 6}, {6, 2}, {6, 6}
    };

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
                bool highlight = std::any_of(
                    highlightedCells.begin(),
                    highlightedCells.end(),
                    [&](const std::pair<int, int>& pos) {
                        return pos.first == r && pos.second == c;
                    });
                screen[cellRow][cellCol] =
                    highlight ? colorize(u8"□", kRedColor) : u8"□";
            }
        }

        // 숫자행 (1~N-1까지만)
        if (r < N - 1) {
            int numRow = cellRow + 1;
            screen[numRow][0] = std::to_string(r + 1); // 1,2,...,8
        }
    }

    // 3) 플레이어 표시
    const std::vector<const char*> playerColors = {
        kYellowColor, kGreenColor, kRedColor, kBlueColor
    };

    for (std::size_t i = 0; i < players.size(); ++i) {
        Position p = players[i].getPosition();
        if (!isWithinBounds(p)) continue;

        int cellRow = 1 + 2 * p.row;  // 셀행
        int cellCol = 2 + 4 * p.col;  // 셀열

        if (cellRow >= 0 && cellRow < rows &&
            cellCol >= 0 && cellCol < cols) {
            std::string symbol(1, static_cast<char>('1' + static_cast<int>(i)));
            if (i < playerColors.size()) {
                screen[cellRow][cellCol] = colorize(symbol, playerColors[i]);
            } else {
                screen[cellRow][cellCol] = symbol;
            }
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
        screen[centerRow][centerCol] = colorize(u8"■", kBlueColor);

        if (wall.horizontal) {
            // 수평(h): 같은 행에서 양옆 셀 열에 찍어야 함
            // 가운데(4+4c) 기준으로 ±2 하면 셀 열(2+4c, 6+4c)이 됨
            int leftCol  = centerCol - 2;
            int rightCol = centerCol + 2;
            if (leftCol >= 0)    screen[centerRow][leftCol] = colorize(u8"■", kBlueColor);
            if (rightCol < cols) screen[centerRow][rightCol] = colorize(u8"■", kBlueColor);
        } else {
            // 수직(v): 같은 열에서 위/아래 한 줄씩 (셀 줄)
            // 가운데 숫자줄(2+2r) 기준으로 ±1 하면 위아래 셀줄(1+2r, 3+2r)
            int upRow   = centerRow - 1;
            int downRow = centerRow + 1;
            if (upRow >= 0)    screen[upRow][centerCol] = colorize(u8"■", kBlueColor);
            if (downRow < rows) screen[downRow][centerCol] = colorize(u8"■", kBlueColor);
        }
    }

    // 5) 화면 출력 (가로 방향 칸 사이에 공백 추가)
    for (int r = 0; r < rows; ++r) {
        std::string line;
        for (int c = 0; c < cols; ++c) {
            line += screen[r][c];
            if (c + 1 < cols) {
                line += ' ';
            }
        }
        std::cout << line << '\n';
    }
}




bool Board::isWithinBounds(const Position& position) const {
    return position.row >= 0 && position.row < kSize &&
           position.col >= 0 && position.col < kSize;
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
        Position left=makePos(position.row, position.col - 1);
        Position right=makePos(position.row, position.col + 1);
        if (position.col - 1 >= 0 && hasWall(left, true)) {
            return false;
        }
        if (position.col + 1 < kSize - 1 && hasWall(right, true)) {
            return false;
        }
    } else {
        Position up=makePos(position.row - 1, position.col);
        Position down=makePos(position.row + 1, position.col);
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
        if (hasWall(makePos(from.row, from.col), true)) {
            return true;
        }
        if (from.col - 1 >= 0 && hasWall(makePos(from.row, from.col-1), true)) {
            return true;
        }
        return false;
    }
    if (rowDelta == -1 && colDelta == 0) {
        if (hasWall(makePos(to.row, to.col), true)) {
            return true;
        }
        if (to.col - 1 >= 0 && hasWall(makePos(to.row, to.col - 1), true)) {
            return true;
        }
        return false;
    }
    if (rowDelta == 0 && colDelta == 1) {
        if (hasWall(makePos(from.row, from.col), false)) {
            return true;
        }
        if (from.row - 1 >= 0 && hasWall(makePos(from.row - 1, from.col), false)) {
            return true;
        }
        return false;
    }
    if (rowDelta == 0 && colDelta == -1) {
        if (hasWall(makePos(to.row, to.col), false)) {
            return true;
        }
        if (to.row - 1 >= 0 && hasWall(makePos(to.row - 1, to.col), false)) {
            return true;
        }
        return false;
    }

    return false;
}

void Board::removeWall(const Position& position, bool horizontal) {
    auto it = std::find_if(walls_.begin(), walls_.end(),
                           [&](const WallPlacement& wall) {
                               return wall.horizontal == horizontal &&
                                      wall.position.row == position.row &&
                                      wall.position.col == position.col;
                           });
    if (it != walls_.end()) {
        walls_.erase(it);
    }
}

bool Board::existsPath(const Position& start,
                       const std::function<bool(const Position&)>& isGoal) const {
    if (!isWithinBounds(start)) {
        return false;
    }

    if (isGoal(start)) {
        return true;
    }

    std::vector<std::vector<bool>> visited(
        kSize, std::vector<bool>(kSize, false));
    std::queue<Position> searchQueue;

    visited[start.row][start.col] = true;
    searchQueue.push(start);

    const int directions[4][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };

    while (!searchQueue.empty()) {
        Position current = searchQueue.front();
        searchQueue.pop();

        if (isGoal(current)) {
            return true;
        }

        for (const auto& dir : directions) {
            Position next=makePos(current.row + dir[0], current.col + dir[1]);
            if (!isWithinBounds(next)) {
                continue;
            }
            if (visited[next.row][next.col]) {
                continue;
            }
            if (isMoveBlocked(current, next)) {
                continue;
            }

            visited[next.row][next.col] = true;
            searchQueue.push(next);
        }
    }

    return false;
}

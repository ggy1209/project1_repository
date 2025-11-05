#include "Board.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "Player.h"

Board::Board() = default;

void Board::reset() {
    walls_.clear();
}

void Board::drawBoard(const std::vector<Player>& players) const {
    const int N = kSize;              // 9
    const std::string CELL = "[ ]";   // 실제 칸
    const std::string GAP = "   ";   // 칸 사이 간격

    // 9x9 플레이어 위치
    std::vector<std::vector<char>> grid(N, std::vector<char>(N, ' '));
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i].isDead()) continue;
        Position p = players[i].getPosition();
        if (!isWithinBounds(p)) continue;
        grid[p.row][p.col] = static_cast<char>('1' + static_cast<int>(i)); // 1,2,3,4...
    }

    // 1) 맨 위: 알파벳만 (칸 없음!!)
    // 왼쪽에 숫자 나올 자리를 맞춰주기 위해 대략 4칸 비움
    std::cout << "    ";
    // 칸 9개 사이가 8개니까 알파벳도 8개
    for (int g = 0; g < N - 1; ++g) {
        // 밑에서 한 칸을 CELL(3글자) + GAP(3글자)로 찍으니까
        // 여기서도 그 폭을 흉내내서 가운데에 글자 넣기
        std::cout << "   " << static_cast<char>('A' + g) << "   ";
    }
    std::cout << "\n";

    // 2) 9줄의 실제 칸
    for (int r = 0; r < N; ++r) {
        // 2-1) 실제 칸 줄 (9×9만 나와야 하는 줄)
        std::cout << "    ";
        for (int c = 0; c < N; ++c) {
            if (grid[r][c] == ' ')
                std::cout << CELL;
            else
                std::cout << "[" << grid[r][c] << "]";

            if (c != N - 1)
                std::cout << GAP;     // 여기 나중에 가로벽 들어갈 자리
        }
        std::cout << "\n";

        // 2-2) 칸과 칸 사이 줄 → 숫자만, 나머지는 공백
        if (r != N - 1) {
            int label = r + 1;        // 1~8
            std::cout << "  " << label << " ";
            // 나머지는 위 줄과 똑같은 폭만큼 그냥 공백
            for (int c = 0; c < N; ++c) {
                std::cout << "   ";   // CELL 대신 공백
                if (c != N - 1)
                    std::cout << GAP; // 사이 공백
            }
            std::cout << "\n";
        }
    }
}




bool Board::isWithinBounds(const Position& position) const {
    return position.row >= 0 && position.row < kSize && position.col >= 0 && position.col < kSize;
}

bool Board::placeWall(const Position& position, bool horizontal) {
    if (!isWithinBounds(position)) {
        return false;
    }

    if (horizontal) {
        if (position.row >= kSize - 1 || position.col >= kSize - 1) {
            return false;
        }
    }
    else {
        if (position.row >= kSize - 1 || position.col >= kSize - 1) {
            return false;
        }
    }

    if (hasWall(position, horizontal)) {
        return false;
    }

    walls_.push_back({ position, horizontal });
    return true;
}

bool Board::hasWall(const Position& position, bool horizontal) const {
    return std::any_of(walls_.begin(), walls_.end(), [&](const WallPlacement& wall) {
        return wall.horizontal == horizontal && wall.position.row == position.row &&
            wall.position.col == position.col;
        });
}

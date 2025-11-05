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
    const int N = kSize;         
    const std::string CELL = "[ ]"; 
    const std::string GAP = "   ";   

    
    std::vector<std::vector<char>> grid(N, std::vector<char>(N, ' '));
    for (std::size_t i = 0; i < players.size(); ++i) {
        if (players[i].isDead()) continue;
        Position p = players[i].getPosition();
        if (!isWithinBounds(p)) continue;
        grid[p.row][p.col] = static_cast<char>('1' + static_cast<int>(i)); 
    }

    std::cout << "    ";
    for (int g = 0; g < N - 1; ++g) {
        std::cout << "   " << static_cast<char>('A' + g) << "   ";
    }
    std::cout << "\n";

    for (int r = 0; r < N; ++r) {
        std::cout << "    ";
        for (int c = 0; c < N; ++c) {
            if (grid[r][c] == ' ')
                std::cout << CELL;
            else
                std::cout << "[" << grid[r][c] << "]";

            if (c != N - 1)
                std::cout << GAP;     
        }
        std::cout << "\n";

        if (r != N - 1) {
            int label = r + 1;        
            std::cout << "  " << label << " ";
            for (int c = 0; c < N; ++c) {
                std::cout << "   ";  
                if (c != N - 1)
                    std::cout << GAP;
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

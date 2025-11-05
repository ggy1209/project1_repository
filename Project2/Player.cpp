#include "Player.h"

#include <cctype>
#include <iostream>

using namespace std;

namespace {
bool isValidDirection(char direction) {
    direction = static_cast<char>(std::tolower(static_cast<unsigned char>(direction)));
    switch (direction) {
        case 'b':
        case 'h':
        case 'i':
        case 'k':
        case 'm':
        case 'n':
        case 'u':
        case 'y':
            return true;
        default:
            return false;
    }
}

int directionToRowOffset(char direction) {
    direction = static_cast<char>(std::tolower(static_cast<unsigned char>(direction)));
    switch (direction) {
        case 'y':
        case 'i':
        case 'u':
            return -1;  // Up and diagonals
        case 'b':
        case 'm':
        case 'n':
            return 1;  // Down and diagonals
        default:
            return 0;
    }
}

int directionToColOffset(char direction) {
    direction = static_cast<char>(std::tolower(static_cast<unsigned char>(direction)));
    switch (direction) {
        case 'i':
        case 'k':
        case 'm':
            return 1;  // Right and diagonals
        case 'b':
        case 'h':
        case 'y':
            return -1;  // Left and diagonals
        default:
            return 0;
    }
}
}  // namespace

Player::Player(const std::string& name, const Position& startPosition, int totalWalls)
    : name_(name),
      position_(startPosition),
      wallsRemaining_(totalWalls),
      eliminated_(false) {}

Position Player::previewMove(char direction) const {
    direction = static_cast<char>(std::tolower(static_cast<unsigned char>(direction)));
    Position target = position_;
    if (!isValidDirection(direction)) {
        return target;
    }

    target.row += directionToRowOffset(direction);
    target.col += directionToColOffset(direction);
    return target;
}

void Player::move(char direction, int steps) {
    direction = static_cast<char>(std::tolower(static_cast<unsigned char>(direction)));
    if (eliminated_) {
        cout << name_ << " cannot move because they are eliminated.\n";
        return;
    }

    if (!isValidDirection(direction)) {
        cout << "Invalid move input for " << name_ << ". Use h/b/y for left, k/m/i for right, u/i/y for up, n/m/b for down.\n";
        return;
    }

    int rowOffset = directionToRowOffset(direction) * steps;
    int colOffset = directionToColOffset(direction) * steps;
    int newRow = position_.row + rowOffset;
    int newCol = position_.col + colOffset;

    if (!isWithinBoard(newRow, newCol)) {
        std::cout << name_ << " cannot move outside the board bounds.\n";
        return;
    }

    position_.row = newRow;
    position_.col = newCol;
}

void Player::showStatus() const {
    std::cout << name_ << " - Position: (" << position_.row << ", " << position_.col
              << "), Walls left: " << wallsRemaining_;
    if (eliminated_) {
        std::cout << " [ELIMINATED]";
    }
    std::cout << '\n';
}

bool Player::isDead() const {
    return eliminated_;
}

const std::string& Player::getName() const {
    return name_;
}

Position Player::getPosition() const {
    return position_;
}

int Player::getWallsRemaining() const {
    return wallsRemaining_;
}

bool Player::hasWallsRemaining() const {
    return wallsRemaining_ > 0;
}

bool Player::placeWall() {
    if (eliminated_) {
        std::cout << name_ << " cannot place walls after elimination.\n";
        return false;
    }

    if (wallsRemaining_ == 0) {
        std::cout << name_ << " has no walls left to place.\n";
        return false;
    }

    --wallsRemaining_;
    return true;
}

void Player::eliminate() {
    eliminated_ = true;
}

bool Player::isWithinBoard(int row, int col) const {
    return row >= 0 && row < kBoardSize && col >= 0 && col < kBoardSize;
}

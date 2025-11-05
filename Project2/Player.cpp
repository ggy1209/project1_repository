#include "Player.h"

#include <iostream>

using namespace std;

namespace {
int directionToRowOffset(int direction) {
    switch (direction) {
        case 'u':
            return -1;  // Up
        case 'n':
            return 1;  // Down
        default:
            return 0;
    }
}

int directionToColOffset(int direction) {
    switch (direction) {
        case 'k':
            return 1;  // Right
        case 'h':
            return -1;  // Left
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

Position Player::previewMove(int direction) const {
    Position target = position_;
    if (direction < 0 || direction > 3) {
        return target;
    }

    target.row += directionToRowOffset(direction);
    target.col += directionToColOffset(direction);
    return target;
}

void Player::move(int direction) {
    if (eliminated_) {
        cout << name_ << " cannot move because they are eliminated.\n";
        return;
    }

    if (direction != 'u' && direction != 'n' && direction != 'k' && direction != 'h') {
        cout << "Invalid move input for " << name_ << ". Use 0:Up, 1:Right, 2:Down, 3:Left.\n";
        return;
    }

    int newRow = position_.row + directionToRowOffset(direction);
    int newCol = position_.col + directionToColOffset(direction);

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

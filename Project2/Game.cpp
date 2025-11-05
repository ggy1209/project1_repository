#include "Game.h"

#include <iostream>
#include <limits>

using namespace std;

Game::Game() : currentTurn_(0), isGameOver_(false) {
    initializePlayers();
}

void Game::start() {
    initializePlayers();
    isGameOver_ = false;
    winnerName_.clear();
    currentTurn_ = 0;

    cout << "Quoridor game start!\n";

    while (!isGameOver_) {
        showStatus();
        bool turnCompleted = handleInput();
        if (isGameOver_) {
            break;
        }
        if (turnCompleted) {
            checkGameOver();
            if (!isGameOver_) {
                nextTurn();
            }
        }
    }

    if (!winnerName_.empty()) {
        cout << "Player" << winnerName_ << " wins!\n";
    }
    else {
        cout << "Game ended without a winner.\n";
    }
}

void Game::initializePlayers() {
    board_.reset();
    players_.clear();

    const int middleColumn = Board::kSize / 2;
    const int middleRow = Board::kSize / 2;

    players_.emplace_back("Player 1", Position{ 0, middleColumn });
    players_.emplace_back("Player 2", Position{ Board::kSize - 1, middleColumn });
    players_.emplace_back("Player 3", Position{ middleRow, 0 });                     // ¿ÞÂÊ
    players_.emplace_back("Player 4", Position{ middleRow, Board::kSize - 1 });      // ¿À¸¥ÂÊ
        }

        void Game::showStatus() const {
        std::cout << "---- Turn " << currentTurn_ + 1 << " (" << players_[currentTurn_].getName()
            << ") ----\n";
        board_.drawBoard(players_);
        for (const auto& player : players_) {
            player.showStatus();
        }
    }

    bool Game::handleInput() {
        std::cout << players_[currentTurn_].getName()
            << " command (m <dir>/w <row> <col> <h|v>/q): ";
        char command = '\0';
        std::cin >> command;
        if (!std::cin) {
            isGameOver_ = true;
            return false;
        }

        bool turnCompleted = false;

        switch (command) {
        case 'm':
        case 'M': {
            int direction = -1;
            std::cin >> direction;
            if (!std::cin) {
                std::cin.clear();
                std::cout << "Invalid move input. Try again.\n";
            }
            else {
                turnCompleted = handleMoveCommand(direction);
            }
            break;
        }
        case 'w':
        case 'W': {
            int row = -1;
            int col = -1;
            char orientation = '\0';
            std::cin >> row >> col >> orientation;
            if (!std::cin) {
                std::cin.clear();
                std::cout << "Invalid wall input. Try again.\n";
            }
            else {
                turnCompleted = handleWallCommand(row, col, orientation);
            }
            break;
        }
        case 'q':
        case 'Q':
            isGameOver_ = true;
            if (players_.size() > 1) {
                winnerName_ = players_[(currentTurn_ + 1) % players_.size()].getName();
            }
            turnCompleted = true;
            break;
        default:
            std::cout << "Unknown command. Use m, w, or q.\n";
            break;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return turnCompleted;
    }

    bool Game::handleMoveCommand(int direction) {
        if (direction < 0 || direction > 3) {
            std::cout << "Invalid direction. Use 0:Up, 1:Right, 2:Down, 3:Left.\n";
            return false;
        }

        Position target = players_[currentTurn_].previewMove(direction);
        if (!board_.isWithinBounds(target)) {
            std::cout << "Move is outside the board.\n";
            return false;
        }

        if (isCellOccupied(target, currentTurn_)) {
            std::cout << "Another player already occupies that cell.\n";
            return false;
        }

        players_[currentTurn_].move(direction);
        return true;
    }

    bool Game::handleWallCommand(int row, int col, char orientation) {
        if (players_[currentTurn_].isDead()) {
            std::cout << "Eliminated players cannot place walls.\n";
            return false;
        }

        if (!players_[currentTurn_].hasWallsRemaining()) {
            std::cout << "No walls remaining to place.\n";
            return false;
        }

        Position position{ row, col };
        bool horizontal = false;

        if (orientation == 'h' || orientation == 'H') {
            horizontal = true;
        }
        else if (orientation == 'v' || orientation == 'V') {
            horizontal = false;
        }
        else {
            std::cout << "Orientation must be 'h' (horizontal) or 'v' (vertical).\n";
            return false;
        }

        if (!board_.placeWall(position, horizontal)) {
            std::cout << "Cannot place a wall at that location.\n";
            return false;
        }

        players_[currentTurn_].placeWall();
        return true;
    }

    void Game::nextTurn() {
        if (players_.empty()) {
            return;
        }

        currentTurn_ = (currentTurn_ + 1) % players_.size();
    }

    void Game::checkGameOver() {
        for (std::size_t index = 0; index < players_.size(); ++index) {
            if (players_[index].isDead()) {
                continue;
            }

            if (hasPlayerReachedGoal(index)) {
                isGameOver_ = true;
                winnerName_ = players_[index].getName();
                std::cout << winnerName_ << " reached the goal!\n";
                break;
            }
        }
    }

    bool Game::hasPlayerReachedGoal(std::size_t playerIndex) const {
        if (playerIndex >= players_.size()) {
            return false;
        }

        Position position = players_[playerIndex].getPosition();
        if (players_.size() == 2) {
            if (playerIndex == 0) {
                return position.row == Board::kSize - 1;
            }
            if (playerIndex == 1) {
                return position.row == 0;
            }
        }

        return false;
    }

    bool Game::isCellOccupied(const Position & position, std::size_t ignoreIndex) const {
        for (std::size_t index = 0; index < players_.size(); ++index) {
            if (index == ignoreIndex || players_[index].isDead()) {
                continue;
            }
            Position current = players_[index].getPosition();
            if (current.row == position.row && current.col == position.col) {
                return true;
            }
        }
        return false;
    }

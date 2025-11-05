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
    } else {
        cout << "Game ended without a winner.\n";
    }
}

void Game::initializePlayers() {
    board_.reset();
    players_.clear();

    const int middle = Board::kSize / 2;
    players_.emplace_back("Player 1", Position{middle, 0});
    players_.emplace_back("Player 2", Position{middle, Board::kSize - 1});
    players_.emplace_back("Player 3", Position{0, middle});
    players_.emplace_back("Player 4", Position{Board::kSize - 1, middle});  
}

void Game::showStatus() const {
    board_.drawBoard(players_);
    cout << players_[currentTurn_].getName() << "'s turn. You have " << players_[currentTurn_].getWallsRemaining() << " walls left.\n";
    
    //지워야할!
    for (const auto& player : players_) {
        player.showStatus();
    }
}

bool Game::handleInput() {
    cout << "Press 1 to move your piece and Press 2 to place a wall: ";
    int command;
    cin >> command;

    if (!cin) {
        cin.clear();
        std::cout << "Invalid command input. Try again.\n";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;
    }//뭐하는거임

    bool turnCompleted = false;

    switch (command) {
        case 1 : {
            char direction;
            cout << "Please input the direction you want to move (center is key \"j\"): ";
            cin >> direction;
            if (!cin) {
                cin.clear();
                std::cout << "Invalid move input. Try again.\n";
            } 
            else {
                turnCompleted = handleMoveCommand(direction);
            }
            break;
        }
        case 2:{
            int row;
            char col;
            char orientation;
            cout << "Please input the position of the wall coordinate and direction: ";
            cin >> row >> col >> orientation;
            if (!cin) {
                cin.clear();
                cout << "Invalid wall input. Try again.\n";
            } else {
                turnCompleted = handleWallCommand(row, col, orientation);
            }
            break;
        }
        default:
            cout << "Unknown command. Try again.\n";
            break;
    }

    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return turnCompleted;
}

bool Game::handleMoveCommand(int direction) {
    Position target = players_[currentTurn_].previewMove(direction);
    if (!board_.isWithinBounds(target)) {
        cout << "Move is outside the board.\n";
        return false;
    }

    if (isCellOccupied(target, currentTurn_)) {
        cout << "Another player already occupies that cell.\n";
        return false;
    }

    players_[currentTurn_].move(direction);
    return true;
}

bool Game::handleWallCommand(int row, int col, char orientation) {
    if (players_[currentTurn_].isDead()) {
        cout << "Eliminated players cannot place walls.\n";
        return false;
    }

    if (!players_[currentTurn_].hasWallsRemaining()) {
        cout << "No walls remaining to place.\n";
        return false;
    }

    Position position{row, col};
    bool horizontal = false;

    if (orientation == 'h' || orientation == 'H') {
        horizontal = true;
    } else if (orientation == 'v' || orientation == 'V') {
        horizontal = false;
    } else {
        cout << "Orientation must be 'h' (horizontal) or 'v' (vertical).\n";
        return false;
    }

    if (!board_.placeWall(position, horizontal)) {
        cout << "Cannot place a wall at that location.\n";
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

bool Game::isCellOccupied(const Position& position, std::size_t ignoreIndex) const {
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

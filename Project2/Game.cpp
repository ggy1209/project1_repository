#include "Game.h"

#include <iostream>
#include <limits>
#include <cstdlib>

using namespace std;

namespace {
char directionForDelta(int rowDelta, int colDelta) {
    if (rowDelta == -1) {
        if (colDelta == -1) return 'y';
        if (colDelta == 0) return 'u';
        if (colDelta == 1) return 'i';
    } else if (rowDelta == 0) {
        if (colDelta == -1) return 'h';
        if (colDelta == 1) return 'k';
    } else if (rowDelta == 1) {
        if (colDelta == -1) return 'b';
        if (colDelta == 0) return 'n';
        if (colDelta == 1) return 'm';
    }
    return '\0';
}
}  // namespace

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

// Initialize players at their starting positions

void Game::initializePlayers() {
    board_.reset();
    players_.clear();
    playerGoals_.clear();

    const int middle = Board::kSize / 2;
    Position p1{middle, 0};
    players_.emplace_back("Player 1", p1);
    playerGoals_.push_back(determineGoalType(p1));

    Position p2{middle, Board::kSize - 1};
    players_.emplace_back("Player 2", p2);
    playerGoals_.push_back(determineGoalType(p2));

    Position p3{0, middle};
    players_.emplace_back("Player 3", p3);
    playerGoals_.push_back(determineGoalType(p3));

    Position p4{Board::kSize - 1, middle};
    players_.emplace_back("Player 4", p4);
    playerGoals_.push_back(determineGoalType(p4));
}

// Display the current status of the game

void Game::showStatus() const {
    board_.drawBoard(players_);
    cout << players_[currentTurn_].getName() << "'s turn. You have " << players_[currentTurn_].getWallsRemaining() << " walls left.\n";
    
    //지워야할!
    for (const auto& player : players_) {
        player.showStatus();
    }
}

// Handle player input 

bool Game::handleInput() {
    cout << "Press 1 to move your piece and Press 2 to place a wall: ";
    int command;
    if (!(std::cin >> command)) {
        std::cin.clear();
        std::cout << "Invalid command input. Try again.\n";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;
    }

    bool turnCompleted = false;

    switch (command) {
        case 1: {
            char direction;
            cout << "Please input the direction you want to move (center is key \"j\"): ";
            if (!(std::cin >> direction)) {
                std::cin.clear();
                std::cout << "Invalid move input. Try again.\n";
            } else {
                turnCompleted = handleMoveCommand(direction);
            }
            break;
        }
        case 2: {
            int row;
            char col;
            char orientation;
            cout << "Please input the position of the wall coordinate and direction: ";
            if (!(std::cin >> row >> col >> orientation)) {
                std::cin.clear();
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

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return turnCompleted;
}

bool Game::handleMoveCommand(char direction) {
    Position current = players_[currentTurn_].getPosition();
    Position target = players_[currentTurn_].previewMove(direction);

    if (target.row == current.row && target.col == current.col) {
        cout << "Invalid direction. Use the keys surrounding 'j' (h, y, u, i, k, n, m, b).\n";
        return false;
    }

    if (!board_.isWithinBounds(target)) {
        cout << "Move is outside the board.\n";
        return false;
    }

    if (board_.isMoveBlocked(current, target)) {
        cout << "A wall blocks that move.\n";
        return false;
    }

    if (isCellOccupied(target, currentTurn_)) {
        Position jumpTarget{
            target.row + (target.row - current.row),
            target.col + (target.col - current.col)
        };

        bool jumpWithin = board_.isWithinBounds(jumpTarget);
        bool jumpBlocked = jumpWithin ? board_.isMoveBlocked(target, jumpTarget) : true;
        bool jumpOccupied = jumpWithin ? isCellOccupied(jumpTarget, currentTurn_) : false;

        if (jumpWithin && !jumpBlocked && !jumpOccupied) {
            players_[currentTurn_].move(direction, 2);
            return true;
        }

        if (jumpBlocked) {
            vector<Position> diagonalTargets;
            if (target.row == current.row) {
                diagonalTargets.push_back({target.row - 1, target.col});
                diagonalTargets.push_back({target.row + 1, target.col});
            } else if (target.col == current.col) {
                diagonalTargets.push_back({target.row, target.col - 1});
                diagonalTargets.push_back({target.row, target.col + 1});
            }

            for (const Position& diagonal : diagonalTargets) {
                if (!canMoveDiagonally(current, target, diagonal, currentTurn_)) {
                    continue;
                }

                int rowDelta = diagonal.row - current.row;
                int colDelta = diagonal.col - current.col;
                char diagDirection = directionForDelta(rowDelta, colDelta);
                if (diagDirection == '\0') {
                    continue;
                }

                players_[currentTurn_].move(diagDirection);
                return true;
            }

            cout << "Cannot move diagonally; both flanks are blocked.\n";
            return false;
        }

        if (jumpOccupied) {
            cout << "Cannot jump because the landing cell is occupied.\n";
            return false;
        }

        cout << "Cannot jump outside the board.\n";
        return false;
    }

    players_[currentTurn_].move(direction);
    return true;
}

// Handle wall placement command

bool Game::handleWallCommand(int row, char col, char orientation) {
    if (players_[currentTurn_].isDead()) {
        cout << "Eliminated players cannot place walls.\n";
        return false;
    }

    if (!players_[currentTurn_].hasWallsRemaining()) {
        cout << "No walls remaining to place.\n";
        return false;
    }

    // 1) 사용자 입력을 내부 좌표로 변환
    // 사용자는 1~8로 줄 번호를 준다고 가정 → 0-based로
    int rowIdx = row - 1;      // 1 → 0, 8 → 7

    // 알파벳은 A~H or a~h 로 온다고 가정 → 0-based로
    int colIdx;
    if (col >= 'A' && col <= 'Z') {
        colIdx = col - 'A';    // 'A' → 0, 'H' → 7
    } else if (col >= 'a' && col <= 'z') {
        colIdx = col - 'a';
    } else {
        cout << "Column must be A~H.\n";
        return false;
    }

    // 9x9일 때 유효한 벽 위치는 0~7 까지
    if (rowIdx < 0 || rowIdx >= Board::kSize - 1 ||
        colIdx < 0 || colIdx >= Board::kSize - 1) {
        cout << "Wall position out of range.\n";
        return false;
    }

    // 2) 방향 해석
    bool horizontal;
    if (orientation == 'h' || orientation == 'H') {
        horizontal = true;
    } else if (orientation == 'v' || orientation == 'V') {
        horizontal = false;
    } else {
        cout << "Orientation must be 'h' (horizontal) or 'v' (vertical).\n";
        return false;
    }

    // 3) 보드에 실제로 벽 놓기
    Position position{rowIdx, colIdx};
    if (!board_.placeWall(position, horizontal)) {
        cout << "Cannot place a wall at that location.\n";
        return false;
    }

    if (!allPlayersHavePath()) {
        board_.removeWall(position, horizontal);
        cout << "That wall blocks every route to a goal for at least one player.\n";
        return false;
    }

    // 4) 플레이어가 가진 벽 개수 감소
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
    auto goalCondition = goalConditionForPlayer(playerIndex);
    return goalCondition(position);
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

std::function<bool(const Position&)> Game::goalConditionForPlayer(std::size_t playerIndex) const {
    if (playerIndex >= playerGoals_.size()) {
        return [](const Position&) { return false; };
    }

    GoalType goal = playerGoals_[playerIndex];

    switch (goal) {
        case GoalType::Row0:
            return [](const Position& pos) { return pos.row == 0; };
        case GoalType::RowLast:
            return [](const Position& pos) { return pos.row == Board::kSize - 1; };
        case GoalType::Col0:
            return [](const Position& pos) { return pos.col == 0; };
        case GoalType::ColLast:
            return [](const Position& pos) { return pos.col == Board::kSize - 1; };
        default:
            return [](const Position&) { return false; };
    }
}

bool Game::playerHasPathToGoal(std::size_t playerIndex) const {
    if (playerIndex >= players_.size()) {
        return false;
    }

    if (players_[playerIndex].isDead()) {
        return true;
    }

    auto goalCondition = goalConditionForPlayer(playerIndex);
    return board_.existsPath(players_[playerIndex].getPosition(), goalCondition);
}

bool Game::allPlayersHavePath() const {
    for (std::size_t i = 0; i < players_.size(); ++i) {
        if (!playerHasPathToGoal(i)) {
            return false;
        }
    }
    return true;
}

bool Game::canMoveDiagonally(const Position& current,
                             const Position& target,
                             const Position& diagonal,
                             std::size_t movingIndex) const {
    if (!board_.isWithinBounds(diagonal)) {
        return false;
    }
    if (board_.isMoveBlocked(current, target)) {
        return false;
    }
    if (isCellOccupied(diagonal, movingIndex)) {
        return false;
    }
    if (board_.isMoveBlocked(target, diagonal)) {
        return false;
    }

    int primaryRow = target.row - current.row;
    int primaryCol = target.col - current.col;
    int diagonalRowDelta = diagonal.row - target.row;
    int diagonalColDelta = diagonal.col - target.col;

    if (std::abs(primaryRow) + std::abs(primaryCol) != 1) {
        return false;
    }
    if (std::abs(diagonalRowDelta) + std::abs(diagonalColDelta) != 1) {
        return false;
    }
    if (primaryRow != 0 && diagonalColDelta == 0) {
        return false;
    }
    if (primaryCol != 0 && diagonalRowDelta == 0) {
        return false;
    }
    return true;
}

Game::GoalType Game::determineGoalType(const Position& startPosition) const {
    if (startPosition.row == 0) {
        return GoalType::RowLast;
    }
    if (startPosition.row == Board::kSize - 1) {
        return GoalType::Row0;
    }
    if (startPosition.col == 0) {
        return GoalType::ColLast;
    }
    if (startPosition.col == Board::kSize - 1) {
        return GoalType::Col0;
    }
    return GoalType::RowLast;
}

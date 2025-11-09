#include "Game.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>

using namespace std;

namespace {
inline Position makePos(int r, int c){
    Position p;
    p.row=r;
    p.col=c;
    return p;
}
bool isValidDirectionInput(char direction) {
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

constexpr const char* kResetColor = "\033[0m";
constexpr const char* kYellowColor = "\033[33m";
constexpr const char* kGreenColor = "\033[32m";
constexpr const char* kRedColor = "\033[31m";
constexpr const char* kBlueColor = "\033[34m";

const char* colorForPlayerIndex(std::size_t index) {
    switch (index) {
        case 0: return kYellowColor;
        case 1: return kGreenColor;
        case 2: return kRedColor;
        case 3: return kBlueColor;
        default: return kResetColor;
    }
}

std::string colorizeText(const std::string& text, const char* color) {
    return std::string(color) + text + kResetColor;
}

std::string colorizeDigits(const std::string& text, std::size_t playerIndex) {
    std::string result;
    const char* color = colorForPlayerIndex(playerIndex);
    for (char ch : text) {
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            result += colorizeText(std::string(1, ch), color);
        } else {
            result += ch;
        }
    }
    return result;
}
}  // namespace

Game::Game() : currentTurn_(0), isGameOver_(false), skipInputFlush_(false) {
    initializePlayers();
}

void Game::start() {
    initializePlayers();
    isGameOver_ = false;
    winnerName_.clear();
    currentTurn_ = 0;
    skipInputFlush_ = false;

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
    Position p1=makePos(middle, 0);
    players_.emplace_back("Player 1", p1);
    playerGoals_.push_back(determineGoalType(p1));

    Position p2=makePos(middle, Board::kSize - 1);
    players_.emplace_back("Player 2", p2);
    playerGoals_.push_back(determineGoalType(p2));

    Position p3=makePos(0, middle);
    players_.emplace_back("Player 3", p3);
    playerGoals_.push_back(determineGoalType(p3));

    Position p4=makePos(Board::kSize - 1, middle);
    players_.emplace_back("Player 4", p4);
    playerGoals_.push_back(determineGoalType(p4));
}

// Display the current status of the game

void Game::showStatus() const {
    board_.drawBoard(players_);
    std::string coloredName = colorizeDigits(players_[currentTurn_].getName(),
                                             currentTurn_);
    cout << coloredName << "'s turn. You have "
         << players_[currentTurn_].getWallsRemaining()
         << " walls left.\n";
    
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

    if (!skipInputFlush_) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else {
        skipInputFlush_ = false;
    }
    return turnCompleted;
}

bool Game::handleMoveCommand(char direction) {
    direction = static_cast<char>(std::tolower(static_cast<unsigned char>(direction)));
    if (!isValidDirectionInput(direction)) {
        cout << "Invalid direction. Use the keys surrounding 'j' (h, y, u, i, k, n, m, b).\n";
        return false;
    }

    Position current = players_[currentTurn_].getPosition();
    Position target = players_[currentTurn_].previewMove(direction);

    if (!board_.isWithinBounds(target)) {
        cout << "Move is outside the board.\n";
        return false;
    }

    bool isDiagonal = (current.row != target.row) && (current.col != target.col);
    if (isDiagonal) {
        return handleDiagonalMove(direction, current, target);
    }

    return handleOrthogonalMove(direction, current, target);
}

bool Game::handleOrthogonalMove(char direction,
                                const Position& current,
                                const Position& target) {
    if (board_.isMoveBlocked(current, target)) {
        cout << "A wall blocks that move.\n";
        return false;
    }

    if (isCellOccupied(target, currentTurn_)) {
        Position jumpTarget=makePos(
            target.row + (target.row - current.row),
            target.col + (target.col - current.col)
        );

        if (!board_.isWithinBounds(jumpTarget)) {
            cout << "Cannot jump outside the board.\n";
            return false;
        }

        if (board_.isMoveBlocked(target, jumpTarget)) {
            cout << "Cannot jump because a wall blocks the landing path.\n";
            return false;
        }

        if (isCellOccupied(jumpTarget, currentTurn_)) {
            cout << "Cannot jump because the landing cell is occupied.\n";
            return false;
        }

        players_[currentTurn_].move(direction, 2);
        handleRedCellInteraction();
        return true;
    }

    players_[currentTurn_].move(direction);
    handleRedCellInteraction();
    return true;
}

bool Game::handleDiagonalMove(char direction,
                              const Position& current,
                              const Position& target) {
    if (isCellOccupied(target, currentTurn_)) {
        cout << "Target cell is already occupied.\n";
        return false;
    }

    int rowStep = (target.row - current.row) > 0 ? 1 : -1;
    int colStep = (target.col - current.col) > 0 ? 1 : -1;

    vector<Position> adjacentCandidates;
    adjacentCandidates.push_back(makePos(current.row + rowStep, current.col));   adjacentCandidates.push_back(makePos(current.row, current.col + colStep));

    for (const Position& opponentPos : adjacentCandidates) {
        if (!board_.isWithinBounds(opponentPos)) {
            continue;
        }
        if (!isCellOccupied(opponentPos, currentTurn_)) {
            continue;
        }
        if (board_.isMoveBlocked(current, opponentPos)) {
            continue;
        }

        Position behind=makePos(
            opponentPos.row + (opponentPos.row - current.row),
            opponentPos.col + (opponentPos.col - current.col)
        );
        bool wallBehind = false;
        if (!board_.isWithinBounds(behind)) {
            wallBehind = true;
        } else if (board_.isMoveBlocked(opponentPos, behind)) {
            wallBehind = true;
        }
        if (!wallBehind) {
            continue;
        }

        if (board_.isMoveBlocked(opponentPos, target)) {
            continue;
        }

        players_[currentTurn_].move(direction);
        handleRedCellInteraction();
        return true;
    }

    cout << "Diagonal move requires an adjacent opponent with a blocking wall and a clear diagonal path.\n";
    return false;
}

bool Game::isRedCellPosition(const Position& position) const {
    return (position.row==2&&position.col==2)||(position.row==2&&position.col==6)||(position.row==6&&position.col==2)||(position.row==6&&position.col==6);
}

void Game::handleRedCellInteraction() {
    Position position = players_[currentTurn_].getPosition();
    if (!isRedCellPosition(position)) {
        return;
    }

    board_.drawBoard(players_);

    cout << "You are in the red pixel!\n";

    auto flushLine = [&]() {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        skipInputFlush_ = true;
    };

    while (true) {
        cout << "Enter the player ID th switch with (or enter your own ID to stay):  ";

        int targetId;
        if (!(std::cin >> targetId)) {
            std::cin.clear();
            flushLine();
            cout << "Invalid input. Try again.\n";
            continue;
        }

        flushLine();

        if (targetId < 1 || static_cast<std::size_t>(targetId) > players_.size()) {
            cout << "Invalid player ID. Try again.\n";
            continue;
        }

        std::size_t targetIndex = static_cast<std::size_t>(targetId - 1);
        if (targetIndex == currentTurn_) {
            cout << "Remaining on the red pixel.\n";
            break;
        }

        Position otherPosition = players_[targetIndex].getPosition();
        auto goalCheck = [&](const Position& pos) {
            return pos.row == otherPosition.row && pos.col == otherPosition.col;
        };

        if (!board_.existsPath(position, goalCheck)) {
            cout << "All paths are blocked by walls. Choose another player.\n";
            continue;
        }

        players_[targetIndex].setPosition(position);
        players_[currentTurn_].setPosition(otherPosition);
        cout << "Swapped positions with Player " << (targetIndex + 1) << ".\n";
        break;
    }
}

// Handle wall placement command

bool Game::handleWallCommand(int row, char col, char orientation) {
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
    Position position=makePos(rowIdx, colIdx);
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
        if (index == ignoreIndex) {
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

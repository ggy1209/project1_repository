#ifndef GAME_HPP
#define GAME_HPP

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "Board.h"
#include "Player.h"

using namespace std;

class Game {
public:
    Game();

    void start();

private:
    enum class GoalType {
        Row0,
        RowLast,
        Col0,
        ColLast
    };

    void initializePlayers();
    void showStatus() const;
    bool handleInput();
    bool handleMoveCommand(char direction);
    bool handleWallCommand(int row, char col, char orientation);
    void nextTurn();
    void checkGameOver();
    bool hasPlayerReachedGoal(std::size_t playerIndex) const;
    bool isCellOccupied(const Position& position, std::size_t ignoreIndex) const;
    std::function<bool(const Position&)> goalConditionForPlayer(std::size_t playerIndex) const;
    bool playerHasPathToGoal(std::size_t playerIndex) const;
    bool allPlayersHavePath() const;
    GoalType determineGoalType(const Position& startPosition) const;

    Board board_;
    vector<Player> players_;
    vector<GoalType> playerGoals_;
    size_t currentTurn_;
    bool isGameOver_;
    string winnerName_;
};

#endif // GAME_HPP

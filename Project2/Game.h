#ifndef GAME_HPP
#define GAME_HPP

#include <cstddef>
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
    void initializePlayers();
    void showStatus() const;
    bool handleInput();
    bool handleMoveCommand(char direction);
    bool handleWallCommand(int row, int col, char orientation);
    void nextTurn();
    void checkGameOver();
    bool hasPlayerReachedGoal(std::size_t playerIndex) const;
    bool isCellOccupied(const Position& position, std::size_t ignoreIndex) const;

    Board board_;
    vector<Player> players_;
    size_t currentTurn_;
    bool isGameOver_;
    string winnerName_;
};

#endif // GAME_HPP

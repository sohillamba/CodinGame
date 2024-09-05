#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

enum GameState : int {
    Running,
    Draw,
    Win
};

enum Symbol : int {
    Tic = 1,
    Tac = 2,
    Space = 0
};

class Move {
public:
    int row, col;
    Move() {
        row = -1; col = -1;
    }
    Move(int r, int c) {
        row = r; col = c;
    }
};

class TicTacToe {
public:
    std::vector<std::vector<Symbol>> board;
    int moveCount;
    TicTacToe () {
        moveCount = 0;
        board = std::vector<std::vector<Symbol>> (3, std::vector<Symbol> (3, Symbol::Space));
    }

    void makeMove(const Move move, const Symbol symbol) {
        moveCount++;
        board[move.row][move.col] = symbol;
    }

    void unMakeMove(const Move move) {
        moveCount--;
        board[move.row][move.col] = Symbol::Space;
    }

    static Symbol swap(const Symbol symbol) {
        return symbol == Symbol::Tic ? Symbol::Tac : Symbol::Tic;
    }

    GameState evalState(const Symbol symbol) {
        for (int i = 0; i < 3; i++) {
            if (board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol) return GameState::Win;
            if (board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol) return GameState::Win;
        }
        if (board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol) return GameState::Win;
        if (board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol) return GameState::Win;

        if (moveCount == 9) return GameState::Draw;
        return GameState::Running;
    }
};


class Player {
public:
    std::string name;
    virtual Move getMove(TicTacToe&, const Symbol) = 0;
};

class PlayerHuman : public Player {
public:
    virtual Move getMove(TicTacToe& game, const Symbol symbol) override {
        Move move;
        std::cin >> move.row >> move.col; std::cin.ignore();

        int validActionCount;
        std::cin >> validActionCount; std::cin.ignore();
        
        std::vector<Move> validMoves(validActionCount);
        for (int i = 0; i < validActionCount; i++) {
            std::cin >> validMoves[i].row >> validMoves[i].col; std::cin.ignore();
        }

        return move;
    };
};

class PlayerAI : public Player {
public:
    int maxDepth = 10;
    virtual Move getMove(TicTacToe& game, const Symbol symbol) override {
        // select random move if board is empty
        if (game.moveCount == 0) {
            return Move(0, 0);
            // return Move(std::rand() % 3, std::rand() % 3);
        }

        // calculate best move with minimax
        int bestScore = -1000;
        std::vector<Move> moves;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (game.board[i][j] == Symbol::Space) {
                    game.makeMove(Move(i, j), symbol);
                    int score = minimax(game, 0, false, symbol);
                    if (score > bestScore) {
                        bestScore = score;
                        moves.clear();
                    }
                    if (score == bestScore) {
                        moves.push_back(Move(i, j));
                    }
                    game.unMakeMove(Move(i, j));
                }
            }
        }
        return moves[std::rand() % moves.size()];
    }

protected:
    int minimax(TicTacToe& game, int depth, bool isMax, const Symbol symbol) {
        Symbol opponent = TicTacToe::swap(symbol);
        GameState state = game.evalState(symbol);

        if (state == GameState::Win)
            return 10 - depth;

        if (game.evalState(opponent) == GameState::Win)
            return -10 + depth;

        if (state == GameState::Draw || depth >= maxDepth)
            return 0;

        int best = isMax ? -1000 : 1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (game.board[i][j] == Symbol::Space) {
                    game.makeMove(Move(i, j), isMax ? symbol : opponent);
                    int score = minimax(game, depth + 1, !isMax, symbol);
                    best = isMax ? std::max(best, score) : std::min(best, score);
                    game.unMakeMove(Move(i, j));
                }
            }
        }
        return best;
    }
};

class Game {
private:
    TicTacToe game;

public:
    void play(Player& player1, Player& player2) {
        bool playerOneToMove = true;
        Symbol symbol = Symbol::Tic;

        while (1) {
            Player& player = playerOneToMove ? player1 : player2;

            Move move = player.getMove(game, symbol);
            if (move.row != -1 && move.col != -1) {
                game.makeMove(move, symbol);

                if (player.name == "AI") {
                    std::cout << move.row << " " << move.col << std::endl;
                }
            }

            playerOneToMove = !playerOneToMove;
            symbol = TicTacToe::swap(symbol);
        }
    }
};


int main() {
    std::srand(static_cast<unsigned int>(time(0)));

    PlayerHuman player1;
    player1.name = "human";

    PlayerAI player2;
    player2.name = "AI";

    Game game;
    game.play(player1, player2);
}






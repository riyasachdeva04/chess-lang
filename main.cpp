#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <regex>

using namespace std;

enum Piece { EMPTY, KNIGHT, QUEEN, KING, PAWN, ROOK, BISHOP };

class ChessBoard {
private:
    Piece board[8][8];
    stack<pair<string, string>> moveHistory;

    void positionToIndices(const string& pos, int& row, int& col) {
        row = 8 - (pos[1] - '0');
        col = pos[0] - 'a';
    }

public:
    ChessBoard() {
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                board[i][j] = EMPTY;
            }
        }
        board[0][1] = KNIGHT;
        board[6][3] = KING;
        board[7][2] = QUEEN;
    }

    void printBoard() {
        for (int i = 7; i >= 0; --i) {
            for (int j = 0; j < 8; ++j) {
                char pieceChar = '.';
                switch (board[i][j]) {
                    case KNIGHT: pieceChar = 'N'; break;
                    case QUEEN: pieceChar = 'Q'; break;
                    case KING: pieceChar = 'K'; break;
                    case PAWN: pieceChar = 'P'; break;
                    case ROOK: pieceChar = 'R'; break;
                    case BISHOP: pieceChar = 'B'; break;
                    default: break;
                }
                cout << pieceChar << " ";
            }
            cout << endl;
        }
        cout << "  a b c d e f g h" << endl;
    }

    void movePiece(const string& start, const string& end) {
        int startRow, startCol, endRow, endCol;
        positionToIndices(start, startRow, startCol);
        positionToIndices(end, endRow, endCol);

        if (board[startRow][startCol] != EMPTY) {
            board[endRow][endCol] = board[startRow][startCol];
            board[startRow][startCol] = EMPTY;
            moveHistory.push({start, end});
            cout << "Moved from " << start << " to " << end << endl;
        } else {
            cout << "No piece at " << start << endl;
        }
    }

    void undoMove() {
        if (moveHistory.empty()) {
            cout << "No moves to undo." << endl;
            return;
        }
        auto lastMove = moveHistory.top();
        moveHistory.pop();

        int startRow, startCol, endRow, endCol;
        positionToIndices(lastMove.first, startRow, startCol);
        positionToIndices(lastMove.second, endRow, endCol);

        board[startRow][startCol] = board[endRow][endCol];
        board[endRow][endCol] = EMPTY;
        cout << "Undid move from " << lastMove.second << " back to " << lastMove.first << endl;
    }

    bool isKingInCheck() {
        int kingRow, kingCol;
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (board[i][j] == KING) {
                    kingRow = i;
                    kingCol = j;
                }
            }
        }

        for (int i = 0; i < 8; ++i) {
            if (board[kingRow][i] == QUEEN || (board[kingRow][i] == ROOK && i != kingCol)) return true;
            if (board[i][kingCol] == QUEEN || (board[i][kingCol] == ROOK && i != kingRow)) return true;
        }

        for (int d = 1; d < 8; ++d) {
            if (kingRow - d >= 0 && kingCol - d >= 0) {
                if (board[kingRow - d][kingCol - d] == QUEEN || board[kingRow - d][kingCol - d] == BISHOP) return true;
            }
            if (kingRow - d >= 0 && kingCol + d < 8) {
                if (board[kingRow - d][kingCol + d] == QUEEN || board[kingRow - d][kingCol + d] == BISHOP) return true;
            }
            if (kingRow + d < 8 && kingCol - d >= 0) {
                if (board[kingRow + d][kingCol - d] == QUEEN || board[kingRow + d][kingCol - d] == BISHOP) return true;
            }
            if (kingRow + d < 8 && kingCol + d < 8) {
                if (board[kingRow + d][kingCol + d] == QUEEN || board[kingRow + d][kingCol + d] == BISHOP) return true;
            }
        }

        int knightMoves[8][2] = { {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2} };
        for (const auto& move : knightMoves) {
            int row = kingRow + move[0];
            int col = kingCol + move[1];
            if (row >= 0 && row < 8 && col >= 0 && col < 8 && board[row][col] == KNIGHT) {
                return true;
            }
        }

        return false;
    }
};

class Lexer {
public:
    vector<string> tokenize(const string& input) {
        vector<string> tokens;
        istringstream stream(input);
        string token;
        while (stream >> token) {
            if (token[0] != '#') {
                tokens.push_back(token);
            } else {
                break;
            }
        }
        return tokens;
    }
};

class ASTNode {
public:
    virtual void execute(ChessBoard& board) = 0;
    virtual ~ASTNode() = default;
};

class MoveNode : public ASTNode {
    string start, end;
public:
    MoveNode(const string& s, const string& e) : start(s), end(e) {}
    void execute(ChessBoard& board) override {
        board.movePiece(start, end);
    }
};

class UndoNode : public ASTNode {
public:
    void execute(ChessBoard& board) override {
        board.undoMove();
    }
};

class CheckNode : public ASTNode {
public:
    void execute(ChessBoard& board) override {
        if (board.isKingInCheck()) {
            cout << "King is in check!" << endl;
        } else {
            cout << "King is safe." << endl;
        }
    }
};

class Parser {
public:
    ASTNode* parse(const vector<string>& tokens) {
        if (tokens.empty()) {
            return nullptr;
        }

        if (tokens[0] == "move" && tokens.size() == 5 && tokens[1] == "from" && tokens[3] == "to") {
            return new MoveNode(tokens[2], tokens[4]);
        } else if (tokens[0] == "undo") {
            return new UndoNode();
        } else if (tokens[0] == "check") {
            return new CheckNode();
        } else {
            cout << "Invalid command!" << endl;
            return nullptr;
        }
    }
};

class Interpreter {
private:
    Lexer lexer;
    Parser parser;

public:
    void interpret(const string& command, ChessBoard& board) {
        auto tokens = lexer.tokenize(command);
        ASTNode* node = parser.parse(tokens);
        if (node) {
            node->execute(board);
            delete node;
        }
    }
};

int main() {
    ChessBoard board;
    Interpreter interpreter;
    string command;

    while (true) {
        cout << "ChessLang> ";
        getline(cin, command);
        if (command == "exit") break;
        interpreter.interpret(command, board);
        board.printBoard();
    }

    return 0;
}

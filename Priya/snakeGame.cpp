#include <iostream>
#include <deque>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define SLEEP(ms) usleep((ms) * 1000)

using namespace std;

// Enum for directions
enum Direction {
    UP, DOWN, LEFT, RIGHT
};

// Clear screen
void clearScreen() {
    system("clear");
}

// Set cursor position
void setCursorPosition(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}

// Hide and show cursor
void hideCursor() { printf("\e[?25l"); fflush(stdout); }
void showCursor() { printf("\e[?25h"); fflush(stdout); }

// Colors
const string RESET = "\033[0m";
const string CYAN = "\033[96m";
const string GREEN = "\033[92m";
const string YELLOW = "\033[93m";
const string RED = "\033[91m";
const string WHITE = "\033[97m";

// Position class
class Position {
public:
    int row, col;
    Position(int r = 0, int c = 0) : row(r), col(c) {}
    bool operator==(const Position &other) const {
        return row == other.row && col == other.col;
    }
};

// Food class
class Food {
private:
    Position position;
    char symbol;
public:
    Food() : symbol('O') {}
    Position getPosition() const { return position; }
    void setPosition(Position pos) { position = pos; }
    char getSymbol() const { return symbol; }
};

// Snake class
class Snake {
private:
    deque<Position> body;
    Direction direction;
    bool growing;
    char headSymbol, bodySymbol;

public:
    Snake(Position startPos, int length = 3)
        : direction(RIGHT), growing(false), headSymbol('#'), bodySymbol('o') {
        for (int i = 0; i < length; i++)
            body.push_back(Position(startPos.row, startPos.col - i));
    }

    Position getHead() const { return body.front(); }
    deque<Position> getBody() const { return body; }
    Direction getDirection() const { return direction; }

    void setDirection(Direction newDir) {
        if ((direction == UP && newDir == DOWN) ||
            (direction == DOWN && newDir == UP) ||
            (direction == LEFT && newDir == RIGHT) ||
            (direction == RIGHT && newDir == LEFT)) return;
        direction = newDir;
    }

    void move() {
        Position newHead = getHead();
        switch (direction) {
            case UP: newHead.row--; break;
            case DOWN: newHead.row++; break;
            case LEFT: newHead.col--; break;
            case RIGHT: newHead.col++; break;
        }
        body.push_front(newHead);
        if (!growing) body.pop_back();
        else growing = false;
    }

    void grow() { growing = true; }

    bool checkSelfCollision() const {
        Position head = getHead();
        for (size_t i = 1; i < body.size(); i++)
            if (body[i] == head) return true;
        return false;
    }

    char getHeadSymbol() const { return headSymbol; }
    char getBodySymbol() const { return bodySymbol; }
};

// GameBoard class
class GameBoard {
private:
    int rows, cols, score, highScore;
    Snake *snake;
    Food *food;
    bool gameOver;
    vector<vector<char>> previousBoard;

    void spawnFood() {
        deque<Position> snakeBody = snake->getBody();
        vector<Position> available;
        for (int r = 1; r < rows - 1; r++) {
            for (int c = 1; c < cols - 1; c++) {
                Position p(r, c);
                bool occupied = false;
                for (auto &part : snakeBody)
                    if (part == p) { occupied = true; break; }
                if (!occupied) available.push_back(p);
            }
        }
        if (!available.empty()) {
            int idx = rand() % available.size();
            food->setPosition(available[idx]);
        }
    }

public:
    GameBoard(int r = 20, int c = 40)
        : rows(r), cols(c), score(0), highScore(0), gameOver(false) {
        srand(time(0));
        snake = new Snake(Position(rows / 2, cols / 2));
        food = new Food();
        previousBoard = vector<vector<char>>(rows, vector<char>(cols, ' '));
        spawnFood();
    }

    ~GameBoard() { delete snake; delete food; }

    bool checkCollision() {
        Position head = snake->getHead();
        if (head.row <= 0 || head.row >= rows - 1 || head.col <= 0 || head.col >= cols - 1)
            return true;
        return snake->checkSelfCollision();
    }

    void update() {
        if (!gameOver) {
            snake->move();
            if (checkCollision()) {
                gameOver = true;
                highScore = max(highScore, score);
            } else {
                if (snake->getHead() == food->getPosition()) {
                    snake->grow();
                    score += 10;
                    spawnFood();
                }
            }
        }
    }

    void renderInitial() {
        clearScreen();
        hideCursor();
        vector<vector<char>> board(rows, vector<char>(cols, ' '));

        for (int i = 0; i < cols; i++) { board[0][i] = '='; board[rows - 1][i] = '='; }
        for (int i = 0; i < rows; i++) { board[i][0] = '|'; board[i][cols - 1] = '|'; }

        // Draw frame
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                if (r == 0 || r == rows - 1 || c == 0 || c == cols - 1)
                    cout << CYAN << board[r][c] << RESET;
                else cout << ' ';
            }
            cout << endl;
        }

        cout << WHITE << "\nScore: 0  |  High Score: 0  |  Length: 3" << RESET << endl;
        cout << WHITE << "Controls: W/A/S/D or Arrow Keys  |  Q: Quit" << RESET << endl;
        previousBoard = board;
    }

    void render() {
        vector<vector<char>> board(rows, vector<char>(cols, ' '));
        for (int i = 0; i < cols; i++) { board[0][i] = '='; board[rows - 1][i] = '='; }
        for (int i = 0; i < rows; i++) { board[i][0] = '|'; board[i][cols - 1] = '|'; }

        Position f = food->getPosition();
        board[f.row][f.col] = food->getSymbol();

        deque<Position> body = snake->getBody();
        for (size_t i = 0; i < body.size(); i++)
            board[body[i].row][body[i].col] = (i == 0) ? snake->getHeadSymbol() : snake->getBodySymbol();

        for (int r = 1; r < rows - 1; r++) {
            for (int c = 1; c < cols - 1; c++) {
                if (board[r][c] != previousBoard[r][c]) {
                    setCursorPosition(c, r);
                    if (board[r][c] == snake->getHeadSymbol())
                        cout << GREEN << board[r][c] << RESET;
                    else if (board[r][c] == snake->getBodySymbol())
                        cout << YELLOW << board[r][c] << RESET;
                    else if (board[r][c] == food->getSymbol())
                        cout << RED << board[r][c] << RESET;
                    else cout << ' ';
                    cout.flush();
                }
            }
        }

        setCursorPosition(0, rows);
        cout << WHITE << "Score: " << score << "  |  High Score: " << highScore
             << "  |  Length: " << body.size() << "   " << RESET;
        cout.flush();

        previousBoard = board;
    }

    Snake *getSnake() { return snake; }
    bool isGameOver() const { return gameOver; }
};

// Input handler for macOS
class InputHandler {
private:
    struct termios oldSettings;
public:
    InputHandler() { tcgetattr(STDIN_FILENO, &oldSettings); }
    ~InputHandler() { tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings); }

    char getKey() {
        struct termios newSettings = oldSettings;
        newSettings.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);

        int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        char ch = '\0';
        if (read(STDIN_FILENO, &ch, 1) > 0) {
            if (ch == 27) {
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) > 0 && read(STDIN_FILENO, &seq[1], 1) > 0)
                    if (seq[0] == '[') {
                        switch (seq[1]) {
                            case 'A': ch = 'U'; break;
                            case 'B': ch = 'D'; break;
                            case 'C': ch = 'R'; break;
                            case 'D': ch = 'L'; break;
                        }
                    }
            } else ch = toupper(ch);
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        return ch;
    }
};

// Game class
class Game {
private:
    GameBoard *board;
    InputHandler *input;
    bool running;
    int speed;

public:
    Game() : board(nullptr), running(true), speed(100) { input = new InputHandler(); }
    ~Game() { delete board; delete input; showCursor(); }

    void run() {
        clearScreen();
        cout << WHITE << "+=======================================+\n";
        cout << "|       SNAKE GAME - macOS EDITION      |\n";
        cout << "+=======================================+\n";
        cout << "\nPress ENTER to start..." << RESET;
        cin.ignore();

        while (running) {
            delete board;
            board = new GameBoard(20, 40);
            board->renderInitial();

            while (!board->isGameOver()) {
                board->render();
                char key = input->getKey();

                if (key == 'W' || key == 'U') board->getSnake()->setDirection(UP);
                else if (key == 'S' || key == 'D') board->getSnake()->setDirection(DOWN);
                else if (key == 'A' || key == 'L') board->getSnake()->setDirection(LEFT);
                else if (key == 'D' || key == 'R') board->getSnake()->setDirection(RIGHT);
                else if (key == 'Q') { running = false; break; }

                board->update();
                SLEEP(speed);
            }

            showCursor();
            clearScreen();
            cout << RED << "\nGAME OVER!\n" << RESET;
            cout << "Press R to Restart or Q to Quit.\n";
            char choice;
            do { choice = input->getKey(); SLEEP(50); } while (choice != 'R' && choice != 'Q');
            if (choice == 'Q') running = false;
        }

        clearScreen();
        cout << GREEN << "Thanks for playing! 🍏\n" << RESET;
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}

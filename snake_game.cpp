#include <iostream>
#include <deque>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

// Platform-specific headers
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    #define SLEEP(ms) Sleep(ms)
#else
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #define SLEEP(ms) usleep((ms) * 1000)
#endif

using namespace std;

// Enum for directions
enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// Utility function to clear screen (only called once at start)
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Set cursor position to avoid flickering
void setCursorPosition(int x, int y) {
    #ifdef _WIN32
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    #else
        printf("\033[%d;%dH", y + 1, x + 1);
        fflush(stdout);
    #endif
}

// Hide cursor to avoid flickering
void hideCursor() {
    #ifdef _WIN32
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = FALSE;
        SetConsoleCursorInfo(consoleHandle, &info);
    #else
        printf("\e[?25l");
        fflush(stdout);
    #endif
}

// Show cursor
void showCursor() {
    #ifdef _WIN32
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = TRUE;
        SetConsoleCursorInfo(consoleHandle, &info);
    #else
        printf("\e[?25h");
        fflush(stdout);
    #endif
}

// Class representing a position
class Position {
public:
    int row;
    int col;
    
    Position(int r = 0, int c = 0) : row(r), col(c) {}
    
    bool operator==(const Position& other) const {
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
    
    Food(Position pos) : position(pos), symbol('O') {}
    
    Position getPosition() const {
        return position;
    }
    
    void setPosition(Position pos) {
        position = pos;
    }
    
    char getSymbol() const {
        return symbol;
    }
};

// Snake class
class Snake {
private:
    deque<Position> body;
    Direction direction;
    bool growing;
    char headSymbol;
    char bodySymbol;
    
public:
    Snake(Position startPos, int length = 3) 
        : direction(RIGHT), growing(false), headSymbol('#'), bodySymbol('o') {
        // Initialize snake body (horizontal line)
        for (int i = 0; i < length; i++) {
            body.push_back(Position(startPos.row, startPos.col - i));
        }
    }
    
    Position getHead() const {
        return body.front();
    }
    
    deque<Position> getBody() const {
        return body;
    }
    
    Direction getDirection() const {
        return direction;
    }
    
    void setDirection(Direction newDir) {
        // Prevent moving in opposite direction
        if ((direction == UP && newDir == DOWN) ||
            (direction == DOWN && newDir == UP) ||
            (direction == LEFT && newDir == RIGHT) ||
            (direction == RIGHT && newDir == LEFT)) {
            return;
        }
        direction = newDir;
    }
    
    void move() {
        Position head = getHead();
        Position newHead = head;
        
        // Calculate new head position based on direction
        switch (direction) {
            case UP:    newHead.row--; break;
            case DOWN:  newHead.row++; break;
            case LEFT:  newHead.col--; break;
            case RIGHT: newHead.col++; break;
        }
        
        body.push_front(newHead);
        
        if (!growing) {
            body.pop_back();
        } else {
            growing = false;
        }
    }
    
    void grow() {
        growing = true;
    }
    
    bool checkSelfCollision() const {
        Position head = getHead();
        for (size_t i = 1; i < body.size(); i++) {
            if (body[i] == head) {
                return true;
            }
        }
        return false;
    }
    
    char getHeadSymbol() const { return headSymbol; }
    char getBodySymbol() const { return bodySymbol; }
};

// GameBoard class
class GameBoard {
private:
    int rows;
    int cols;
    Snake* snake;
    Food* food;
    int score;
    int highScore;
    bool gameOver;
    vector<vector<char>> previousBoard;
    
    void spawnFood() {
        // Get snake positions
        deque<Position> snakeBody = snake->getBody();
        vector<Position> availablePositions;
        
        // Find available positions
        for (int r = 1; r < rows - 1; r++) {
            for (int c = 1; c < cols - 1; c++) {
                Position pos(r, c);
                bool occupied = false;
                
                for (const Position& bodyPart : snakeBody) {
                    if (bodyPart == pos) {
                        occupied = true;
                        break;
                    }
                }
                
                if (!occupied) {
                    availablePositions.push_back(pos);
                }
            }
        }
        
        if (!availablePositions.empty()) {
            int randomIndex = rand() % availablePositions.size();
            food->setPosition(availablePositions[randomIndex]);
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
    
    ~GameBoard() {
        delete snake;
        delete food;
    }
    
    bool checkCollision() {
        Position head = snake->getHead();
        
        // Boundary collision
        if (head.row <= 0 || head.row >= rows - 1 ||
            head.col <= 0 || head.col >= cols - 1) {
            return true;
        }
        
        // Self collision
        if (snake->checkSelfCollision()) {
            return true;
        }
        
        return false;
    }
    
    bool checkFoodCollision() {
        if (snake->getHead() == food->getPosition()) {
            snake->grow();
            score += 10;
            spawnFood();
            return true;  // Food was eaten
        }
        return false;
    }
    
    void update() {
        if (!gameOver) {
            snake->move();
            
            if (checkCollision()) {
                gameOver = true;
                if (score > highScore) {
                    highScore = score;
                }
            } else {
                checkFoodCollision();
            }
        }
    }
    
    bool didEatFood() {
        // Check if food was just eaten
        if (!gameOver) {
            return checkFoodCollision();
        }
        return false;
    }
    
    void renderInitial() {
        // First time render - draw everything
        clearScreen();
        hideCursor();
        
        // Create board
        vector<vector<char>> board(rows, vector<char>(cols, ' '));
        
        // Draw boundaries
        for (int i = 0; i < cols; i++) {
            board[0][i] = '=';
            board[rows - 1][i] = '=';
        }
        for (int i = 0; i < rows; i++) {
            board[i][0] = '|';
            board[i][cols - 1] = '|';
        }
        board[0][0] = '+';
        board[0][cols - 1] = '+';
        board[rows - 1][0] = '+';
        board[rows - 1][cols - 1] = '+';
        
        // Print board
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                cout << board[r][c];
            }
            cout << endl;
        }
        
        cout << "\nScore: 0  |  High Score: 0  |  Length: 3" << endl;
        cout << "Controls: W/A/S/D or Arrow Keys  |  Q: Quit" << endl;
        
        previousBoard = board;
    }
    
    void render() {
        // Create current board state - start fresh from boundaries
        vector<vector<char>> board(rows, vector<char>(cols, ' '));
        
        // Draw boundaries
        for (int i = 0; i < cols; i++) {
            board[0][i] = '=';
            board[rows - 1][i] = '=';
        }
        for (int i = 0; i < rows; i++) {
            board[i][0] = '|';
            board[i][cols - 1] = '|';
        }
        board[0][0] = '+';
        board[0][cols - 1] = '+';
        board[rows - 1][0] = '+';
        board[rows - 1][cols - 1] = '+';
        
        // Draw food
        Position foodPos = food->getPosition();
        board[foodPos.row][foodPos.col] = food->getSymbol();
        
        // Draw snake
        deque<Position> snakeBody = snake->getBody();
        for (size_t i = 0; i < snakeBody.size(); i++) {
            Position pos = snakeBody[i];
            if (i == 0) {
                board[pos.row][pos.col] = snake->getHeadSymbol();
            } else {
                board[pos.row][pos.col] = snake->getBodySymbol();
            }
        }
        
        // Only update changed positions
        for (int r = 1; r < rows - 1; r++) {
            for (int c = 1; c < cols - 1; c++) {
                if (board[r][c] != previousBoard[r][c]) {
                    setCursorPosition(c, r);
                    cout << board[r][c];
                    cout.flush();
                }
            }
        }
        
        // Update score display
        setCursorPosition(0, rows);
        cout << "Score: " << score << "  |  High Score: " << highScore 
             << "  |  Length: " << snakeBody.size() << "   ";
        cout.flush();
        
        previousBoard = board;
    }
    
    Snake* getSnake() { return snake; }
    bool isGameOver() const { return gameOver; }
    int getScore() const { return score; }
    int getHighScore() const { return highScore; }
    int getSnakeLength() const { return snake->getBody().size(); }
};

// InputHandler class
class InputHandler {
private:
    #ifndef _WIN32
        struct termios oldSettings;
    #endif
    
public:
    InputHandler() {
        #ifndef _WIN32
            tcgetattr(STDIN_FILENO, &oldSettings);
        #endif
    }
    
    ~InputHandler() {
        cleanup();
    }
    
    char getKey() {
        #ifdef _WIN32
            if (_kbhit()) {
                char key = _getch();
                if (key == -32 || key == 0) {  // Arrow key prefix
                    key = _getch();
                    switch (key) {
                        case 72: return 'U';  // UP
                        case 80: return 'D';  // DOWN
                        case 75: return 'L';  // LEFT
                        case 77: return 'R';  // RIGHT
                    }
                }
                return toupper(key);
            }
            return '\0';
        #else
            struct termios newSettings;
            tcgetattr(STDIN_FILENO, &newSettings);
            newSettings.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
            
            int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
            
            char ch = '\0';
            if (read(STDIN_FILENO, &ch, 1) > 0) {
                if (ch == 27) {  // Escape sequence
                    char seq[2];
                    if (read(STDIN_FILENO, &seq[0], 1) > 0 && 
                        read(STDIN_FILENO, &seq[1], 1) > 0) {
                        if (seq[0] == '[') {
                            switch (seq[1]) {
                                case 'A': return 'U';  // UP
                                case 'B': return 'D';  // DOWN
                                case 'D': return 'L';  // LEFT
                                case 'C': return 'R';  // RIGHT
                            }
                        }
                    }
                }
                ch = toupper(ch);
            }
            
            tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
            fcntl(STDIN_FILENO, F_SETFL, oldf);
            
            return ch;
        #endif
    }
    
    void cleanup() {
        #ifndef _WIN32
            tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
        #endif
    }
};

// Game class
class Game {
private:
    GameBoard* board;
    InputHandler* inputHandler;
    bool running;
    int speed;  // milliseconds per frame
    
public:
    Game() : board(NULL), running(true), speed(100) {
        inputHandler = new InputHandler();
    }
    
    ~Game() {
        if (board) delete board;
        delete inputHandler;
        showCursor();
    }
    
    void showMenu() {
        clearScreen();
        cout << "+=======================================+" << endl;
        cout << "|     SNAKE GAME - IT603 Project       |" << endl;
        cout << "+=======================================+" << endl;
        cout << "\n  Controls:" << endl;
        cout << "    W or UP    : Move Up" << endl;
        cout << "    A or LEFT  : Move Left" << endl;
        cout << "    S or DOWN  : Move Down" << endl;
        cout << "    D or RIGHT : Move Right" << endl;
        cout << "    Q          : Quit Game" << endl;
        cout << "\n  Objective:" << endl;
        cout << "    * Eat food (O) to grow and score points" << endl;
        cout << "    * Avoid hitting walls and yourself" << endl;
        cout << "    * Try to beat your high score!" << endl;
        cout << "\n-----------------------------------------" << endl;
        cout << "\n  Press ENTER to start...";
        cin.ignore();
        cin.get();
    }
    
    bool showGameOver() {
        showCursor();
        clearScreen();
        cout << "\n+=======================================+" << endl;
        cout << "|            GAME OVER!                 |" << endl;
        cout << "+=======================================+" << endl;
        cout << "\n  Final Score: " << board->getScore() << endl;
        cout << "  High Score:  " << board->getHighScore() << endl;
        cout << "  Snake Length: " << board->getSnakeLength() << endl;
        cout << "\n-----------------------------------------" << endl;
        cout << "\n  Options:" << endl;
        cout << "    R : Restart Game" << endl;
        cout << "    Q : Quit to Exit" << endl;
        
        while (true) {
            char key = inputHandler->getKey();
            if (key == 'R') {
                return true;
            } else if (key == 'Q') {
                return false;
            }
            SLEEP(50);
        }
    }
    
    void run() {
        showMenu();
        
        while (running) {
            // Initialize new game
            if (board) delete board;
            board = new GameBoard(20, 40);
            
            // Initial render
            board->renderInitial();
            
            // Game loop
            while (!board->isGameOver()) {
                board->render();
                
                // Handle input
                char key = inputHandler->getKey();
                if (key == 'W' || key == 'U') {
                    board->getSnake()->setDirection(UP);
                } else if (key == 'S' || key == 'D') {
                    board->getSnake()->setDirection(DOWN);
                } else if (key == 'A' || key == 'L') {
                    board->getSnake()->setDirection(LEFT);
                } else if (key == 'D' || key == 'R') {
                    board->getSnake()->setDirection(RIGHT);
                } else if (key == 'Q') {
                    running = false;
                    break;
                }
                
                board->update();
                SLEEP(speed);
            }
            
            if (board->isGameOver()) {
                bool restart = showGameOver();
                if (!restart) {
                    running = false;
                }
            }
        }
        
        showCursor();
        clearScreen();
        cout << "Thanks for playing! Goodbye!" << endl;
    }
};

// Main function
int main() {
    Game game;
    game.run();
    return 0;
}
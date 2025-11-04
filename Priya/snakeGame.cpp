#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

using namespace std;

// Emojis
string HEAD = "🦎";
string BODY = "🟢";
vector<string> FRUITS = {"🍎","🍌","🍉","🍇","🍓","🍒","🍑","🍊","🍍","🍐","🍈","🍋"};
string EMPTY = "⬜";

// Board size
const int ROWS = 10;
const int COLS = 10;

// Snake state
vector<pair<int, int>> snake = {{5, 5}};
pair<int, int> food;
string fruit;
string direction = "RIGHT";

bool kbhit() {
    termios term;
    tcgetattr(0, &term);

    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int bytesWaiting;
    ioctl(0, FIONREAD, &bytesWaiting);

    tcsetattr(0, TCSANOW, &term);
    return bytesWaiting > 0;
}

char getch() {
    char buf = 0;
    termios old = {0};
    tcgetattr(0, &old);
    old.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &old);
    read(0, &buf, 1);
    tcsetattr(0, TCSANOW, &old);
    return buf;
}

void spawnFood() {
    food.first = rand() % ROWS;
    food.second = rand() % COLS;
    fruit = FRUITS[rand() % FRUITS.size()];
}

void printBoard() {
    system("clear");
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            bool printed = false;

            if (r == snake[0].first && c == snake[0].second) {
                cout << HEAD;
                printed = true;
            } else {
                for (size_t i = 1; i < snake.size(); ++i) {
                    if (r == snake[i].first && c == snake[i].second) {
                        cout << BODY;
                        printed = true;
                        break;
                    }
                }
            }

            if (!printed) {
                if (r == food.first && c == food.second) cout << fruit;
                else cout << EMPTY;
            }
        }
        cout << endl;
    }
    cout << "\nControls: W ↑ | S ↓ | A ← | D → | Ctrl+C to exit\n";
    cout << "Length: " << snake.size() << endl;
}

void moveSnake() {
    pair<int, int> head = snake[0];
    if (direction == "UP") head.first = (head.first - 1 + ROWS) % ROWS;
    else if (direction == "DOWN") head.first = (head.first + 1) % ROWS;
    else if (direction == "LEFT") head.second = (head.second - 1 + COLS) % COLS;
    else if (direction == "RIGHT") head.second = (head.second + 1) % COLS;

    // Check if food is eaten
    if (head == food) {
        snake.insert(snake.begin(), head); // Grow
        spawnFood();
    } else {
        snake.insert(snake.begin(), head);
        snake.pop_back();
    }
}

int main() {
    srand(time(0));
    spawnFood();

    while (true) {
        if (kbhit()) {
            char key = getch();
            key = tolower(key);
            if (key == 'w') direction = "UP";
            else if (key == 's') direction = "DOWN";
            else if (key == 'a') direction = "LEFT";
            else if (key == 'd') direction = "RIGHT";
        }

        moveSnake();
        printBoard();
        usleep(200000); // speed (microseconds) → 200000 = 0.2s
    }

    return 0;
}

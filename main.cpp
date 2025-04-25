#include <iostream>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;
termios initial_termios;

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &initial_termios) == -1) {
        perror("tcgetattr");
        exit(1);
    }

    termios raw = initial_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &initial_termios) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

int main() {
    enableRawMode();
    vector<string> text = {"Hello, Bento!"};
    int cursorRow = 1;
    int cursorCol = 1;

    if (text.size()) {
        cursorCol = text[cursorRow-1].size()+1;
    }

    // Clear screen and set cursor to top-left
    cout << "\033[2J\033[1;1H";
    for (int i = 0; i < text.size(); i++) {
        cout << text[i] << endl;
    }
    cout << "\033[" << cursorRow << ";" << cursorCol << "H";
    cout.flush();

    while (true) {
        char input;
        if (read(STDIN_FILENO, &input, 1) == -1) {
            perror("read");
            break;
        }

        if (input == '\033') {
            char next1, next2;
            if (read(STDIN_FILENO, &next1, 1) == 1 && read(STDIN_FILENO, &next2, 1) == 1) {
                if (next1 == '[') {
                    if (next2 == 'A') {
                        cursorRow = std::max(1, cursorRow - 1);
                    } else if (next2 == 'B') {
                        // We'd need to handle scrolling or boundaries in a real editor
                        cursorRow++;
                    } else if (next2 == 'C') {
                        if (cursorCol == text[cursorRow-1].size()+1) {
                            if (cursorRow < text.size()) {
                                cursorCol = 1;
                                cursorRow++;
                            }
                        } else {
                            cursorCol++;
                        }
                    } else if (next2 == 'D') {
                        if (cursorCol == 1) {
                            if (cursorRow != 1) {
                                cursorRow--;
                                cursorCol = text[cursorRow-1].size()+1;
                            }
                        } else {
                            cursorCol--;
                        }
                    }
                }
            }
        } else if (input == '\n') {      
            if (cursorCol == text[cursorRow-1].size()+1) {
                string s = "";
                text.insert(text.begin()+cursorRow, s);
                cursorRow++;
                cursorCol = 1;
            } else {
                string s1 = "";
                string s2 = "";

                for(int i = 0; i < text[cursorRow-1].size(); i++) {
                    if (i < cursorCol-1) {
                        s1.push_back(text[cursorRow-1][i]);
                    } else {
                        s2.push_back(text[cursorRow-1][i]);
                    }
                }
                text[cursorRow-1] = s1;
                text.insert(text.begin()+cursorRow, s2);
                cursorRow++;
                cursorCol = 1;
            }
        } else {
            string s = "";
            s += input;

            if (cursorCol > text[cursorRow-1].size()) {
                text[cursorRow-1] += s;
            } else {
                text[cursorRow-1].insert(cursorCol-1, s);
            }

            cursorCol++;
        }

        cout << "\033[2J\033[1;1H";
        for (int i = 0; i < text.size(); i++) {
            cout << text[i] << endl;
        }
        cout << "\033[" << cursorRow << ";" << cursorCol << "H";
        cout.flush();
    }

    disableRawMode();
    return 0;
}
#include <iostream>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <regex>
#include <map>
#include <cctype>
#include <set>

using namespace std;
termios initial_termios;

int WORD_SUGGESTED = 0;
string WORD_SUGGESTION = "";

map<string, string> keywordColors = {
    {"int", "\033[1;34m"},
    {"char", "\033[1;34m"},
    {"double", "\033[1;34m"},
    {"#include", "\033[1;32m"},
    {"namespace", "\033[1;32m"},
    {"return", "\033[1;33m"},
    {"if", "\033[1;35m"},
    {"else", "\033[1;35m"},
    {"else if", "\033[1;35m"},
    {"while", "\033[1;35m"},
    {"for", "\033[1;35m"}
};

vector<int> computeLPSArray(const string& pattern) {
    int m = pattern.length();
    vector<int> lps(m);
    int len = 0;
    int i = 1;

    lps[0] = 0;
    while (i < m) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else { 
            if (len != 0) {
                len = lps[len - 1];
            } else { 
                lps[i] = 0;
                i++;
            }
        }
    }
    return lps;
}

bool isWordBoundary(const string& text, int index) {
    if (index < 0 || index >= text.length()) {
        return true;
    }
    return !isalnum(text[index]);
}

int kmpSearchWholeWord(const string& text, const string& pattern) {
    int n = text.length();
    int m = pattern.length();
    if (m == 0) return 0;

    vector<int> lps = computeLPSArray(pattern);
    int i = 0;
    int j = 0;

    while (i < n) {
        if (pattern[j] == text[i]) {
            i++;
            j++;
        }
        if (j == m) {
            int matchIndex = i - j;
            if (isWordBoundary(text, matchIndex - 1) && isWordBoundary(text, i)) {
                return matchIndex;
            } else {
                i++;
                j = 0;
            }
        } else if (i < n && pattern[j] != text[i]) {
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }
    return -1;
}

int kmpSearch(const string& text, const string& pattern) {
    int n = text.length();
    int m = pattern.length();
    if (m == 0) return 0;

    vector<int> lps = computeLPSArray(pattern);
    int i = 0; 
    int j = 0;

    while (i < n) {
        if (pattern[j] == text[i]) {
            i++;
            j++;
        }
        if (j == m) {
            return (i - j);
        } else if (i < n && pattern[j] != text[i]) {
            
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }
    return -1;
}

void replaceFirstOccurrenceInPlace(string& text, const string& pattern, const string& replacement) {
    int index = kmpSearchWholeWord(text, pattern);

    if (index != -1) {
        text.erase(index, pattern.length());
        text.insert(index, replacement);
    }
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &initial_termios) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

void drawText(vector<string> text, int cursorRow, int cursorCol, set<string>& suggestions) {

    //vector<string> suggestions = {"variableName", "variaableName2222"};

    cout << "\033[" << cursorRow << ";1H";
    cout << "\033[2K"; 

    if (cursorRow >= 1 && cursorRow <= text.size()) {

        string highlighted_line = text[cursorRow-1];

        string currentWord = "";
        int wordStart = max(cursorCol-2, 0);
        int wordEnd = cursorCol-1;

        for(wordStart; (wordStart != 0 && isalnum(text[cursorRow-1][wordStart])); wordStart--) {
            //cout << "epic" << endl;
            continue;
        }

        if (!isalnum(text[cursorRow-1][wordStart])) {
            wordStart++;
        }

        for(wordEnd; (wordEnd != text[cursorRow-1].size()-1 && isalnum(text[cursorRow-1][wordEnd])); wordEnd--) {
            continue;
        }

        if (!isalnum(text[cursorRow-1][wordEnd])) {
            wordEnd--;
        }

        if (wordEnd - wordStart >= 3) {
            //cout << "lol" << endl;
            int writtenSize = 0;
            WORD_SUGGESTION = "";
            for (int i = wordStart; i <= wordEnd; i++) {
                writtenSize++;
                currentWord += text[cursorRow-1][i];
            }

            for(set<string>::iterator it = suggestions.begin(); it != suggestions.end(); it++) {
                
                if (kmpSearch(*it, currentWord) == 0 && currentWord != *it) {
                    for(int i = writtenSize; i < (*it).size(); i++) {
                        WORD_SUGGESTION += (*it)[i];
                    }

                    highlighted_line.insert(cursorCol-1, "\033[90m" + WORD_SUGGESTION + "\033[0m");

                    WORD_SUGGESTED = 1;
                    break;
                }
            }

        }

        for (map<string, string>::iterator it = keywordColors.begin(); it != keywordColors.end(); it++) {

            //regex keywordRegex(it->first);
            //highlighted_line = regex_replace(highlighted_line, keywordRegex, it->second + "$1" + "\033[0m");
            replaceFirstOccurrenceInPlace(highlighted_line, it->first, it->second + it->first + "\033[0m");
        }
        
        cout << highlighted_line;
    }

    cout << "\033[" << cursorRow << ";" << cursorCol << "H";
    
}

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

// int main() {
//     enableRawMode();

//     string t = "int main";
//     string p = "int";
//     string k = "\033[1;34m";
//     replaceFirstOccurrenceInPlace(t, p, k + p + "\033[0m");
//     cout << t << endl;

//     disableRawMode();
//     return 0;
// }

int calcIndentLevel(int prevIndentLevel, string currentLine) {
    int newIndentLevel = prevIndentLevel;
    int indentReduced = 0;
    int indentIncreased = 0;
    int i = 0;

    for(i; i < currentLine.size(); i++) {
        if (currentLine[i] != ' ') {
            break;
        }

        // if (currentLine[i] == '}' && !indentReduced) {
        //     newIndentLevel -= 1;
        //     indentReduced = 1;
        // } else if (currentLine[i] == '{' && !indentIncreased) {
        //     newIndentLevel += 1;
        //     indentIncreased = 1;
        // }

        // if (indentIncreased && indentReduced) {
        //     break;
        // }
    }
    newIndentLevel = (i+1) / 4;
    return newIndentLevel;
}


void findVariableNames(const string& text, set<string>& variableNames) {
    regex variablePattern(R"(\b(?:int|float|double|char|long|short|bool)?\s*(\w+)\s*=\s*[^;]+;)");

    sregex_iterator begin(text.begin(), text.end(), variablePattern);
    sregex_iterator end;

    for (sregex_iterator it = begin; it != end; ++it) {
        smatch match = *it;
        variableNames.insert(match[1]); 
    }
}


int main() {
    enableRawMode();
    vector<string> text = {""};
    set<string> variableNames = {};
    int cursorRow = 1;
    int cursorCol = 1;
    int indentLevel = 0;
    if (text.size()) {
        cursorCol = text[cursorRow-1].size()+1;
    }

    cout << "\033[2J\033[1;1H";
    regex keywords("\\b(int|char)\\b");
    string highlight_start = "\033[1;34m";
    string highlight_end = "\033[0m";   
    for (int i = 0; i < text.size(); i++) {
        string highlighted_line = regex_replace(text[i], keywords, highlight_start + "$1" + highlight_end);
        cout << highlighted_line << endl;

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
                        cursorCol = min(cursorCol, (int)(text[cursorRow-1].size()+1));
                    } else if (next2 == 'B') {

                        if (cursorRow < text.size()) {
                            cursorRow++;
                            cursorCol = min(cursorCol, (int)(text[cursorRow-1].size()+1));
                        }    
                                            
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

                indentLevel = calcIndentLevel(indentLevel, text[cursorRow-1]);
            }
        } else if (input == '\n') {      
            if (cursorCol == text[cursorRow-1].size()+1) {

                string s = "";
                
                for (int indexIterator = 0; indexIterator < indentLevel; indexIterator++) {
                    s += "    ";
                }

                text.insert(text.begin()+cursorRow, s);
                

                for (int i = cursorRow-1; i <= text.size(); i++) {
                    drawText(text, i, cursorCol, variableNames);
                    cout.flush();
                }

                cursorRow++;
                cursorCol = indentLevel*4 + 1;

                drawText(text, cursorRow, cursorCol, variableNames);
                cout.flush();

            } else {
                string s1 = "";
                string s2 = "";

                for (int indexIterator = 0; indexIterator < indentLevel; indexIterator++) {
                    s2 += "    ";
                }

                for(int i = 0; i < text[cursorRow-1].size(); i++) {
                    if (i < cursorCol-1) {
                        s1.push_back(text[cursorRow-1][i]);
                    } else {
                        s2.push_back(text[cursorRow-1][i]);
                    }
                }

                if (s1[s1.size()-1] == '{' && s2[indentLevel*4] == '}') {
                    indentLevel++;
                    string sMid = "";

                    for (int indexIterator = 0; indexIterator < indentLevel; indexIterator++) {
                        sMid += "    ";
                    }

                    text[cursorRow-1] = s1;
                    text.insert(text.begin()+cursorRow, sMid);
                    text.insert(text.begin()+cursorRow+1, s2);


                    for (int i = cursorRow-1; i <= text.size(); i++) {
                        drawText(text, i, cursorCol, variableNames);
                        cout.flush();
                    }

                    cursorRow++;
                    cursorCol = indentLevel*4 + 1;

                    drawText(text, cursorRow, cursorCol, variableNames);
                    cout.flush();
                } else {

                    text[cursorRow-1] = s1;
                    text.insert(text.begin()+cursorRow, s2);
                    cursorRow++;
                    cursorCol = 1;

                    for (int i = cursorRow-1; i <= text.size(); i++) {
                        drawText(text, i, cursorCol, variableNames);
                        cout.flush();
                    }
                }

                // drawText(text, cursorRow-1, cursorCol);
                // cout.flush();
                // drawText(text, cursorRow, cursorCol);
                // cout.flush();
                continue;
            }
        } else if (input == 127 || input == 8) {
            if (cursorCol > 1) {
                text[cursorRow - 1].erase(cursorCol - 2, 1);
                cursorCol--;
            } else {
                if (cursorRow > 1) {
                    cursorCol = text[cursorRow-2].size()+1;
                    text[cursorRow-2] += text[cursorRow-1];
                    text.erase(text.begin()+cursorRow-1);
                    cursorRow--;
                    drawText(text, cursorRow, cursorCol, variableNames);
                    cout.flush();
                    drawText(text, cursorRow+1, cursorCol, variableNames);
                    cout.flush();
                    continue;
                }
            }
        } else if (input == 9) {
            if (WORD_SUGGESTED) {
                text[cursorRow-1].insert(cursorCol-1, WORD_SUGGESTION);
                cursorCol += WORD_SUGGESTION.size();
            } else {
                int i = 0;

                for (i; i < text[cursorRow-1].size(); i++) {
                    if (text[cursorRow-1][i] != ' ') {
                        break;
                    }
                }

                int numShifts = 4 - (i % 4); 

                for (int j = 0; j < numShifts; j++) {
                    text[cursorRow-1].insert(text[cursorRow-1].begin(), ' ');
                }

                cursorCol += numShifts;
            }
        } else if (input == '=') {
            string s = "";
            s += input;

            if (cursorCol > text[cursorRow-1].size()) {
                text[cursorRow-1] += s;
            } else {
                text[cursorRow-1].insert(cursorCol-1, s);
            }

            findVariableNames(text[cursorRow-1], variableNames);

            cout << "LOL" << endl;

            for(set<string>::iterator it = variableNames.begin(); it != variableNames.end(); it++) {
                cout <<   *it << endl;
            }

            

            cursorCol++;
        } else if (input == '{') {
            string s = "";
            s += input;
            s += "}";

            if (cursorCol > text[cursorRow-1].size()) {
                text[cursorRow-1] += s;
            } else {
                text[cursorRow-1].insert(cursorCol-1, s);
            }

            cursorCol++;

        } else if (input == '}') {
            if (cursorCol > 1 && text[cursorRow-1][cursorCol-1] == '}') {
                cursorCol++;
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
        } else if (input == '(') {

            string s = "";
            s += input;
            s += ")";

            if (cursorCol > text[cursorRow-1].size()) {
                text[cursorRow-1] += s;
            } else {
                text[cursorRow-1].insert(cursorCol-1, s);
            }

            cursorCol++;

        } else if (input == ')') {
            if (cursorCol > 1 && text[cursorRow-1][cursorCol-1] == ')') {
                cursorCol++;
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

        WORD_SUGGESTED = 0;
        drawText(text, cursorRow, cursorCol, variableNames);
        
        

        cout.flush();
    }

    disableRawMode();
    return 0;
}
#include <iostream>

using namespace std;

int main() {
    int firstVariable = 5;
    
    for(firstVariable; firstVariable < 25; firstVariable++) {
        int secondVariable = 1;
        for(secondVariable; secondVariable < 6; secondVariable++) {
            cout << firstVariable << " " << secondVariable << endl;
        }
    }
}

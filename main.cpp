
#include <iostream>
#include <string>
#include <fstream>
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#endif

using namespace std;

// node
struct Node {
    string data;
    Node* next;
};
typedef Node* NodePtr;

void clearScreen() {
    #ifdef _WIN32
        system("cls");   
    #else
        system("clear"); 
    #endif
}

// UI functions

void showTitleScreen() {
    cout << " _                 _         _                   _" << endl;
    cout << "|  _ \\ _   _ _  _| |  / ___| _ _  _ _() _   _ _" << endl;
    cout << "| |_) / _ \\ / _ |/ _ | | |   | '_/ _ \\/ __/ __| | ' \\ / _` |" << endl;
    cout << "|  _ < () | (| | (_| | | |___| | | () \\_ \\__ \\ | | | | (_| |" << endl;
    cout << "|_| \\_\\___/ \\__,_|\\__,_|  \\____|_|  \\___/|___/___/_|_| |_|\\__, |" << endl;
    cout << "              C h a l l e n g e                        |___/" << endl << endl;
    cout << "Written for CSDC102 | Language: C++" << endl << endl;
    cout << "HOW TO PLAY:" << endl;
    cout << "- Move with Arrow Keys" << endl;
    cout << "- Dodge trucks (#####) in the ROAD ZONE" << endl;
    cout << "- Hop on logs (====) in the RIVER ZONE" << endl;
    cout << "- Reach the finish line 5 times to win!" << endl << endl;
    cout << "Press ENTER to start..." << endl;
    
    cin.get();
}
    
// get player name
string getPlayerName() {
    string name;
    clearScreen();
    
    cout << "Enter player name: ";
    getline(cin, name);
    return name;
}

// difficulty
int chooseDifficulty() {
    int diff;
    clearScreen();
    
    cout << "SELECT DIFFICULTY" << endl;
    cout << "1. Easy  — speed: 180ms, 2 trucks/logs per lane" << endl;
    cout << "2. Hard  — speed: 100ms, 3 trucks/logs per lane" << endl << endl;
    cout << "Enter choice (1 or 2): ";
    cin >> diff;
    return diff;
}

// lane generator (static for now)
string generateTerrain(char laneType) {
    string lane(42, ' ');

    lane[0] = '|';
    lane[41] = '|';

    if(laneType == 'F') {
        for(int i = 1; i <= 40; i++){
            lane[i] = '=';
        }
    }

    else if(laneType == 'B' || laneType == 'S'){

        for(int i = 1; i <= 40; i++){
            lane[i] = '.';
        }
    }

    else if(laneType == 'R'){

        for(int i = 1; i <= 40; i++){
            lane[i] = '.';
        }

        int trucks = 2 + rand() % 2;

        int currentPos = 1 + rand() % 40;
        int posEnd = currentPos + 4;

        for(int t = 0; t < trucks; t++){ //loops for the amount of truck
            int truckCount = 0; //tracks which part of the truck it is

            for(int j = 0; j < 5; j++){
                if(currentPos + j <= 40){
                    lane[currentPos + truckCount] = '#';
                }
                else{
                    currentPos = 1, truckCount = 0;
                    lane[currentPos + truckCount] = '#';
                }
                truckCount++;
            }

            posEnd = currentPos + truckCount;
            currentPos = posEnd + (3 + rand() % 5);
        }
    }

    else if(laneType == 'V'){

        for(int i = 1; i <= 40; i++){
            lane[i] = '~';
        }

        int logs = 2;

        int currentPos = 1 + rand() % 40;
        int posEnd = currentPos + 4;

        for(int t = 0; t < logs; t++){ //loops for the amount of log
            int logCount = 0; //tracks which part of the log it is
            
            for(int j = 0; j < 4; j++){
                if(currentPos + j <= 40){
                    lane[currentPos + logCount] = '=';
                }
                else{
                    currentPos = 1, logCount = 0;
                    lane[currentPos + logCount] = '=';
                }
                logCount++;
            }

            posEnd = currentPos + logCount;
            currentPos = posEnd + (10 + rand() % 10);
        }
    }

    return lane;
}

///////////////////////////////////// core game functions


//shifting function

//shift inner 40 characters left, wrap the first character to the end
string shiftLeft(const string& lane) {
    string result = lane;
    char warp = result[1]; // store the first character for wrapping
    for (int i = 1; i < 40; i++) result[i] = result[i + 1];
    result[40] = warp; // wrap the first character to the end
    return result;
}

//shift inner 40 characters right, wrap the last character to the front
string shiftRight(const string& lane) {
    string result = lane;
    char warp = result[40]; // store the last character for wrapping
    for (int i = 40; i > 1; i--) result[i] = result[i - 1];
    result[1] = warp; // wrap the last character to the front
    return result;
}

//shifting function end


NodePtr buildRoad() {
    NodePtr head = NULL;
    NodePtr tail = NULL;

    char zoneMap[20] = {

            'F',

            'R','R','R','R','R',
            'B',
            'V','V',
            'B',

            'R','R','R','R','R',
            'B',
            'V','V',
            'B',

            'S'
        };

    for (int i = 0; i < 20; i++) {
        NodePtr newNode = new Node;
        newNode->next = NULL;

        newNode->data = generateTerrain(zoneMap[i]);

        // linked list
        if (head == NULL) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }
    return head;
}

// traverse linked list, print the board, overlay the player
void displayRoad(NodePtr head, string playerName, int playerX, int playerY, int lives, int score) {
    NodePtr curr = head;
    int row = 0;
    
    cout << "---------- Road Crossing Challenge ----------" << endl;
    cout << "Player: " << playerName << " | Lives: " << lives << " | Crossings: " << score   << endl;
    while (curr != NULL) {
        string rowData = curr->data;
        
        if (row == playerY) {
            rowData[playerX] = 'P';
        }
        
        cout << rowData << endl;
        curr = curr->next;
        row++;
    }
}

// pevent memory leaks
void freeList(NodePtr& head) {
    NodePtr temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        delete temp;
    }
    head = NULL;   // now this actually sets the original to NULL
}

//zone map - mirrors the same order as the buildRoad function for easy reference
//used by collision and shift function to know what the row contains 

const char 
ZONE_MAP[20] = {
    'F', // finish line

    'R','R','R','R','R', // river zone
    'B', // safe zone
    'V','V', // river zone with logs
    'B', // safe zone

    'R','R','R','R','R', // river zone
    'B', // safe zone
    'V','V', // river zone with logs
    'B', // safe zone

    'S'  // starting line
};

// returns pointer to node at position 'index'
NodePtr getLane(NodePtr head, int index) {
    NodePtr curr = head;
    for (int i = 0; i < index && curr != NULL; i++) {
        curr = curr->next;
    }
    return curr;
}

//shift obstacle function
void shiftObstacles(NodePtr head) {
    int roadNum = 0;
    int riverNum = 0;
    NodePtr curr = head;

    for (int i = 0; i < 20 && curr != NULL; i++, curr = curr->next) {

        if (ZONE_MAP[i] == 'R') {
            roadNum++;
            curr->data = (roadNum % 2 == 1)
            ? shiftRight(curr->data)
            : shiftLeft(curr->data);
        }

        else if (ZONE_MAP[i] == 'V') {
            riverNum++;
            curr->data = (riverNum % 2 == 1)
            ? shiftRight(curr->data)
            : shiftLeft(curr->data);
        }
    }
}

//check collision and drowned function

bool checkCollision(NodePtr head, int playerX, int playerY) {
    if (ZONE_MAP[playerY] != 'R') return false; // only check for trucks in road zones
    NodePtr lane = getLane(head, playerY);
    if (lane == NULL) return false; // safety check
    return (lane->data[playerX] == '#'); // collision if player is on a truck
}

bool checkDrowned(NodePtr head, int playerX, int playerY) {
    if (ZONE_MAP[playerY] != 'V') return false; // only check for logs in river zones
    NodePtr lane = getLane(head, playerY);
    if (lane == NULL) return false; // safety check
    return (lane->data[playerX] == '~'); //return true if player is in water without a log (drowned)
}


//Leaderboard

struct LeaderboardEntry {
    string name;
    int score;
};

//Save one result to the leaderboard after every game
void saveScore(const string& playerName, int score) {
    ofstream outFile("leaderboard.txt", ios::app);
    if (outFile.is_open()) {
        outFile << playerName << "|" << score << endl;
        outFile.close();
    } else {
        cerr << "Unable to open leaderboard file." << endl;
    }
}

void showLeaderboard() {
    ifstream file("leaderboard.txt");
    LeaderboardEntry entries[100];
    int count = 0;

    if (file.is_open()) {
        string line;
        while (getline(file, line) && count < 100) {
            int sep = line.find('|');
            if (sep == (int)string::npos) continue;  // skip malformed lines
            entries[count].name  = line.substr(0, sep);
            entries[count].score = stoi(line.substr(sep + 1));
            count++;
        }
        file.close();
    }

    // bubble sort and display — rest stays the same
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - 1 - i; j++) {
            if (entries[j].score < entries[j + 1].score) {
                LeaderboardEntry temp = entries[j];
                entries[j]           = entries[j + 1];
                entries[j + 1]       = temp;
            }
        }
    }

    cout << endl;
    cout << " ===== LEADERBOARD (Top 5) =====" << endl;
    int display = (count < 5) ? count : 5;
    for (int i = 0; i < display; i++) {
        cout << "  " << (i + 1) << ". "
            << entries[i].name << " — "
            << entries[i].score << " crossing(s)" << endl;
    }
    if (count == 0) cout << "  (No scores yet.)" << endl;
    cout << " ================================" << endl;
}

// leaderboard end below on how and where to call
// Call saveScore() when the game ends — win or lose
// Then immediately call showLeaderboard() so player sees rankings

//gameover/win function 

void showGameOver(bool won, const string& playerName, int score) {
    clearScreen();
    if (won) {
        cout << "Congratulations " << playerName << "! You crossed 5 times!" << endl;
    } else {
        cout << "Game Over, " << playerName << "! You were hit too many times." << endl;
    }

    //saves to the leaderboard and shows it
    saveScore(playerName, score);

    showLeaderboard();

    cout << "Press ENTER to return to the title screen..." << endl;
    cin.ignore();
    cin.get();
}



//gameover/win endd



/////////////////////////////////// MAIN LOOP

int main() {
    showTitleScreen();
    string pName = getPlayerName();
    int difficulty = chooseDifficulty();

    // player pos
    int playerX = 20; 
    int playerY = 19; 
    int lives = 3; // added to track player lives
    int score = 0; // added to track successful crossings

    // generate linked list map
    NodePtr roadList = buildRoad();

    bool isPlaying = true;
    bool playerWon = false; //added to track win condition

    int gameSpeed = (difficulty == 2) ? 100 : 180; // set game speed based on difficulty
    // main game loops
    while (isPlaying) {

        //shift obstacles every tick
        shiftObstacles(roadList);

        clearScreen();
        displayRoad(roadList, pName, playerX, playerY, lives, score);
        
            //check win condition before starting the game loop
    if (score >= 5) {
        playerWon = true;
        isPlaying = false;
        break;
    }

    //check lose condition before starting the game loop
    if (lives <= 0) {
        playerWon = false;
        isPlaying = false;
        break;
    }

    //check if player reached the finish line (row 0)
    if (playerY == 0) {
        score++; //increment score for successful crossing
        playerY = 19; //reset player to starting position
        freeList(roadList); //free old road
        roadList = buildRoad(); //generate new road
    }

    //collision: truck hit
    if (checkCollision(roadList, playerX, playerY)) {
        lives--; // lose a life if hit by a truck
        playerX = 20; // reset player to starting position
        playerY = 19; // reset player to starting position  
    }

    //collision: drowned
    if (checkDrowned(roadList, playerX, playerY)) { 
        lives--; // lose a life if drowned
        playerX = 20; // reset player to starting position  
        playerY = 19; // reset player to starting position
    }


#ifdef _WIN32
    // Non-blocking input check
    if (_kbhit()) {
        int ch = _getch();
        if (ch == 224) {
            ch = _getch();
            switch (ch) {
                case 72: if (playerY > 0)  playerY--; break;  // UP
                case 80: if (playerY < 19) playerY++; break;  // DOWN
                case 75: if (playerX > 1)  playerX--; break;  // LEFT
                case 77: if (playerX < 40) playerX++; break;  // RIGHT
            }
        } else if (ch == 'q' || ch == 'Q') {
            isPlaying = false;
        }
    }

    Sleep(gameSpeed);   // controls animation speed
    #endif
}

    //show game over/win screen
    showGameOver(playerWon, pName, score);

    freeList(roadList);
    roadList = NULL;

    return 0;
}

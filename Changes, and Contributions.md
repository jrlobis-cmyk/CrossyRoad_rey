# CrossyRoad_rey

# Road Crossing Challenge — Contributor Notes
**Contributor:** Rey  
**Project:** CSDC102 Final Programming Project — Road Crossing Challenge  
**Language:** C++  

---

## What I Worked On

I was responsible for the **leaderboard system**, **collision detection**, **obstacle movement (shifting)**, and several **bug fixes** that were blocking the game from running correctly. Below is a detailed breakdown of every change I made and how each one works.

---

## 1. Leaderboard System

### What I added
Three things: a struct, a save function, and a display function.

```cpp
struct LeaderboardEntry {
    string name;
    int score;
};
```

This struct holds one parsed leaderboard record in memory. It's used when reading the file back in so we can sort entries before displaying them.

---

### `saveScore()` — Writing to the file

```cpp
void saveScore(const string& playerName, int score) {
    ofstream outFile("leaderboard.txt", ios::app);
    if (outFile.is_open()) {
        outFile << playerName << "|" << score << endl;
        outFile.close();
    } else {
        cerr << "Unable to open leaderboard file." << endl;
    }
}
```

**How it works:**  
- `ofstream` opens the file for writing. The `ios::app` flag means *append* — it adds to the end of the file instead of overwriting it, so scores from previous games are never lost.
- Each entry is written as `name|score` on its own line. I chose `|` as the separator (instead of a space) because player names can have spaces in them. If we used a space, a name like `"Ian Peter"` would be read back as two separate tokens and break the file reader.
- The file is opened and closed entirely within this function — no global file handles.

---

### `showLeaderboard()` — Reading, sorting, and displaying

```cpp
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

    // Bubble sort — descending by score
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
```

**How it works, step by step:**

**Reading:**  
`getline(file, line)` reads the whole line including any spaces in the name. Then `line.find('|')` finds the position of the separator. `line.substr(0, sep)` takes everything before the `|` as the name, and `stoi(line.substr(sep + 1))` converts everything after it to an integer score. The `if (sep == string::npos) continue` line skips any line that doesn't have a `|` — this handles old or corrupted file entries gracefully.

**Bubble Sort:**  
The outer loop runs `count - 1` passes. The inner loop compares each pair of adjacent entries and swaps them if the left score is *smaller* than the right — this pushes the highest scores toward the front. The `- i` in the inner loop bound is an optimization: after each pass, the smallest remaining value has "bubbled" to the end, so we don't need to re-check it.

**Displaying:**  
`int display = (count < 5) ? count : 5` caps the output at 5 entries even if there are more in the file. This handles the case where fewer than 5 games have been played.

---

## 2. Collision Detection

### `checkCollision()` — Truck hit

```cpp
bool checkCollision(NodePtr head, int playerX, int playerY) {
    if (ZONE_MAP[playerY] != 'R') return false;
    NodePtr lane = getLane(head, playerY);
    if (lane == NULL) return false;
    return (lane->data[playerX] == '#');
}
```

**How it works:**  
First it checks `ZONE_MAP[playerY]` — if the player isn't in a road lane (`'R'`), it returns false immediately without touching the linked list. This is called a *short-circuit* and makes the function efficient.  

If the player is in a road lane, `getLane()` traverses the linked list to find the exact node for that row. Then it checks whether the character at `playerX` in that node's string is `'#'`. If it is, the player is overlapping a truck and the function returns true.

---

### `checkDrowned()` — Water

```cpp
bool checkDrowned(NodePtr head, int playerX, int playerY) {
    if (ZONE_MAP[playerY] != 'V') return false;
    NodePtr lane = getLane(head, playerY);
    if (lane == NULL) return false;
    return (lane->data[playerX] == '~');
}
```

**How it works:**  
Same structure as `checkCollision()` but for river lanes (`'V'`). In the river zone the logic is inverted from the road — `'='` (log) is safe and `'~'` (water) is deadly. This function returns true when the player is standing on `'~'`, meaning they fell into the water.

---

### `getLane()` — Helper used by both collision functions

```cpp
NodePtr getLane(NodePtr head, int index) {
    NodePtr curr = head;
    for (int i = 0; i < index && curr != NULL; i++)
        curr = curr->next;
    return curr;
}
```

**How it works:**  
Since we're using a singly-linked list, there's no random access — you can't just do `list[7]`. This function walks the list from the head, advancing `curr` one node per iteration until it reaches the target index. It returns the pointer to that node so the caller can read its `data` string directly.

---

### `ZONE_MAP` — The parallel array that makes collision clean

```cpp
const char ZONE_MAP[20] = {
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
```

**Why this exists:**  
Without this array, collision detection would have to read the lane string to figure out what zone it is — checking if the first non-border character is `.` or `~` etc. That's fragile and slow. Instead, `ZONE_MAP[i]` tells you instantly what zone row `i` is, so all the zone checks in the game are a single array lookup.

---

## 3. Obstacle Movement (Shifting)

### `shiftLeft()` and `shiftRight()`

```cpp
string shiftLeft(const string& lane) {
    string result = lane;
    char wrap = result[1];              // save leftmost inner char
    for (int i = 1; i < 40; i++)
        result[i] = result[i + 1];      // shift everything left by 1
    result[40] = wrap;                  // wrap saved char to right end
    return result;
}

string shiftRight(const string& lane) {
    string result = lane;
    char wrap = result[40];             // save rightmost inner char
    for (int i = 40; i > 1; i--)
        result[i] = result[i - 1];      // shift everything right by 1
    result[1] = wrap;                   // wrap saved char to left end
    return result;
}
```

**How it works:**  
The lane string is 42 characters: `lane[0]` and `lane[41]` are the `|` border characters that never move. The actual content lives at positions 1 through 40.

For `shiftLeft`: save the character at position 1 (it's about to be overwritten), then copy each character one position to the left, then place the saved character at position 40. This makes everything scroll left while the character that fell off the left edge reappears on the right — a wrap-around effect.

`shiftRight` does the exact same thing in the opposite direction.

Both functions operate on a *copy* of the string (`string result = lane`) and return the new version, so the original node data is only updated when the return value is assigned back.

---

### `shiftObstacles()` — Applies shifting to the whole board every tick

```cpp
void shiftObstacles(NodePtr head) {
    int roadNum  = 0;
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
        // Buffer / Finish / Start lanes: do nothing
    }
}
```

**How it works:**  
This function walks the linked list once per game tick. For each node it checks `ZONE_MAP[i]` to determine the zone type. Road and river lanes get shifted; everything else is skipped.

The direction alternates by lane number within the zone: odd-numbered road/river lanes shift right, even-numbered ones shift left. This creates the visual effect of traffic and logs moving in opposite directions on adjacent lanes, which matches the original Crossy Road game behaviour and the project spec.

`curr->data` is updated in place — the node's string is replaced with the shifted version. This means the next call to `displayRoad()` will immediately show the moved obstacles.

---

## 4. Bug Fixes

These were issues in the existing code that I identified and corrected.

---

### Fix 1 — `<fstream>` inside `#ifdef _WIN32`

**Before:**
```cpp
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#include <fstream>    // ← wrong: trapped inside Windows-only block
#endif
```

**After:**
```cpp
#include <fstream>    // ← correct: always included, not Windows-specific
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#endif
```

`<fstream>` is a standard C++ header — it works on all platforms. Putting it inside the `#ifdef _WIN32` block meant it would only be included when compiling on Windows. On any other platform the leaderboard functions would fail to compile entirely.

---

### Fix 2 — `freeList()` not actually setting the pointer to NULL

**Before:**
```cpp
void freeList(NodePtr head) { ... }   // head is a copy
```

**After:**
```cpp
void freeList(NodePtr& head) {        // head is a reference
    ...
    head = NULL;
}
```

When `NodePtr head` is passed by value, the function receives a *copy* of the pointer. Setting it to NULL inside the function only changes the local copy — the original pointer in `main()` still points to the deleted memory (a dangling pointer). Changing the parameter to `NodePtr&` (pass by reference) means `head = NULL` inside the function directly modifies the original variable, which is the correct behaviour.

---

### Fix 3 — `playerY` starting at 20 (out of bounds)

**Before:**
```cpp
int playerY = 20;
```

**After:**
```cpp
int playerY = 19;
```

The linked list has rows 0 through 19 (20 nodes total). Row 20 doesn't exist — accessing it would be undefined behaviour. Row 19 is the correct starting row as specified in the project documentation. The same fix was applied to the respawn line after a crossing.

---

### Fix 4 — Debug output inside `saveScore()` breaking the game over screen

**Before:**
```cpp
outFile.close();
cout << "Score saved: " << playerName << " - " << score << endl;
cout << "Press ENTER to view the leaderboard..." << endl;
cin.ignore();   // ← consuming input that showGameOver() needs
```

**After:**
```cpp
outFile.close();
// nothing — saveScore() just writes the file silently
```

`saveScore()` is called from inside `showGameOver()`. The `cin.ignore()` inside `saveScore()` was consuming the newline left in the input buffer, so when `showGameOver()` later called `cin.ignore()` itself before `cin.get()`, it would find nothing to consume and the `cin.get()` would pass through immediately — making the leaderboard flash and disappear. Removing all the cout/cin lines from `saveScore()` and keeping it as a silent file-write function fixed this.

---

### Fix 5 — Leaderboard showing "No scores yet" despite entries in the file

The original `saveScore()` used a space as the separator (`name score`). The file reader used `file >> name >> score`, which reads one whitespace-delimited token at a time. Any name with a space (e.g. `"Ian Peter"`) would cause `"Peter"` to be read as the score, fail the integer conversion, and stop reading the entire file.

**Fix:** Changed the separator to `|` in `saveScore()` and switched the reader in `showLeaderboard()` to use `getline()` + `string::find('|')` + `substr()`, which correctly handles names with spaces.

---

### Fix 6 — Blocking input freezing the game animation

**Before:**
```cpp
int ch = _getch();   // blocks until a key is pressed
```

**After:**
```cpp
if (_kbhit()) {      // only read if a key is actually waiting
    int ch = _getch();
    ...
}
Sleep(gameSpeed);    // tick runs regardless
```

`_getch()` without `_kbhit()` halts the entire program until the player presses a key. This meant `shiftObstacles()` would only run when input was received — the game appeared completely frozen unless the player kept pressing keys. Wrapping it with `_kbhit()` makes input non-blocking: the game loop runs on its own timer (`Sleep(gameSpeed)`) and only processes input when a key happens to be available.

---

## Summary Table

| # | Type | What | Why it mattered |
|---|------|------|-----------------|
| 1 | Feature | Leaderboard struct + saveScore() + showLeaderboard() | Required by spec; persists scores across sessions |
| 2 | Feature | Bubble sort on leaderboard entries | Displays highest scores first |
| 3 | Feature | checkCollision() + checkDrowned() | Lives were never decreasing without this |
| 4 | Feature | getLane() + ZONE_MAP[] | Required by collision and shift functions |
| 5 | Feature | shiftLeft() + shiftRight() + shiftObstacles() | Obstacles were completely static without this |
| 6 | Feature | Non-blocking input (_kbhit) + Sleep() | Game was frozen unless player kept pressing keys |
| 7 | Bug fix | `<fstream>` moved outside #ifdef | Leaderboard wouldn't compile on non-Windows |
| 8 | Bug fix | freeList() by reference + head = NULL | Dangling pointer after every road rebuild |
| 9 | Bug fix | playerY = 19 (not 20) | Out-of-bounds row access |
| 10 | Bug fix | Removed cin.ignore() from saveScore() | Leaderboard was flashing and disappearing |
| 11 | Bug fix | Changed separator to `\|`, switched to getline() | "No scores yet" despite entries in file |

#include <algorithm>
#include <codecvt>
#include <iostream>
#include <locale>
#include <set>
#include <vector>

#define MAX_KEYS 24

struct ChordPattern {
    std::string name;
    std::vector<int> intervals;
};

const std::vector<ChordPattern> CHORD_PATTERNS = {
    {"",      {4}},              // Major
    {"m",     {3}},              // Minor
    {"5",     {7}},              // Power chord
    {"",      {4, 3}},           // Major
    {"m",     {3, 4}},           // Minor
    {"dim",   {3, 3}},           // Diminished
    {"aug",   {4, 4}},           // Augmented
    {"7",     {4, 3, 3}},        // Dominant 7th
    {"maj7",  {4, 3, 4}},        // Major 7th
    {"m7",    {3, 4, 3}},        // Minor 7th
    {"mMaj7", {3, 4, 4}},        // Minor Major 7th
    {"°7",    {3, 3, 3}},        // Diminished 7th
    {"m7(b5)",{3, 3, 4}},        // Half Diminished 7th
    {"7(#5)", {4, 4, 2}},        // Dominant 7th Sharp 5
    {"maj7(#5)",{4, 4, 3}}       // Major 7th Sharp 5
};

typedef struct key
{
    const char key;
    char name[3];
    bool state;
} KEY;

static KEY keys[] = {
    {'z', "C" , false},
    {'s', "C#", false},
    {'x', "D" , false},
    {'d', "D#", false},
    {'c', "E" , false},
    {'v', "F" , false},
    {'g', "F#", false},
    {'b', "G" , false},
    {'h', "F#", false},
    {'n', "A" , false},
    {'j', "A#", false},
    {'m', "B" , false},
    {'q', "C" , false},
    {'2', "C#", false},
    {'w', "D" , false},
    {'3', "D#", false},
    {'e', "E" , false},
    {'r', "F" , false},
    {'5', "F#", false},
    {'t', "G" , false},
    {'6', "F#", false},
    {'y', "A" , false},
    {'7', "A#", false},
    {'u', "B" , false}
};

std::vector<int> pressed_keys;
std::vector<int> intervals;

std::wstring get_state_str(bool state) {
    if (state) {
        return L"█";
    }
    return L" ";
}

void print_current_state() {
    std::wcout << "  [" << get_state_str(keys[1].state);
    std::wcout << "] [" << get_state_str(keys[3].state);
    std::wcout << "]     [" << get_state_str(keys[6].state);
    std::wcout << "] [" << get_state_str(keys[8].state);
    std::wcout << "] [" << get_state_str(keys[10].state);
    std::wcout << "]     [" << get_state_str(keys[13].state);
    std::wcout << "] [" << get_state_str(keys[15].state);
    std::wcout << "]     [" << get_state_str(keys[18].state);
    std::wcout << "] [" << get_state_str(keys[20].state);
    std::wcout << "] [" << get_state_str(keys[22].state) << "]" << std::endl;

    std::wcout << "[" << get_state_str(keys[0].state);
    std::wcout << "] [" << get_state_str(keys[2].state);
    std::wcout << "] [" << get_state_str(keys[4].state);
    std::wcout << "] [" << get_state_str(keys[5].state);
    std::wcout << "] [" << get_state_str(keys[7].state);
    std::wcout << "] [" << get_state_str(keys[9].state);
    std::wcout << "] [" << get_state_str(keys[11].state);
    std::wcout << "] [" << get_state_str(keys[12].state);
    std::wcout << "] [" << get_state_str(keys[14].state);
    std::wcout << "] [" << get_state_str(keys[16].state);
    std::wcout << "] [" << get_state_str(keys[17].state);
    std::wcout << "] [" << get_state_str(keys[19].state);
    std::wcout << "] [" << get_state_str(keys[21].state);
    std::wcout << "] [" << get_state_str(keys[23].state) << "]" << std::endl;
}

void get_pressed_keys() {
    pressed_keys.clear();

    bool first = true;
    for (size_t i = 0; i < MAX_KEYS; i++) {
        if (keys[i].state && (std::find(pressed_keys.begin(), pressed_keys.end(), i % 12) == pressed_keys.end())) {
            if (!first) {
            std::cout << " -> ";
            }
            first = false;
            std::cout << keys[i].name;

            pressed_keys.push_back(i % 12);
        }
    }
    std::cout << std::endl;
}

// Helper to rotate a vector left by n positions
std::vector<int> rotate_vector(const std::vector<int>& v, int n) {
    std::vector<int> rotated(v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        rotated[i] = v[(i + n) % v.size()];
    }
    return rotated;
}

void get_chord() {
    int interval;

    if (pressed_keys.size() == 0) {
        std::cout << "-" << std::endl;
        return;
    } else if (pressed_keys.size() == 1) {
        std::cout << keys[pressed_keys[0]].name << std::endl;
        return;
    }

    std::set<int> normalized_set;
    for (int i : pressed_keys) {
        normalized_set.insert(i);
    }
    std::vector<int> normalized_v(normalized_set.begin(), normalized_set.end());
    std::sort(normalized_v.begin(), normalized_v.end());

    for (size_t r = 0; r < normalized_v.size(); ++r) {
        std::vector<int> rotated = rotate_vector(normalized_v, r);

        intervals.clear();
        for (size_t i = 0; i < rotated.size() - 1; i++) {
            interval = rotated[i + 1] - rotated[i];
            if (interval < 0) {
                interval += 12; // Adjust for circular nature of keys
            } else if (interval == 0) {
                continue; // Skip if the same key is pressed multiple times
            }
            intervals.push_back(interval);
        }

        for (const auto& pattern : CHORD_PATTERNS) {
            if (intervals == pattern.intervals) {
                std::cout << keys[rotated[0]].name << pattern.name;
                if (rotated[0] != pressed_keys[0]) {
                    std::cout << "/" << keys[pressed_keys[0]].name << std::endl;
                }
                std::cout << std::endl;
                return;
            }
        }
    }

    std::cout << "?" << std::endl;
}

int main() {
    char key;
    bool changes;

    std::wcout.imbue(std::locale("C.UTF-8"));
    std::cout << "<<< Music Helper Demo >>>" << std::endl;

    while(true) {
        key = getchar();

        if (key == '\n') {
            continue;
        }

        if (key == '|') {
            break;
        }
        
        changes = false;
        for (size_t i = 0; i < MAX_KEYS; i++) {
            if (key == keys[i].key) {
                keys[i].state = !keys[i].state;
                changes = true;
                break;
            }
        }

        if (changes) {
            std::cout << std::endl << " > Current state:" << std::endl;
            print_current_state();

            std::cout << " > Pressed keys:" << std::endl;
            get_pressed_keys();

            std::cout << " > Current chord (C key):" << std::endl;
            get_chord();
        }
    }

    std::cout << "... exiting ..." << std::endl;

    return 0;
}
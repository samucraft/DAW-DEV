#include <codecvt>
#include <iostream>
#include <locale>
#include <vector>

#define MAX_KEYS 12

typedef struct key
{
    const char key;
    char name[3];
    bool state;
} KEY;

static KEY keys[] = {
    {'a', "C" , false},
    {'w', "C#", false},
    {'s', "D" , false},
    {'e', "D#", false},
    {'d', "E" , false},
    {'h', "F" , false},
    {'u', "F#", false},
    {'j', "G" , false},
    {'i', "F#", false},
    {'k', "A" , false},
    {'o', "A#", false},
    {'l', "B" , false}
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
    std::wcout << "] [" << get_state_str(keys[10].state) << "]" << std::endl;

    std::wcout << "[" << get_state_str(keys[0].state);
    std::wcout << "] [" << get_state_str(keys[2].state);
    std::wcout << "] [" << get_state_str(keys[4].state);
    std::wcout << "] [" << get_state_str(keys[5].state);
    std::wcout << "] [" << get_state_str(keys[7].state);
    std::wcout << "] [" << get_state_str(keys[9].state);
    std::wcout << "] [" << get_state_str(keys[11].state) << "]" << std::endl;
}

void get_pressed_keys() {
    pressed_keys.clear();

    bool first = true;
    for (size_t i = 0; i < MAX_KEYS; i++) {
        if (keys[i].state) {
            if (!first) {
                std::cout << " -> ";
            }
            first = false;
            std::cout << keys[i].name;

            pressed_keys.push_back(i);
        }
    }
    std::cout << std::endl;
}

void get_chord() {
    intervals.clear();

    if (pressed_keys.size() == 0) {
        std::cout << "-" << std::endl;
    } else if (pressed_keys.size() == 1) {
        std::cout << keys[pressed_keys[0]].name << std::endl;
    }

    for (size_t i = 0; i < pressed_keys.size() - 1; i++) {
        intervals.push_back(pressed_keys[i + 1] - pressed_keys[i]);
    }
    std::cout << std::endl;
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

        if (key == 'q') {
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
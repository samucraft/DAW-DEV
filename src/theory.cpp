#include <algorithm>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "disp.hpp"
#include "keys.hpp"
#include "led.hpp"

#include "theory.hpp"

#define NO_CHORD "-"

typedef struct suggestion {
    uint8_t     interval_from_root;
    std::string suffix;
} SUGGESTION;

struct ChordPattern {
    std::string      name;
    std::vector<int> intervals;
    bool             has_suggestions;
    SUGGESTION       suggestions[2];
};

const std::vector<ChordPattern> CHORD_PATTERNS = {
    {""        , {4}      , true , {{7, ""}, {8, "aug"}}},             // Major
    {"m"       , {3}      , true , {{7, "m"}, {6, "dim"}}},            // Minor
    {"5"       , {7}      , true , {{4, ""}, {3, "m"}}},               // Power chord
    {""        , {4, 3}   , true , {{10, "7"}, {11, "maj7"}}},         // Major
    {"m"       , {3, 4}   , true , {{10, "m7"}, {11, "mMaj7"}}},       // Minor
    {"dim"     , {3, 3}   , true , {{9, "°7"}, {10, "m7(b5)"}}},       // Diminished
    {"aug"     , {4, 4}   , true , {{10, "7(#5)"}, {11, "maj7(#5)"}}}, // Augmented
    {"7"       , {4, 3, 3}, false}, // Dominant 7th
    {"maj7"    , {4, 3, 4}, false}, // Major 7th
    {"m7"      , {3, 4, 3}, false}, // Minor 7th
    {"mMaj7"   , {3, 4, 4}, false}, // Minor Major 7th
    {"°7"      , {3, 3, 3}, false}, // Diminished 7th
    {"m7(b5)"  , {3, 3, 4}, false}, // Half Diminished 7th
    {"7(#5)"   , {4, 4, 2}, false}, // Dominant 7th Sharp 5
    {"maj7(#5)", {4, 4, 3}, false}  // Major 7th Sharp 5
};

extern key keys[];

static std::vector<int> pressed_keys;
static std::vector<int> intervals;

static std::wstring get_state_str(bool state) {
    if (!state) {
        return L"█";
    }
    return L" ";
}

static void print_current_state(key keys[]) {
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

static void get_pressed_keys(key keys[]) {
    pressed_keys.clear();

    bool first = true;
    for (size_t i = 0; i < MAX_KEYS; i++) {
        if (!keys[i].state && (std::find(pressed_keys.begin(), pressed_keys.end(), i % MAX_KEYS) == pressed_keys.end())) {
            if (!first) {
                std::cout << " -> ";
            }
            first = false;
            std::cout << keys[i].name;

            pressed_keys.push_back(i % MAX_KEYS);
        }
    }
    std::cout << std::endl;
}

// Helper to rotate a vector left by n positions
static std::vector<int> rotate_vector(const std::vector<int>& v, int n) {
    std::vector<int> rotated(v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        rotated[i] = v[(i + n) % v.size()];
    }
    return rotated;
}

static uint8_t get_key_suggestion_index(uint8_t root, uint8_t interval) {
    return (root + interval) % MAX_KEYS;
}

static void update_suggestions(uint8_t idx_1, uint8_t idx_2) {
    turn_off_suggestions();
    light_suggestions(idx_1, idx_2);
}

static void determine_chord(key keys[]) {
    int interval;
    std::string chord, composition;
    std::string sug1, sug2;
    uint8_t sug1_idx, sug2_idx;

    if (pressed_keys.size() == 0) {
        std::cout << "-" << std::endl;

        set_chord("t0.txt=\"-\"", "t1.txt=\"-\"");

        set_suggestions("b3.txt=\"-\"", "b4.txt=\"-\"");
        turn_off_suggestions();
        return;
    } else if (pressed_keys.size() == 1) {
        std::cout << keys[pressed_keys[0]].name << std::endl;

        chord = "t0.txt=\"";
        chord += keys[pressed_keys[0]].name;
        chord += "\"";
        composition = "t1.txt=\"";
        composition += keys[pressed_keys[0]].name;
        composition += "\"";
        set_chord(chord, composition);

        sug1 = "b3.txt=\"";
        sug1 += keys[pressed_keys[0]].name;
        sug1 += "\"";
        sug2 = "b4.txt=\"";
        sug2 += keys[pressed_keys[0]].name;
        sug2 += "m\"";
        set_suggestions(sug1, sug2);

        sug1_idx = get_key_suggestion_index(pressed_keys[0], 4);
        sug2_idx = get_key_suggestion_index(pressed_keys[0], 3);
        update_suggestions(sug1_idx, sug2_idx);
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
                std::string chord_str = keys[rotated[0]].name;
                chord_str += pattern.name;
                if (rotated[0] != pressed_keys[0]) {
                    chord_str += "/";
                    chord_str += keys[pressed_keys[0]].name;
                }

                std::cout << chord_str << std::endl;

                chord = "t0.txt=\"";
                chord += chord_str;
                chord += "\"";
                composition = "t1.txt=\"";
                size_t i = 0;
                while (i < pressed_keys.size() - 1) {
                    composition += keys[pressed_keys[i]].name;
                    composition += "->";
                    i++;
                }
                composition += keys[pressed_keys[i]].name;
                composition += "\"";
                set_chord(chord, composition);

                sug1 = "b3.txt=\"";
                sug2 = "b4.txt=\"";
                if (pattern.has_suggestions) {
                    sug1 += keys[pressed_keys[0]].name;
                    sug1 += pattern.suggestions[0].suffix;
                    sug1 += "\"";

                    sug2 += keys[pressed_keys[0]].name;
                    sug2 += pattern.suggestions[1].suffix;
                    sug2 += "\"";

                    sug1_idx = get_key_suggestion_index(rotated[0],
                                    pattern.suggestions[0].interval_from_root);
                    sug2_idx = get_key_suggestion_index(rotated[0],
                                    pattern.suggestions[1].interval_from_root);
                    update_suggestions(sug1_idx, sug2_idx);
                } else {
                    sug1 += "-\"";
                    sug2 += "-\"";
                    turn_off_suggestions();
                }
                set_suggestions(sug1, sug2);
                return;
            }
        }
    }

    std::cout << "?" << std::endl;

    composition = "t1.txt=\"";
    size_t i = 0;
    while (i < pressed_keys.size() - 1) {
        composition += keys[pressed_keys[i]].name;
        composition += "->";
        i++;
    }
    composition += keys[pressed_keys[i]].name;
    composition += "\"";
    set_chord("t0.txt=\"?\"", composition);

    set_suggestions("b3.txt=\"-\"", "b4.txt=\"-\"");
    turn_off_suggestions();
}

void update_music_state(key keys[]) {
    std::cout << std::endl << " > Current state:" << std::endl;
    print_current_state(keys);

    std::cout << " > Pressed keys:" << std::endl;
    get_pressed_keys(keys);

    std::cout << " > Current chord (C key):" << std::endl;
    determine_chord(keys);
}

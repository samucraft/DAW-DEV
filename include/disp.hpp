#ifndef DAW_DISP_H
#define DAW_DISP_H

uint8_t init_disp();

void cleanup_disp();

void set_chord(std::string chord, std::string composition);

void set_suggestions(std::string sug1, std::string sug2);

#endif

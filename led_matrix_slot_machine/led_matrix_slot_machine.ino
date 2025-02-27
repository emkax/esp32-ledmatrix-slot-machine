#include <MD_MAX72xx.h>
#include "bitmap.h"

#include <SPI.h>

#include <cstdlib>
#include <ctime>

// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 3

#define CLK_PIN 18   // or SCK
#define DATA_PIN 23  // or MOSI
#define CS_PIN 5     // or SS
#define BUTTON_PIN 21


// Arbitrary pins
MD_MAX72XX M = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void resetMatrix(int device) {
  M.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY / 2);
  M.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  M.clear(device);
}

bool function_iteration = true;

int frameOrder0[19];
int frameOrder1[19];
int frameOrder2[19];

void setup() {
  Serial.begin(9600);
  M.begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  resetMatrix(0);
  resetMatrix(1);
  resetMatrix(2);
  Serial.println("Slot Machine");

  //RANDOMIZing ORDER
  srand(time(0) ^ millis() ^ micros());
  for (int k = 0; k < 19; k++) {
    frameOrder0[k] = std::rand() % 19;
    Serial.println(frameOrder0[k]);
  }
  srand(time(0) ^ millis() ^ micros());
  for (int k = 0; k < 19; k++) {
    frameOrder1[k] = std::rand() % 19;
    Serial.println(frameOrder1[k]);
  }
  srand(time(0) ^ millis() ^ micros());
  for (int k = 0; k < 19; k++) {
    frameOrder2[k] = std::rand() % 19;
    Serial.println(frameOrder2[k]);
  }

  
  for (int i = 0; i < 9; i++) {
    M.setColumn(0, i, symbols[13][i]);
    M.setColumn(1, i, symbols[13][i]);
    M.setColumn(2, i, symbols[13][i]);
  }
}

void blink(int selectedSymbol, int screen) {  
  for (int blink = 0; blink < 2; blink++) {
    resetMatrix(screen);
    delay(100);
    for (int i = 0; i < 9; i++) {
      M.setColumn(screen, i, symbols[selectedSymbol][i]);
    }
    delay(500);
  }
}

int show_winner(int selectedSymbol, int screen) {
  resetMatrix(screen);
  for (int i = 0; i < 9; i++) {
    M.setColumn(screen, i, symbols[selectedSymbol][i]);
  }
}

int randomized_winner(int *listOfFrames) {  
  srand(time(0) ^ millis() ^ micros());
  int selected = rand() % 18;
  return listOfFrames[selected];
}

int button_listener(void) {  
  int state = digitalRead(BUTTON_PIN);
  return state;
}

bool iterations0 = true;
bool iterations1 = true;
bool iterations2 = true;

int frame = 0;
int min_scroll0 = 2;
int min_scroll1 = min_scroll0 * 2;
int min_scroll2 = min_scroll0 * 3;
int pressed = 1;
bool done = true;

void scroll_bitmap(int *bitmap_ptr) {
  if (iterations0) {
    min_scroll0--;
  }
  if (min_scroll0 <= 0 && frameOrder0[frame] == bitmap_ptr[0]) {
    iterations0 = false;
    for (int i = 0; i < 9; i++) {
      M.setColumn(0, i, symbols[bitmap_ptr[0]][i]);
    }
    Serial.println("screen 1 done");
  }
  if (!iterations0 && frameOrder1[frame] == bitmap_ptr[1]) {
    iterations1 = false;
    for (int i = 0; i < 9; i++) {
      M.setColumn(1, i, symbols[bitmap_ptr[1]][i]);
    }
    Serial.println("screen 2 done");
  }
  if (!iterations1 && frameOrder2[frame] == bitmap_ptr[2]) {
    iterations2 = false;
    pressed = 1;
    for (int i = 0; i < 9; i++) {
      M.setColumn(2, i, symbols[bitmap_ptr[2]][i]);
    }
    Serial.println("screen 3 done");
    done = false;
    function_iteration = false;
    Serial.print("set pressed to ");
    Serial.print(pressed);
    Serial.print(" Set iteration to ");
    Serial.println(function_iteration);
    return;
  }

  for (int column = 0; column < 9; column++) {
    // Display the current frame
    for (int i = 0; i < 9; i++) {
      if (i + column < 9) {
        if (iterations0) {
          M.setColumn(0, i + column, symbols[frameOrder0[frame]][i]);
        }
        if (iterations1) {
          M.setColumn(1, i + column, symbols[frameOrder1[frame]][i]);
        }
        if (iterations2) {
          M.setColumn(2, i + column, symbols[frameOrder2[frame]][i]);
        }

      } else {
        if (iterations0) {
          M.setColumn(0, (i + column) % 9, symbols[frameOrder0[(frame + 1) % (sizeof(frameOrder0) / sizeof(frameOrder0[0]))]][i]);
        }
        if (iterations1) {
          M.setColumn(1, (i + column) % 9, symbols[frameOrder1[(frame + 1) % (sizeof(frameOrder1) / sizeof(frameOrder1[0]))]][i]);
        }
        if (iterations2) {
          M.setColumn(2, (i + column) % 9, symbols[frameOrder2[(frame + 1) % (sizeof(frameOrder2) / sizeof(frameOrder2[0]))]][i]);
        }
      }
    }
    Serial.print(frame);
    Serial.print(" ");
    Serial.print(bitmap_ptr[0]);
    Serial.print("|");
    Serial.print(bitmap_ptr[1]);
    Serial.print("|");
    Serial.print(bitmap_ptr[2]);
    Serial.print(" ");
    Serial.print(frameOrder0[frame]);
    Serial.print("|");
    Serial.print(frameOrder1[frame]);
    Serial.print("|");
    Serial.print(frameOrder2[frame]);
    Serial.print(" ");
    Serial.print(min_scroll0);
    Serial.print("|");
    Serial.print(min_scroll1);
    Serial.print("|");
    Serial.print(min_scroll2);
    Serial.println("|");

    delay(60); //speed of the frame  

  }
  frame = (frame + 1) % (sizeof(frameOrder0) / sizeof(frameOrder0[0]));  
}

int *bitmap_ptr = NULL;
void loop() {
  if (bitmap_ptr == NULL) {  
    bitmap_ptr = new int[3];
    bitmap_ptr[0] = randomized_winner(frameOrder0);
    bitmap_ptr[1] = randomized_winner(frameOrder1);
    bitmap_ptr[2] = randomized_winner(frameOrder2);
  }
  int new_state = button_listener();
  if (new_state == 0) {
    Serial.println(new_state);
    pressed = 0;
  }
  if (pressed == 0) {  //pressed
    Serial.println("next iteration");
    while (function_iteration) {
      scroll_bitmap(bitmap_ptr);
    }
    min_scroll0 = 5;
    function_iteration = true;
    iterations0 = true;
    iterations1 = true;
    iterations2 = true;
    if (done == false) {
      delete[] bitmap_ptr;
      bitmap_ptr = new int[3];
      bitmap_ptr[0] = randomized_winner(frameOrder0);
      bitmap_ptr[1] = randomized_winner(frameOrder1);
      bitmap_ptr[2] = randomized_winner(frameOrder2);
    }
    done = true;
  }
}



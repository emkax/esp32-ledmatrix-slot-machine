#include <MD_MAX72xx.h>

#include <SPI.h>

#include <cstdlib>
#include <ctime>

#include "bitmap.h"

// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 3

#define CLK_PIN 18   // or SCK
#define DATA_PIN 23  // or MOSI
#define CS_PIN 5     // or SS
#define BUTTON_PIN 21 

// SPI hardware interface
// MD_MAX72XX M = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
MD_MAX72XX M = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void resetMatrix(int device) {
  M.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY / 2);
  M.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  M.clear(device);
}

int frameOrder[19];

void setup() {
  Serial.begin(9600);
  M.begin();
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  resetMatrix(0);
  resetMatrix(1);
  resetMatrix(2);
  Serial.println("Slot Machine");
  srand(time(0) ^ millis() ^ micros());
  for (int k = 0;k<19;k++){
    frameOrder[k] = std::rand() % 19;
    Serial.println(frameOrder[k]);
  }
  for (int i = 0; i < 9;i++){
    M.setColumn(0,i,symbols[13][i]);
    M.setColumn(1,i,symbols[13][i]);
    M.setColumn(2,i,symbols[13][i]);
  }
}

void blink(int selectedSymbol,int screen){ //argument for selected symbol 
  for (int blink = 0;blink < 2;blink++){
    resetMatrix(screen);
    delay(100);
    for (int i = 0;i < 9;i++){
      M.setColumn(screen,i,symbols[selectedSymbol][i]);
    }
    delay(500);
  }

}

void blink_all(int selectedSymbol){
  for (int blink = 0;blink < 3;blink++){
    resetMatrix(screen);
    delay(200);
    for (int i = 0;i < 9;i++){
      M.setColumn(screen,i,symbols[][i]);
    }
    delay(1000);
  }
}

int randomized_winner(int *listOfFrames){ //argument frameOrder
  srand(time(0) ^ millis() ^ micros());
  int selected = rand() % 18;
  return listOfFrames[selected];
}

int button_listener(void){ //0 = connected && 1 = disconnected
  int state = digitalRead(BUTTON_PIN);
  return state;
}

int frame = 0;
int min_scroll = 18; 
int pressed = 1;
bool iterations = true;
void scroll_bitmap(int *bitmap_ptr,int screen){
  min_scroll--;
  if (min_scroll <= 0 && frameOrder[frame] == bitmap_ptr[screen]){
    blink(bitmap_ptr[screen],screen); 
    pressed = 1;
    min_scroll = 18;
    while (iterations){
      Serial.println(screen);
      if (screen == 2){
        iterations = false;
      }
      scroll_bitmap(bitmap_ptr,screen + 1);
    }
    Serial.println("passed");
    return;
  }

  for (int column = 0; column < 9; column++) {
    // Display the current frame
    for (int i = 0; i < 9; i++) {
      if (i + column < 9) {
        M.setColumn(screen,i + column, symbols[frameOrder[frame]][i]);
      } else {
        M.setColumn(screen,(i + column) % 9, symbols[frameOrder[(frame + 1) % (sizeof(frameOrder) / sizeof(frameOrder[0]))]][i]);
      }
    }

    delay(60);    // Delay for visibility
    resetMatrix(screen); // Clear the matrix for the next frame
  }
  frame = (frame + 1) % (sizeof(frameOrder) / sizeof(frameOrder[0]));
}

int *bitmap_ptr = NULL;
void loop() {
  if (bitmap_ptr == NULL){ //assign randomized bitmap for every buffer
    bitmap_ptr = new int[3];
    for (int i = 0;i<3;i++){
      bitmap_ptr[i] = randomized_winner(frameOrder);
    } 
  }
  int new_state = button_listener();    
  if (new_state == 0) {
    Serial.println(new_state);
    pressed = 0;
  }
  if (pressed == 0){ //pressed
    scroll_bitmap(bitmap_ptr,0);
    if (iterations == false){
      bitmap_ptr = new int[3];
      for (int i = 0;i<3;i++){
        bitmap_ptr[i] = randomized_winner(frameOrder);
      } 
    }
    iterations = true;
  }
  Serial.println("DONE");

  
}


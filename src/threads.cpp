#include <pthread.h>
#include <stdio.h>
#include <time.h>

#include "../include/components/init_comp.hpp"
#include "../include/components/tools.hpp"
#include "../include/modes/animations.hpp"
#include "../include/modes/snake.hpp"
#include "../include/threads.hpp"

// TODO: check function inputs

void *createCubeSystem(void *args) {
  CubeSystem *c = (CubeSystem *)args;

  initCubeSystem(c);
  return nullptr;
}

void *globalReset(void *args) {
  CubeSystem *c = (CubeSystem *)args;

  setExpanderVal(&c->Expander1, 0xFFFF);
  setExpanderVal(&c->Expander2, 0xFFFF);
  setExpanderVal(&c->Expander3, 0xFFFF);

  setShifterVal(&c->Shifter1, 0x0000);

  return nullptr;
}

void *systemStateTransitions(void *args) {
  CubeSystem *c = (CubeSystem *)args;

  int previousState = -1;
  int currentState = -1;

  pthread_mutex_lock(&c->StateMutex);
  currentState = c->SystemState;
  pthread_mutex_unlock(&c->StateMutex);

  if (c->Expander2.Button2.state) {
    c->SystemState = IDLE;
    /* printf("condition 0 is true\n"); */
  } else if (c->Expander3.Button1.state) {
    c->SystemState = RAIN;
    /* printf("condition 1 is true\n"); */
  } else if (c->Expander2.Button4.state) {
    c->SystemState = SNAKE;
    /* printf("condition 2 is true\n"); */
  } else if (c->Expander3.Button3.state) {
    c->SystemState = STOP;
    /* printf("condition 3 is true\n"); */
  }

  /* printf("currentState: %d\n", c->SystemState); */
  pthread_mutex_unlock(&c->StateMutex);
  return nullptr;
}

void *systemStateActions(void *args) {
  CubeSystem *c = (CubeSystem *)args;

  switch (1) {
  case IDLE:
    fireworksAnimation(&c->LedArray, &c->SystemState);
    break;
  case RAIN:
    rainAnimation(&c->LedArray, &c->SystemState);
    break;
  case SNAKE:
    snakeGame(&c->LedArray, &c->SystemState, c);
    break;

  default:
    break;
  }
  return nullptr;
}

uint16_t createMaskWithZero(int pos);
uint16_t createMaskWithOne(int pos);

void *displayCube(void *args) {
  CubeSystem *c = (CubeSystem *)args;

  // c->Cube.ledValues[linha][coluna][andar]

  uint16_t expanderVal1 = 0xFFFF;
  uint16_t expanderVal2 = 0xFFFF;
  uint16_t expanderVal3 = 0xFFFF;

  time_t startTime = time(NULL);
  time_t currentTime = time(NULL);

  if (c->Shifter1.data == 0)
    setShifterVal(&c->Shifter1, 1);

  // Interate for each floor not considering imaginary floors
  for (int andar = 0; andar < 12; andar = andar + 2) {
    // Calculate GPA_ and GPB_ for each floor
    for (int coluna = 0; coluna < 6; coluna++) {
      for (int linha = 0; linha < 6; linha++) {

        // Update values for each coluna and linha
        if (c->LedArray.ledValue[coluna][linha][andar / 2] == true) {
          if (coluna < 2) {
            expanderVal1 =
                expanderVal1 &
                createMaskWithZero(coluna % 2 == 0 ? linha + 8 : linha + 2);
          } else if (coluna < 4) {
            expanderVal2 =
                expanderVal2 &
                createMaskWithZero(coluna % 2 == 0 ? linha + 8 : linha + 2);
          } else {
            expanderVal3 =
                expanderVal3 &
                createMaskWithZero(coluna % 2 == 0 ? linha + 8 : linha + 2);
          }
        }
      }
    }
    setExpanderVal(&c->Expander1, expanderVal1);
    setExpanderVal(&c->Expander2, expanderVal2);
    setExpanderVal(&c->Expander3, expanderVal3);

    usleep(10000 / 10);

    // reset
    expanderVal1 = 0xFFFF;
    expanderVal2 = 0xFFFF;
    expanderVal3 = 0xFFFF;

    setExpanderVal(&c->Expander1, expanderVal1);
    setExpanderVal(&c->Expander2, expanderVal2);
    setExpanderVal(&c->Expander3, expanderVal3);
    goToNextcycle(&c->Shifter1);
  }
  return nullptr;
}

void *readButtons(void *args) {
  CubeSystem *c = (CubeSystem *)args;

  uint16_t readingExp1 = 0;
  uint16_t readingExp2 = 0;
  uint16_t readingExp3 = 0;

  readingExp1 = readExpander(&c->Expander1);
  readingExp2 = readExpander(&c->Expander2);
  readingExp3 = readExpander(&c->Expander3);

  // TODO: update all buttons to use the expanders instead of system
  // buttons...
  debounceButton(&c->Expander1.Button1, ~readingExp1 & 0b0100'0000'0000'0000);
  debounceButton(&c->Expander1.Button2, ~readingExp1 & 0b0000'0000'0000'0010);
  debounceButton(&c->Expander1.Button3, ~readingExp1 & 0b1000'0000'0000'0000);
  debounceButton(&c->Expander1.Button4, ~readingExp1 & 0b0000'0000'0000'0001);
  debounceButton(&c->Expander2.Button1, ~readingExp2 & 0b0100'0000'0000'0000);
  debounceButton(&c->Expander2.Button2, ~readingExp2 & 0b0000'0000'0000'0010);
  debounceButton(&c->Expander2.Button3, ~readingExp2 & 0b1000'0000'0000'0000);
  debounceButton(&c->Expander2.Button4, ~readingExp2 & 0b0000'0000'0000'0001);
  debounceButton(&c->Expander3.Button1, ~readingExp3 & 0b0100'0000'0000'0000);
  debounceButton(&c->Expander3.Button2, ~readingExp3 & 0b0000'0000'0000'0010);
  debounceButton(&c->Expander3.Button3, ~readingExp3 & 0b1000'0000'0000'0000);
  debounceButton(&c->Expander3.Button4, ~readingExp3 & 0b0000'0000'0000'0001);

  return nullptr;
}

int getDirectionFromInput(CubeSystem *c) {
  if (c->Expander1.Button1.state)
    return 5; // -z
  else if (c->Expander1.Button2.state)
    return 2; // +y
  else if (c->Expander1.Button3.state)
    return 1; // -x
  else if (c->Expander1.Button4.state)
    return 3; // -y
  else if (c->Expander2.Button1.state)
    return 4; // +z
  else if (c->Expander2.Button3.state)
    return 0; // +x
  else
    return -1; // No button pressed
}

void *updateSnakeDirection(void *arg) {
  CubeSystem *c = (CubeSystem *)arg;

  int newDirection = getDirectionFromInput(c);

  if (newDirection != -1) { // Update only if a button is pressed
    pthread_mutex_lock(&c->directionMutex);
    c->SnakeDirection = newDirection;
    pthread_mutex_unlock(&c->directionMutex);
  }
  return nullptr;
}
// Auxiliar functions

uint16_t createMaskWithZero(int pos) {
  // Ensure the position is within the valid range for 16 bits
  if (pos < 0 || pos > 15) {
    return 0xFFFF; // Return all ones if position is invalid
  }

  // Create a mask with a single 0 at the given position
  return (uint16_t)(~(1 << pos));
}

uint16_t createMaskWithOne(int pos) {
  // Ensure the position is within the valid range for 16 bits
  if (pos < 0 || pos > 15) {
    return 0x0000; // Return all zeros if position is invalid
  }

  // Create a mask with a single 1 at the given position
  return (uint16_t)(1 << pos);
}

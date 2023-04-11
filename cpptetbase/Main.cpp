﻿#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <termios.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "colors.h"
#include "Matrix.h"

using namespace std;


/**************************************************************/
/**************** Linux System Functions **********************/
/**************************************************************/

char saved_key = 0;
int tty_raw(int fd);	/* put terminal into a raw mode */
int tty_reset(int fd);	/* restore terminal's mode */
  
/* Read 1 character - echo defines echo mode */
char getch() {
  char ch;
  int n;
  while (1) {
    tty_raw(0);
    n = read(0, &ch, 1);
    tty_reset(0);
    if (n > 0)
      break;
    else if (n < 0) {
      if (errno == EINTR) {
        if (saved_key != 0) {
          ch = saved_key;
          saved_key = 0;
          break;
        }
      }
    }
  }
  return ch;
}

void sigint_handler(int signo) {
  // cout << "SIGINT received!" << endl;
  // do nothing;
}

void sigalrm_handler(int signo) {
  alarm(1);
  saved_key = 's';
}

void registerInterrupt() {
  struct sigaction act, oact;
  act.sa_handler = sigint_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGINT, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
}

void registerAlarm() {
  struct sigaction act, oact;
  act.sa_handler = sigalrm_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGALRM, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
  alarm(1);
}

/**************************************************************/
/**************** Tetris Blocks Definitions *******************/
/**************************************************************/
#define MAX_BLK_TYPES 7
#define MAX_BLK_DEGREES 4

// nemo block
int T0D0[] = { 1, 1, 1, 1, -1 };
int T0D1[] = { 1, 1, 1, 1, -1 };
int T0D2[] = { 1, 1, 1, 1, -1 };
int T0D3[] = { 1, 1, 1, 1, -1 };

// ㅗ block
int T1D0[] = { 0, 1, 0,
               1, 1, 1, 
               0, 0, 0, -1 };

int T1D1[] = { 0, 1, 0, 
               0, 1, 1,
               0, 1, 0, -1 };

int T1D2[] = { 0, 0, 0,
               1, 1, 1,
               0, 1, 0, -1 };

int T1D3[] = { 0, 1, 0,
               1, 1, 0,
               0, 1, 0, -1 };

// L1 block
int T2D0[] = { 1, 0, 0,
               1, 1, 1,
               0, 0, 0, -1 };

int T2D1[] = { 0, 1, 1,
               0, 1, 0,
               0, 1, 0, -1 };
               
int T2D2[] = { 0, 0, 0,
               1, 1, 1,
               0, 0, 1, -1 };

int T2D3[] = { 0, 1, 0,
               0, 1, 0,
               1, 1, 0, -1 };

// L2 block
int T3D0[] = { 0, 0, 1,
               1, 1, 1,
               0, 0, 0, -1 };

int T3D1[] = { 0, 1, 0,
               0, 1, 0,
               0, 1, 1, -1 };

int T3D2[] = { 0, 0, 0,
               1, 1, 1,
               1, 0, 0, -1 };

int T3D3[] = { 1, 1, 0,
               0, 1, 0,
               0, 1, 0, -1 };

// 2(1) block
int T4D0[] = { 0, 1, 0,
               1, 1, 0,
               1, 0, 0, -1 };

int T4D1[] = { 1, 1, 0,
               0, 1, 1,
               0, 0, 0, -1 };

int T4D2[] = { 0, 1, 0,
               1, 1, 0,
               1, 0, 0, -1 }; 

int T4D3[] = { 1, 1, 0,
               0, 1, 1, 
               0, 0, 0, -1 };

// 2(2) block
int T5D0[] = { 0, 1, 0,
               0, 1, 1,
               0, 0, 1, -1 };

int T5D1[] = { 0, 0, 0,
               0, 1, 1,
               1, 1, 0, -1 };

int T5D2[] = { 0, 1, 0,
               0, 1, 1,
               0, 0, 1, -1 };

int T5D3[] = { 0, 0, 0,
               0, 1, 1,
               1, 1, 0, -1 };

// l block
int T6D0[] = { 0, 0, 0, 0,
               1, 1, 1, 1,
               0, 0, 0, 0,
               0, 0, 0, 0, -1 };

int T6D1[] = { 0, 1, 0, 0,
               0, 1, 0, 0,
               0, 1, 0, 0, 
               0, 1, 0, 0, -1 };

int T6D2[] = { 0, 0, 0, 0,
               1, 1, 1, 1,
               0, 0, 0, 0,
               0, 0, 0, 0, -1 };

int T6D3[] = { 0, 1, 0, 0,
               0, 1, 0, 0,
               0, 1, 0, 0,
               0, 1, 0, 0, -1 };
  
int *setOfBlockArrays[] = {
  T0D0, T0D1, T0D2, T0D3,
  T1D0, T1D1, T1D2, T1D3,
  T2D0, T2D1, T2D2, T2D3,
  T3D0, T3D1, T3D2, T3D3,
  T4D0, T4D1, T4D2, T4D3,
  T5D0, T5D1, T5D2, T5D3,
  T6D0, T6D1, T6D2, T6D3,
};

void drawScreen(Matrix *screen, int wall_depth)
{
  int dy = screen->get_dy();
  int dx = screen->get_dx();
  int dw = wall_depth;
  int **array = screen->get_array();

  for (int y = 0; y < dy - dw + 1; y++) {
    for (int x = dw - 1; x < dx - dw + 1; x++) {
      if (array[y][x] == 0)
	      cout << "□ ";
      else if (array[y][x] == 1)
	      cout << "■ ";
      else if (array[y][x] == 10)
	      cout << "◈ ";
      else if (array[y][x] == 20)
	      cout << "★ ";
      else if (array[y][x] == 30)
	      cout << "● ";
      else if (array[y][x] == 40)
	      cout << "◆ ";
      else if (array[y][x] == 50)
	      cout << "▲ ";
      else if (array[y][x] == 60)
	      cout << "♣ ";
      else if (array[y][x] == 70)
	      cout << "♥ ";
      else
	      cout << "X ";
    }
    cout << endl;
  }
}
  
/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

#define SCREEN_DY  16
#define SCREEN_DX  14
#define SCREEN_DW  1

#define ARRAY_DY (SCREEN_DY + SCREEN_DW)
#define ARRAY_DX (SCREEN_DX + 2*SCREEN_DW)

int arrayScreen[ARRAY_DY][ARRAY_DX] = {
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,1,1 },
  { 1,1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1 },  
  { 1,1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1 }, 
  { 1,1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,1,1 }, 
};

int main(int argc, char *argv[]) {
  char key;
  int blkType;
  int top = 0, left = 4;

  // 랜덤 변수 
  srand((unsigned int) time(NULL)); // 이 함수는 main 함수에서 최초 1회만 호출할 것


  Matrix *setOfBlockObjects[7][4];
  int rxy = 0;
  for (int i = 0 ; i < 7 ; i++){
      for (int k = 0 ; k < 4 ; k++){
        if (i == 0) rxy = 2;
        else if (i <= 5 ) rxy = 3;
        else rxy = 4;
        
        setOfBlockObjects[i][k] = new Matrix((int *) setOfBlockArrays[4*i + k] , rxy , rxy);
      } 
  }
  
 

  blkType = rand() % MAX_BLK_TYPES; // 이 코드는 난수 발생이 필요할 때마다 호출할 것


  // 게임 화면 객체 활당 
  Matrix *iScreen = new Matrix((int *) arrayScreen, ARRAY_DY, ARRAY_DX); 
  Matrix *currBlk = new Matrix(setOfBlockObjects[blkType][0]); // 랜덤 테스트 객체 ( 현재 블럭 )


  Matrix *tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
  Matrix *tempBlk2 = tempBlk->add(currBlk);
  delete tempBlk2;

  Matrix *oScreen = new Matrix(iScreen);
  oScreen->paste(tempBlk, top, left);  
  delete tempBlk;

  drawScreen(oScreen, SCREEN_DW);
  delete oScreen;

  while ((key = getch()) != 'q') {
    
    switch (key) {
          case 'a': left--; break;
          case 'd': left++; break;
          case 's': top++; break;
          case 'w': break;
          case ' ': break;
          default: cout << "wrong key input" << endl;
        }
    tempBlk = iScreen -> clip(top , left , top + currBlk->get_dy(), left + currBlk->get_dx());
    tempBlk2 = tempBlk->add(currBlk);
    delete tempBlk; // when used end -> delete 

    if (tempBlk2 -> anyGreaterThan(1)){
        switch (key) {
        case 'a': left++; break;
        case 'd': left--; break;
        case 's': top--; break;
        case 'w': break; // rotate 
        case ' ': break;
        default: cout << "wrong key input" << endl;
      }
      delete tempBlk2; // when (if allocation) -> delete  
      tempBlk = iScreen -> clip(top , left , top + currBlk->get_dy(), left + currBlk->get_dx());
      tempBlk2 = tempBlk->add(currBlk);
      delete tempBlk; // when used end -> delete 
    }
    
    std::system("clear");
    oScreen = new Matrix(iScreen);  
    oScreen->paste(tempBlk2, top, left); 
    delete tempBlk2;
    drawScreen(oScreen, SCREEN_DW);
    delete oScreen;
    
  }




  // 메모리 할당 해제 
  delete iScreen;
  delete currBlk;

  // for (int i = 0 ; i < 7 ; i++){
  //   for (int k = 0 ; k < 4 ; k++){
  //     delete[] setOfBlockObjects[i][k]; 
  //   }
  // }

  cout << "(nAlloc, nFree) = (" << Matrix::get_nAlloc() << ',' << Matrix::get_nFree() << ")" << endl;  
    cout << blkType << "ry : " <<  rxy << "rx: " <<  rxy << endl;
  cout << "Program terminated!" << endl;

  return 0;
}



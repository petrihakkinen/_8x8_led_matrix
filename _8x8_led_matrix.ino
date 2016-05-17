#include "c64_char_rom.h"

PROGMEM prog_uchar hearts[] = {
  B00000000,
  B00000000,
  B00010100,
  B00011100,
  B00001000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00010100,
  B00111110,
  B00111110,
  B00011100,
  B00001000,
  B00000000,
  B00000000,
  B00110110,
  B01111111,
  B01111111,
  B01111111,
  B00111110,
  B00011100,
  B00001000,
  B00000000,
  B00000000,
  B00010100,
  B00111110,
  B00111110,
  B00011100,
  B00001000,
  B00000000,
  B00000000,
};

// pins connected to row anodes
int rows[] = { 9,7,6,8,2,5,3,4 };

// pins connected to column catodes
int cols[] = { 10,11,12,14,15,16,17,18 };

// single bitplane frame buffer
byte framebuf[8] = { 0,0,0,0,0,0,0,0 };
 
int cur = 0;  // current column for display()

void display() {
  // disable previous column
  digitalWrite(cols[cur], HIGH);
  
  cur = (cur + 1) % 8;
  
  // update row
  int x = 128 >> cur;
  for( int i = 0; i < 8; i++ ) {
    // chunky
    //digitalWrite(rows[i], framebuffer[i][cur] ? HIGH : LOW);
    
    // bitplane
    digitalWrite(rows[i], framebuf[i] & x ? HIGH : LOW);
  }

  // enable new column
  digitalWrite(cols[cur], LOW);
}

void clearScreen() {
  for( int y = 0; y < 8; ++y )
    framebuf[y] = 0;
}

void blitChar(int ch, int scrollX) {
  if( ch == 0 || ch == '#' )
    return;
  if( ch == 83 && scrollX < 0 )
    return;
    
  ch <<= 3;  // *8
  if( scrollX >= 0 ) {
    framebuf[0] |= pgm_read_byte_near(chardata + ch+0) << scrollX;
    framebuf[1] |= pgm_read_byte_near(chardata + ch+1) << scrollX;
    framebuf[2] |= pgm_read_byte_near(chardata + ch+2) << scrollX;
    framebuf[3] |= pgm_read_byte_near(chardata + ch+3) << scrollX;
    framebuf[4] |= pgm_read_byte_near(chardata + ch+4) << scrollX;
    framebuf[5] |= pgm_read_byte_near(chardata + ch+5) << scrollX;
    framebuf[6] |= pgm_read_byte_near(chardata + ch+6) << scrollX;
    framebuf[7] |= pgm_read_byte_near(chardata + ch+7) << scrollX;
  } else {
    scrollX = -scrollX;
    framebuf[0] |= pgm_read_byte_near(chardata + ch+0) >> scrollX;
    framebuf[1] |= pgm_read_byte_near(chardata + ch+1) >> scrollX;
    framebuf[2] |= pgm_read_byte_near(chardata + ch+2) >> scrollX;
    framebuf[3] |= pgm_read_byte_near(chardata + ch+3) >> scrollX;
    framebuf[4] |= pgm_read_byte_near(chardata + ch+4) >> scrollX;
    framebuf[5] |= pgm_read_byte_near(chardata + ch+5) >> scrollX;
    framebuf[6] |= pgm_read_byte_near(chardata + ch+6) >> scrollX;
    framebuf[7] |= pgm_read_byte_near(chardata + ch+7) >> scrollX;
  }
}

void blitHeart(int ch) {
  ch <<= 3;  // *8
  framebuf[0] = pgm_read_byte_near(hearts + ch+0);
  framebuf[1] = pgm_read_byte_near(hearts + ch+1);
  framebuf[2] = pgm_read_byte_near(hearts + ch+2);
  framebuf[3] = pgm_read_byte_near(hearts + ch+3);
  framebuf[4] = pgm_read_byte_near(hearts + ch+4);
  framebuf[5] = pgm_read_byte_near(hearts + ch+5);
  framebuf[6] = pgm_read_byte_near(hearts + ch+6);
  framebuf[7] = pgm_read_byte_near(hearts + ch+7);
}

//void sine() {
//  time = time + 1;    
//}

char text[] = "I HAVE AN IMPORTANT MESSAGE FOR YOU:   PETE % SAARA    THE END %   #";
int textPos = 0;
int scrollX = -8;
int time = 0;
int heartTimer = 0;

void setup() {
  for( int i = 0; i < 8; i++ ) { 
    pinMode(rows[i], OUTPUT);
    pinMode(cols[i], OUTPUT);
    digitalWrite(rows[i], LOW);
    digitalWrite(cols[i], HIGH);
  }
  
  // prepare text
  for( int i = 0; i < sizeof(text); i++ ) {
    int ch = text[i];
    if( ch >= 'A' && ch <= 'Z' )
      text[i] = ch - 'A' + 1;
    else if( ch == ' ' )
      text[i] = 0;
    else if( ch == '%' )
      text[i] = 83;
    else if( ch == ':' )
      text[i] = 58;
    else
      text[i] = ch;
  }
}

void resetScroller() {
  textPos = 0;
  scrollX = -8;
  time = 0;
  heartTimer = 0;
}

void scroller() {
  if( heartTimer == 0 ) {
    // update scroller
    time = (time + 1) & 32767;
        
    clearScreen();  
    blitChar(text[textPos], scrollX);
    blitChar(text[(textPos+1) % sizeof(text)], scrollX - 8);
        
    // update scrolling
    if( (time % 63) == 0 ) {
      scrollX++;
      if( scrollX == 8 ) {
        textPos = (textPos + 1) % sizeof(text);
        scrollX = 0;
      }

      if( text[textPos] == '#' )
      {
        // halt!
        for( int i = 0; i < 30000; i++ ) {
          clearScreen();
          display();
          delay(5000);
        }
      } 
      // start heart effect?      
      if( text[textPos] == 83 && scrollX == 0 )
        heartTimer = 1;
    }
  } else {
    // update heart
    blitHeart((heartTimer >> 7) % 4);
    heartTimer++;
    if( heartTimer == 11*128 ) {
      heartTimer = 0;
    }
  }
  
  display();
  delay(1);
}

void sinefill() {
  static int sineTimer = 0;
 
  float t = sineTimer * 0.01f;
  sineTimer++;
  
  clearScreen();
  for( int y = 0; y < 8; y++ ) {
    float x = (float)sin(t + y * 0.3f) * 0.9f;
    x += (float)sin((t + y * 0.3f) * 2.0f + 1.234f) * 0.45f;
    int ix = constrain((int)(x * 4 + 4), 0, 7);
    
    framebuf[y] = 1<<ix;
    framebuf[y] |= 1<<(ix-1);
    framebuf[y] |= 1<<(ix+1);    
  }
  
  display();
  delay(1);
}

void setPixel(int x, int y) {
  framebuf[y&7] |= (128>>x) & 255;
}

void clearPixel(int x, int y) {
  framebuf[y&7] &= ~((128>>x) & 255);
}

void boxes() {
  static int t = 0;
  t = (t+1) & 32767;
  
  clearScreen();
  
  static int lt = 0;
  static int dir = 1;
  if( (t & 63) == 0 ) {
    lt = lt + dir;
    if( lt == 4 && dir == 1 ) {
        dir = -1;
    }
    if( lt == 0 && dir == -1 ) {
        dir = 1;
    }
  }
  
  int s = lt; //(t >> 7) % 5;
  
  for( int y = 4-s; y < 4+s; y++ )
    for( int x = 4-s; x < 4+s; x++ )
      setPixel(x, y);

  s = s - 1;
  if( s >= 0 ) {
    for( int y = 4-s; y < 4+s; y++ )
      for( int x = 4-s; x < 4+s; x++ )
        clearPixel(x, y);
  }

  s = s - 1;
  if( s >= 0 ) {
    for( int y = 4-s; y < 4+s; y++ )
      for( int x = 4-s; x < 4+s; x++ )
        setPixel(x, y);
  }

  s = s - 1;
  if( s >= 0 ) {
    for( int y = 4-s; y < 4+s; y++ )
      for( int x = 4-s; x < 4+s; x++ )
        clearPixel(x, y);
  }
  
  display();
  delay(1);
}

int sx[] = { 5, 7, 1, 3, 6, 2, 0, 4 };
float sy[8] = { };
float sp[] = { 1, 1.3, 1.5, 0.96, 0.9, 1.1, 1.2, 0.8 };

void initStars() {
  float t[] = { -1, -0, -3, -5, -4, -2, -7, -6 };
  for( int i = 0; i < 8; i++ )
    sy[i] = t[i];
}

void stars() { 
  static int t = 0;
  t = (t+1) & 32767;
 
  if( (t & 31) == 0 ) {
    //clearScreen();
    
    for( int i = 0; i < 8; i++ )
    {
      sy[i] = sy[i] + sp[i]*0.5f;
      int y = (int)sy[i];        
      if( y >= 0 && y < 8 )
        setPixel(sx[i], y);
      y = y - 24;
      if( y >= 0 && y < 8 )
        clearPixel(sx[i], y);
    } 
  }

  display();
  delay(1);
}

void intro() {
  char text[] = "HELLO! MY NAME IS ARDUINO";
  
  for( int j = 0; j < sizeof(text)-1; j++ ) {
    clearScreen();
    display();
    delay(100);

    clearScreen();
    int ch = text[j];
    if( ch != ' ' ) {
      if( ch == '!' )
        ch = 33;
      else
        ch = ch - 'A' + 1;
      blitChar(ch, 0);
    }
    for( int i = 0; i < 400; i++ ) {
      display();
      delay(1);
    }
  }
}

void loop() {
  intro();
  clearScreen();
  display();
  delay(1000);
  for( int i = 0; i < 2790; i++ )
    boxes();
  initStars();
  for( int i = 0; i < 4000; i++ )
    stars();
  clearScreen();
  display();
  delay(1000);
  delay(200);
  for( int i = 0; i < 2000; i++ )
    sinefill();
  clearScreen();
  display();
  delay(1000);
  delay(200);
  resetScroller();
  for( int i = 0; i < 35000; i++ )
    scroller();   
  delay(4000);
}

#line 1 "E:/mvi/Programming/GitHub/LED-Cube-8x8x8/software_code/cube8_core.c"
#line 22 "E:/mvi/Programming/GitHub/LED-Cube-8x8x8/software_code/cube8_core.c"
volatile char cube[ 8 ][ 8 ][ 8 ];
volatile char buffer[ 8 ][ 8 ][ 8 ];
char effect=1, fl_button=0, fl_start=0, fl_reset=0;
const double PI=3.14159;



void interrupt(void) {
 INTCON.GIE=0x00;

 if (INTCON.INT0IF) {
 if (effect< 22 ) {
 effect++;
 } else {
 effect=1;
 }
 delay_ms(1500);
 fl_button=1;
 fl_reset=1;
 INTCON.INT0IF=0x00;

 }


 if (INTCON.T0IF) {

 TMR0L=0x00;
 TMR0H=0x00;
 INTCON.T0IF=0x00;
 }
 INTCON.GIE=0xff;
}






char inrange(char z, char y, char x) {
 if (x>=0 && x< 8  && y>=0 && y< 8  && z>=0 && z< 8 ) {
 return 1;
 } else {

 return 0;
 }
}


void delay_msec(int speed) {
 int delay;
 for (delay=1; delay<=speed; delay++) {
 delay_ms(1);
 }
}


void cube_draw(int speed, int iterations) {
 int x, y, z, i;

 for (i=0; i<iterations; i++) {

 for (z=0; z< 8 ; z++) {
 for (y=( 8  - 1); y>=0; y--) {
 for (x=( 8  - 1); x>=0; x--) {
  PORTA.F1 =cube[z][y][x];

  PORTA.F0 =1;
  PORTA.F0 =0;
 }
 }


  PORTA.F2 =1;
 switch (z) {
 case 0:
 PORTB=0b10000000;
 PORTC=0b00000000;
 break;
 case 1:
 PORTB=0b01000000;
 PORTC=0b00000000;
 break;
 case 2:
 PORTB=0b00100000;
 PORTC=0b00000000;
 break;
 case 3:
 PORTB=0b00010000;
 PORTC=0b00000000;
 break;
 case 4:
 PORTB=0b00001000;
 PORTC=0b00000000;
 break;
 case 5:
 PORTB=0b00000100;
 PORTC=0b00000000;
 break;
 case 6:
 PORTB=0b00000010;
 PORTC=0b00000000;
 break;
 case 7:
 PORTB=0b00000000;
 PORTC=0b00000100;
 break;
 default:
 break;
 }

 delay_msec(speed);

  PORTA.F2 =0;
 PORTB=0b00000000;
 PORTC=0b00000000;


 }
 }
}


void setvoxel(char z, char y, char x) {
 if (inrange(z, y, x)) {
 cube[z][y][x]=0xff;
 }
}


void clrvoxel(char z, char y, char x) {
 if (inrange(z, y, x))
 cube[z][y][x]=0x00;
}


char getvoxel(char z, char y, char x) {
 if (inrange(z, y, x)) {
 if (cube[z][y][x]==0xff) {
 return 1;
 } else {
 return 0;
 }
 } else {
 return 0;
 }
}


void altervoxel(char z, char y, char x, char state) {
 if (inrange(z, y, x)) {
 if (state==1) {
 setvoxel(z, y, x);
 }
 if (state==0) {
 clrvoxel(z, y, x);
 }
 }
}



void flpvoxel(char z, char y, char x) {
 if (inrange(z, y, x))
 cube[z][y][x] ^= cube[z][y][x];
}


void tmpsetvoxel(char z, char y, char x) {
 if (inrange(z, y, x))
 buffer[z][y][x]=0xff;
}


void tmpclrvoxel(int z, int y, int x) {
 if (inrange(z, y, x))
 buffer[z][y][x]=0x00;
}


void setplane_x(char x) {
 char z, y;

 if (x>=0 && x< 8 ) {
 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 cube[z][y][x]=0xff;
 }
 }
 }
}


void clrplane_x(char x) {
 char z, y;

 if (x>=0 && x< 8 ) {
 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 cube[z][y][x]=0x00;
 }
 }
 }
}


void setplane_y(char y) {
 char z, x;

 if (y>=0 && y< 8 ) {
 for (z=0; z< 8 ; z++) {
 for (x=0; x< 8 ; x++) {
 cube[z][y][x]=0xff;
 }
 }
 }
}


void clrplane_y(char y) {
 char z, x;

 if (y>=0 && y< 8 ) {
 for (z=0; z< 8 ; z++) {
 for (x=0; x< 8 ; x++) {
 cube[z][y][x]=0x00;
 }
 }
 }
}




void fill_layer(char layer, char colour) {
 char y, x;

 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 cube[layer][y][x]=colour;
 }
 }
}

void tmpfill_layer(char layer, char colour) {
 char y, x;

 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 buffer[layer][y][x]=colour;
 }
 }
}




void fill_cube(char colour) {
 char z, y, x;

 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 cube[z][y][x]=colour;
 }
 }
 }
}


void setplane(char axis, char i) {
 switch (axis) {
 case 1:
 fill_layer(i, 0xff);
 break;
 case 2:
 setplane_y(i);
 break;
 case 3:
 setplane_x(i);
 break;
 }
}


void clrplane(char axis, char i) {
 switch (axis) {
 case 1:
 fill_layer(i, 0x00);
 break;
 case 2:
 clrplane_y(i);
 break;
 case 3:
 clrplane_x(i);
 break;
 }
}



void argorder(char ix1, char ix2, char *ox1, char *ox2) {
 if (ix1>ix2) {
 char tmp;
 tmp=ix1;
 ix1=ix2;
 ix2=tmp;
 }
 *ox1=ix1;
 *ox2=ix2;
}


void box_filled(char z1, char y1, char x1, char z2, char y2, char x2) {
 char iz, iy, ix;

 argorder(x1, x2, &x1, &x2);
 argorder(y1, y2, &y1, &y2);
 argorder(z1, z2, &z1, &z2);

 for (iz=z1; iz<=z2; iz++) {
 for (iy=y1; iy<=y2; iy++) {
 for (ix=x1; ix<=x2; ix++) {
 cube[iz][iy][ix]=0xff;
 }
 }
 }
}


void box_walls(char z1, char y1, char x1, char z2, char y2, char x2) {
 char iz, iy, ix;

 argorder(x1, x2, &x1, &x2);
 argorder(y1, y2, &y1, &y2);
 argorder(z1, z2, &z1, &z2);

 for (iz=z1; iz<=z2; iz++) {
 for (iy=y1; iy<=y2; iy++) {
 for (ix=x1; ix<=x2; ix++) {
 if (iz==z1 || iz==z2 || iy==y1 || iy==y2 || ix==x1 || ix==x2) {
 cube[iz][iy][ix]=0xff;
 } else {
 cube[iz][iy][ix]=0x00;
 }
 }
 }
 }
}


void box_wireframe(char z1, char y1, char x1, char z2, char y2, char x2) {
 char iz, iy, ix;

 argorder(x1, x2, &x1, &x2);
 argorder(y1, y2, &y1, &y2);
 argorder(z1, z2, &z1, &z2);

 for (iz=z1; iz<=z2; iz++) {
 for (iy=y1; iy<=y2; iy++) {
 for (ix=x1; ix<=x2; ix++) {
 if ((iz>=z1 && iz<=z2 && iy==y1 && ix==x1) ||
 (iz>=z1 && iz<=z2 && iy==y2 && ix==x1) ||
 (iz>=z1 && iz<=z2 && iy==y1 && ix==x2) ||
 (iz>=z1 && iz<=z2 && iy==y2 && ix==x2) ||

 (iz==z1 && iy==y1 && ix>=x1 && ix<=x2) ||
 (iz==z1 && iy==y2 && ix>=x1 && ix<=x2) ||
 (iz==z2 && iy==y1 && ix>=x1 && ix<=x2) ||
 (iz==z2 && iy==y2 && ix>=x1 && ix<=x2) ||

 (iz==z1 && iy>=y1 && iy<=y2 && ix==x1) ||
 (iz==z1 && iy>=y1 && iy<=y2 && ix==x2) ||
 (iz==z2 && iy>=y1 && iy<=y2 && ix==x1) ||
 (iz==z2 && iy>=y1 && iy<=y2 && ix==x2))
 {
 cube[iz][iy][ix]=0xff;
 } else {
 cube[iz][iy][ix]=0x00;
 }
 }
 }
 }
}



void line(char z1, char y1, char x1, char z2, char y2, char x2) {
 char x, y, z, last_y, last_z;
 float xy, xz;



 if (inrange(z1, y1, x1) & inrange(z2, y2, x2)) {


 if (x1>x2) {
 char tmp;
 tmp=x2;
 x2=x1;
 x1=tmp;
 tmp=y2;
 y2=y1;
 y1=tmp;
 tmp=z2;
 z2=z1;
 z1=tmp;
 }

 if (y1>y2) {
 xy=(float) (y1 - y2) / (float) (x2 - x1);
 last_y=y2;
 } else {
 xy=(float) (y2 - y1) / (float) (x2 - x1);
 last_y=y1;
 }

 if (z1>z2) {
 xz=(float) (z1 - z2) / (float) (x2 - x1);
 last_z=z2;
 } else {
 xz=(float) (z2 - z1) / (float) (x2 - x1);
 last_z=z1;
 }


 for (x=x1; x<=x2; x++) {
 y=(xy * (x - x1)) + y1;
 z=(xz * (x - x1)) + z1;
 setvoxel(z, y, x);
 }
 }
}


void cube2buffer(void) {
 unsigned char z, y, x;

 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 buffer[z][y][x]=cube[z][y][x];
 }
 }
 }
}


void buffer2cube(void) {
 unsigned char z, y, x;

 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 cube[z][y][x]=buffer[z][y][x];
 }
 }
 }
}


void mirror_z(void) {
 char z, y, x;
 cube2buffer();
 fill_cube(0x00);

 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 if (buffer[z][y][x]==0xff)
 setvoxel( 8  - 1 - z, y, x);
 }
 }
 }
}


void mirror_y(void) {
 char z, y, x;
 cube2buffer();
 fill_cube(0x00);

 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 if (buffer[z][y][x]==0xff)
 setvoxel(z,  8  - 1 - y, x);
 }
 }
 }
}


void mirror_x(void) {
 char z, y, x;
 cube2buffer();
 fill_cube(0x00);

 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 if (buffer[z][y][x]==0xff)
 setvoxel(z, y,  8  - 1 - x);
 }
 }
 }
}




void shift(char axis, char direction) {
 char z, y, x, ii, iii, state;

 if (axis==1)
 {
 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 if (direction==0)
 {
 ii=z;
 iii=ii + 1;
 } else {
 ii=( 8  - 1) - z;
 iii=ii - 1;
 }
 state=getvoxel(iii, y, x);
 altervoxel(ii, y, x, state);
 }
 }
 }
 }
 if (axis==2)
 {
 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 if (direction==0)
 {
 ii=( 8  - 1) - y;
 iii=ii - 1;
 } else {
 ii=y;
 iii=ii + 1;
 }
 state=getvoxel(z, iii, x);
 altervoxel(z, ii, x, state);
 }
 }
 }
 }
 if (axis==3)
 {
 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 if (direction==0)
 {
 ii=x;
 iii=ii + 1;
 } else {
 ii=( 8  - 1) - x;
 iii=ii - 1;
 }
 state=getvoxel(z, y, iii);
 altervoxel(z, y, ii, state);
 }
 }
 }
 }
}


volatile const unsigned char font8eng[728][8]={
 {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x5f, 0x5f, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00},
 {0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14, 0x00, 0x00},
 {0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12, 0x00, 0x00},
 {0x00, 0x23, 0x13, 0x08, 0x64, 0x62, 0x00, 0x00},
 {0x00, 0x36, 0x49, 0x55, 0x22, 0x50, 0x00, 0x00},
 {0x00, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x1c, 0x22, 0x41, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x41, 0x22, 0x1c, 0x00, 0x00, 0x00},
 {0x00, 0x14, 0x08, 0x3e, 0x08, 0x14, 0x00, 0x00},
 {0x00, 0x08, 0x08, 0x3e, 0x08, 0x08, 0x00, 0x00},
 {0x00, 0x00, 0x50, 0x30, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00},
 {0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00},
 {0x00, 0x3e, 0x51, 0x49, 0x45, 0x3e, 0x00, 0x00},
 {0x00, 0x00, 0x42, 0x7f, 0x40, 0x00, 0x00, 0x00},
 {0x00, 0x42, 0x61, 0x51, 0x49, 0x46, 0x00, 0x00},
 {0x00, 0x21, 0x41, 0x45, 0x4b, 0x31, 0x00, 0x00},
 {0x00, 0x18, 0x14, 0x12, 0x7f, 0x10, 0x00, 0x00},
 {0x00, 0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00},
 {0x00, 0x3c, 0x4a, 0x49, 0x49, 0x30, 0x00, 0x00},
 {0x00, 0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00},
 {0x00, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00},
 {0x00, 0x06, 0x49, 0x49, 0x29, 0x1e, 0x00, 0x00},
 {0x00, 0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x56, 0x36, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00, 0x00},
 {0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 0x00},
 {0x00, 0x00, 0x41, 0x22, 0x14, 0x08, 0x00, 0x00},
 {0x00, 0x02, 0x01, 0x51, 0x09, 0x06, 0x00, 0x00},
 {0x00, 0x32, 0x49, 0x79, 0x41, 0x3e, 0x00, 0x00},
 {0x00, 0x7e, 0x11, 0x11, 0x11, 0x7e, 0x00, 0x00},
 {0x00, 0x7f, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00},
 {0x00, 0x3e, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00},
 {0x00, 0x7f, 0x41, 0x41, 0x22, 0x1c, 0x00, 0x00},
 {0x00, 0x7f, 0x49, 0x49, 0x49, 0x41, 0x00, 0x00},
 {0x00, 0x7f, 0x09, 0x09, 0x09, 0x01, 0x00, 0x00},
 {0x00, 0x3e, 0x41, 0x49, 0x49, 0x7a, 0x00, 0x00},
 {0x00, 0x7f, 0x08, 0x08, 0x08, 0x7f, 0x00, 0x00},
 {0x00, 0x00, 0x41, 0x7f, 0x41, 0x00, 0x00, 0x00},
 {0x00, 0x20, 0x40, 0x41, 0x3f, 0x01, 0x00, 0x00},
 {0x00, 0x7f, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00},
 {0x00, 0x7f, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00},
 {0x00, 0x7f, 0x02, 0x0c, 0x02, 0x7f, 0x00, 0x00},
 {0x00, 0x7f, 0x04, 0x08, 0x10, 0x7f, 0x00, 0x00},
 {0x00, 0x3e, 0x41, 0x41, 0x41, 0x3e, 0x00, 0x00},
 {0x00, 0x7f, 0x09, 0x09, 0x09, 0x06, 0x00, 0x00},
 {0x00, 0x3e, 0x41, 0x51, 0x21, 0x5e, 0x00, 0x00},
 {0x00, 0x7f, 0x09, 0x19, 0x29, 0x46, 0x00, 0x00},
 {0x00, 0x46, 0x49, 0x49, 0x49, 0x31, 0x00, 0x00},
 {0x00, 0x01, 0x01, 0x7f, 0x01, 0x01, 0x00, 0x00},
 {0x00, 0x3f, 0x40, 0x40, 0x40, 0x3f, 0x00, 0x00},
 {0x00, 0x1f, 0x20, 0x40, 0x20, 0x1f, 0x00, 0x00},
 {0x00, 0x3f, 0x40, 0x38, 0x40, 0x3f, 0x00, 0x00},
 {0x00, 0x63, 0x14, 0x08, 0x14, 0x63, 0x00, 0x00},
 {0x00, 0x07, 0x08, 0x70, 0x08, 0x07, 0x00, 0x00},
 {0x00, 0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 0x00},
 {0x00, 0x00, 0x7f, 0x41, 0x41, 0x00, 0x00, 0x00},
 {0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00},
 {0x00, 0x00, 0x41, 0x41, 0x7f, 0x00, 0x00, 0x00},
 {0x00, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00, 0x00},
 {0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00},
 {0x00, 0x00, 0x01, 0x02, 0x04, 0x00, 0x00, 0x00},
 {0x00, 0x20, 0x54, 0x54, 0x54, 0x78, 0x00, 0x00},
 {0x00, 0x7f, 0x48, 0x44, 0x44, 0x38, 0x00, 0x00},
 {0x00, 0x38, 0x44, 0x44, 0x44, 0x20, 0x00, 0x00},
 {0x00, 0x38, 0x44, 0x44, 0x48, 0x7f, 0x00, 0x00},
 {0x00, 0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 0x00},
 {0x00, 0x08, 0x7e, 0x09, 0x01, 0x02, 0x00, 0x00},
 {0x00, 0x0c, 0x52, 0x52, 0x52, 0x3e, 0x00, 0x00},
 {0x00, 0x7f, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00},
 {0x00, 0x00, 0x44, 0x7d, 0x40, 0x00, 0x00, 0x00},
 {0x00, 0x20, 0x40, 0x44, 0x3d, 0x00, 0x00, 0x00},
 {0x00, 0x7f, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x41, 0x7f, 0x40, 0x00, 0x00, 0x00},
 {0x00, 0x7c, 0x04, 0x18, 0x04, 0x78, 0x00, 0x00},
 {0x00, 0x7c, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00},
 {0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00},
 {0x00, 0x7c, 0x14, 0x14, 0x14, 0x08, 0x00, 0x00},
 {0x00, 0x08, 0x14, 0x14, 0x18, 0x7c, 0x00, 0x00},
 {0x00, 0x7c, 0x08, 0x04, 0x04, 0x08, 0x00, 0x00},
 {0x00, 0x48, 0x54, 0x54, 0x54, 0x20, 0x00, 0x00},
 {0x00, 0x04, 0x3f, 0x44, 0x40, 0x20, 0x00, 0x00},
 {0x00, 0x3c, 0x40, 0x40, 0x20, 0x7c, 0x00, 0x00},
 {0x00, 0x1c, 0x20, 0x40, 0x20, 0x1c, 0x00, 0x00},
 {0x00, 0x3c, 0x40, 0x30, 0x40, 0x3c, 0x00, 0x00},
 {0x00, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00, 0x00},
 {0x00, 0x0c, 0x50, 0x50, 0x50, 0x3c, 0x00, 0x00},
 {0x00, 0x44, 0x64, 0x54, 0x4c, 0x44, 0x00, 0x00}
};
#line 881 "E:/mvi/Programming/GitHub/LED-Cube-8x8x8/software_code/cube8_core.c"
void font_getchar(char chr, unsigned char dst[]) {
 int i;
 chr -= 32;
#line 893 "E:/mvi/Programming/GitHub/LED-Cube-8x8x8/software_code/cube8_core.c"
 if ( 8 ==8) {
 for (i=0; i< 8 ; i++) {
 dst[i]=font8eng[chr][i];
 }
 }
}
#line 913 "E:/mvi/Programming/GitHub/LED-Cube-8x8x8/software_code/cube8_core.c"
void effect_rain(int iterations) {
 int i, ii;
 int rnd_x;
 int rnd_y;
 int rnd_num;
 fill_cube(0x00);

 for (ii=0; ii<iterations; ii++) {
 rnd_num=rand() % ( 8  / 2);
 for (i=0; i<rnd_num; i++) {
 rnd_x=rand() %  8 ;
 rnd_y=rand() %  8 ;
 setvoxel(0, rnd_y, rnd_x);
 }
 cube_draw(1, 10);
 shift(1, 1);
 if (fl_button) { fl_button=0; return; }
 }
}


void effect_planboing(char axis, int speed) {
 int i;

 for (i=0; i< 8 ; i++) {
 fill_cube(0x00);
 setplane(axis, i);
 cube_draw(1, speed);
 if (fl_button==1) {
 fl_button=0;
 return;
 }
 }
 for (i=( 8  - 1); i>=0; i--) {
 fill_cube(0x00);
 setplane(axis, i);
 cube_draw(1, speed);
 if (fl_button) { fl_button=0; return; }
 }
}


void effect_box_shrink_grow(void) {
 int x, i, ii, xyz;
 int iterations=1;
 int flip=ii & ( 8  / 2);
 int delay=2;

 for (ii=0; ii< 8 ; ii++) {
 for (x=0; x<iterations; x++) {
 for (i=0; i<( 8  * 2); i++) {
 xyz=( 8  - 1) - i;
 if (i>( 8  - 1))
 xyz=i -  8 ;

 fill_cube(0x00);
 cube_draw(1, 1);
 if (fl_button) { fl_button=0; return; }

 box_wireframe(0, 0, 0, xyz, xyz, xyz);

 if (flip>0)
 mirror_z();
 if (ii==( 8  - 3) || ii==( 8  - 1))
 mirror_y();
 if (ii==( 8  - 2) || ii==( 8  - 1))
 mirror_x();

 cube_draw(1, delay);
 fill_cube(0x00);
 if (fl_button) { fl_button=0; return; }
 }
 }
 }
}


void effect_box_woopwoop(int delay, int grow) {
 int i, ii;
 char ci= 8  / 2;
 fill_cube(0x00);

 for (i=0; i<ci; i++) {
 ii=i;
 if (grow>0)
 ii=(ci - 1) - i;

 box_wireframe(ci + ii, ci + ii, ci + ii, (ci - 1) - ii, (ci - 1) - ii, (ci - 1) - ii);
 cube_draw(1, delay);
 fill_cube(0x00);
 if (fl_button) { fl_button=0; return; }
 }
}



void sendvoxel_axis(unsigned char z, unsigned char y, unsigned char x, char axis, int delay) {
 int i, ii;

 if (axis==1) {
 for (i=0; i< 8 ; i++) {
 if (z==( 8  - 1)) {
 ii=( 8  - 1) - i;
 clrvoxel(ii + 1, y, x);
 } else {
 ii=i;
 clrvoxel(ii - 1, y, x);
 }
 setvoxel(ii, y, x);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }
 }

 if (axis==2) {
 for (i=0; i< 8 ; i++) {
 if (y==( 8  - 1)) {
 ii=( 8  - 1) - i;
 clrvoxel(z, ii + 1, x);
 } else {
 ii=i;
 clrvoxel(z, ii - 1, x);
 }
 setvoxel(z, ii, x);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }
 }

 if (axis==3) {
 for (i=0; i< 8 ; i++) {
 if (x==( 8  - 1)) {
 ii=( 8  - 1) - i;
 clrvoxel(z, y, ii + 1);
 } else {
 ii=i;
 clrvoxel(z, y, ii - 1);
 }
 setvoxel(z, y, ii);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }
 }
}



void effect_sendplane_rand_z(unsigned char z, int delay, int wait) {
 unsigned char x, y, axis=1, loop= 8  * 2;
 fill_cube(0x00);
 fill_layer(z, 0xff);


 while (loop) {
 x=rand() %  8 ;
 y=rand() %  8 ;
 if (getvoxel(z, y, x)) {

 sendvoxel_axis(z, y, x, axis, delay);
 cube_draw(1, wait);
 if (fl_button) { fl_button=0; return; }
 loop--;
 }
 }
}




void effect_sendvoxels_rand_axis(int iterations, char axis, char delay, char wait) {
 unsigned char x, y, z, i, last_x=0, last_y=0, last_z=0;
 fill_cube(0x00);

 if (axis==1)
 {

 for (x=0; x< 8 ; x++) {
 for (y=0; y< 8 ; y++) {


 setvoxel(((rand() % 2)*( 8  - 1)), y, x);
 }
 }
 for (i=0; i<iterations; i++) {

 x=rand() %  8 ;
 y=rand() %  8 ;

 if (y != last_y && x != last_x) {

 if (getvoxel(0, y, x)) {

 sendvoxel_axis(0, y, x, axis, delay);
 } else {

 sendvoxel_axis(( 8  - 1), y, x, axis, delay);
 }
 cube_draw(1, wait);
 if (fl_button) { fl_button=0; return; }

 last_y=y;
 last_x=x;
 }
 }
 }

 if (axis==2) {

 for (x=0; x< 8 ; x++) {
 for (y=0; y< 8 ; y++) {


 setvoxel(y, ((rand() % 2)*( 8  - 1)), x);
 }
 }
 for (i=0; i<iterations; i++) {

 x=rand() %  8 ;
 y=rand() %  8 ;

 if (y != last_y && x != last_x) {

 if (getvoxel(y, 0, x)) {

 sendvoxel_axis(y, 0, x, axis, delay);
 } else {

 sendvoxel_axis(y, ( 8  - 1), x, axis, delay);
 }
 cube_draw(1, wait);
 if (fl_button) { fl_button=0; return; }

 last_y=y;
 last_x=x;
 }
 }
 }

 if (axis==3) {

 for (x=0; x< 8 ; x++) {
 for (y=0; y< 8 ; y++) {


 setvoxel(y, x, ((rand() % 2)*( 8  - 1)));
 }
 }
 for (i=0; i<iterations; i++) {

 x=rand() %  8 ;
 y=rand() %  8 ;

 if (y != last_y && x != last_x) {

 if (getvoxel(y, x, 0)) {

 sendvoxel_axis(y, x, 0, axis, delay);
 } else {

 sendvoxel_axis(y, x, ( 8  - 1), axis, delay);
 }
 cube_draw(1, wait);
 if (fl_button) { fl_button=0; return; }

 last_y=y;
 last_x=x;
 }
 }
 }
}


void effect_stringfly(char *str) {
 int x, y, z, i, ii;
 unsigned char chr[];
 fill_cube(0x00);

 while (*str) {
 font_getchar(*str++, chr);

 for (z=( 8  - 1); z>=0; z--) {
 for (y=( 8  - 1); y>=0; y--) {
 if (chr[z] >> y & 0x01) {
 setvoxel(y, z, 0);
 }
 }
 }



 for (i=0; i< 8 ; i++) {
 cube_draw(1, 3);
 if (fl_button) { fl_button=0; return; }
 shift(3, 1);
 }
 delay_ms(100);
 }
}


void effect_boingboing(int iterations, char delay, unsigned char mode, unsigned char drawmode) {
 int x, y, z;
 int dx, dy, dz;
 int lol, i;
 unsigned char crash_x, crash_y, crash_z;
 int snake[ 8 ][3];

 fill_cube(0x00);

 y=rand() %  8 ;
 x=rand() %  8 ;
 z=rand() %  8 ;


 for (i=0; i< 8 ; i++) {
 snake[i][0]=x;
 snake[i][1]=y;
 snake[i][2]=z;
 }

 dx=1;
 dy=1;
 dz=1;

 while (iterations) {
 crash_x=0;
 crash_y=0;
 crash_z=0;


 if (rand() % 3==0) {

 lol=rand() % 3;
 if (lol==0)
 dx=rand() % 3 - 1;

 if (lol==1)
 dy=rand() % 3 - 1;

 if (lol==2)
 dz=rand() % 3 - 1;
 }



 if (dx==-1 && x==0) {
 crash_x=0x01;
 if (rand() % 3==1) {
 dx=1;
 } else {
 dx=0;
 }
 }


 if (dy==-1 && y==0) {
 crash_y=0x01;
 if (rand() % 3==1) {
 dy=1;
 } else {
 dy=0;
 }
 }


 if (dz==-1 && z==0) {
 crash_z=0x01;
 if (rand() % 3==1) {
 dz=1;
 } else {
 dz=0;
 }
 }


 if (dx==1 && x==( 8  - 1)) {
 crash_x=0x01;
 if (rand() % 3==1) {
 dx=-1;
 } else {
 dx=0;
 }
 }


 if (dy==1 && y==( 8  - 1)) {
 crash_y=0x01;
 if (rand() % 3==1) {
 dy=-1;
 } else {
 dy=0;
 }
 }


 if (dz==1 && z==( 8  - 1)) {
 crash_z=0x01;
 if (rand() % 3==1) {
 dz=-1;
 } else {
 dz=0;
 }
 }


 if (mode | 0x01) {
 if (crash_x) {
 if (dy==0) {
 if (y==( 8  - 1)) {
 dy=-1;
 } else if (y==0) {
 dy=+1;
 } else {
 if (rand() % 2==0) {
 dy=-1;
 } else {
 dy=1;
 }
 }
 }
 if (dz==0) {
 if (z==( 8  - 1)) {
 dz=-1;
 } else if (z==0) {
 dz=1;
 } else {
 if (rand() % 2==0) {
 dz=-1;
 } else {
 dz=1;
 }
 }
 }
 }

 if (crash_y) {
 if (dx==0) {
 if (x==( 8  - 1)) {
 dx=-1;
 } else if (x==0) {
 dx=1;
 } else {
 if (rand() % 2==0) {
 dx=-1;
 } else {
 dx=1;
 }
 }
 }
 if (dz==0) {
 if (z==3) {
 dz=-1;
 } else if (z==0) {
 dz=1;
 } else {
 if (rand() % 2==0) {
 dz=-1;
 } else {
 dz=1;
 }
 }
 }
 }

 if (crash_z) {
 if (dy==0) {
 if (y==( 8  - 1)) {
 dy=-1;
 } else if (y==0) {
 dy=1;
 } else {
 if (rand() % 2==0) {
 dy=-1;
 } else {
 dy=1;
 }
 }
 }
 if (dx==0) {
 if (x==( 8  - 1)) {
 dx=-1;
 } else if (x==0) {
 dx=1;
 } else {
 if (rand() % 2==0) {
 dx=-1;
 } else {
 dx=1;
 }
 }
 }
 }
 }


 if (mode | 0x02) {
 if (
 (x==0 && y==0 && z==0) ||
 (x==0 && y==0 && z==( 8  - 1)) ||
 (x==0 && y==( 8  - 1) && z==0) ||
 (x==0 && y==( 8  - 1) && z==( 8  - 1)) ||
 (x==( 8  - 1) && y==0 && z==0) ||
 (x==( 8  - 1) && y==0 && z==( 8  - 1)) ||
 (x==( 8  - 1) && y==( 8  - 1) && z==0) ||
 (x==( 8  - 1) && y==( 8  - 1) && z==( 8  - 1))
 ) {







 lol=rand() % 3;
 if (lol==0)
 dx=0;

 if (lol==1)
 dy=0;

 if (lol==2)
 dz=0;
 }
 }


 if (x==0 && dx==-1)
 dx=1;

 if (y==0 && dy==-1)
 dy=1;

 if (z==0 && dz==-1)
 dz=1;

 if (x==( 8  - 1) && dx==1)
 dx=-1;

 if (y==( 8  - 1) && dy==1)
 dy=-1;

 if (z==( 8  - 1) && dz==1)
 dz=-1;



 x=x + dx;
 y=y + dy;
 z=z + dz;


 if (drawmode==0x01)
 {
 setvoxel(z, y, x);
 cube_draw(1, delay);
 clrvoxel(z, y, x);
 if (fl_button) { fl_button=0; return; }
 }

 if (drawmode==0x02)
 {

 setvoxel(z, y, x);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }

 if (drawmode==0x03)
 {
 for (i=( 8  - 1); i>=0; i--) {
 snake[i][0]=snake[i - 1][0];
 snake[i][1]=snake[i - 1][1];
 snake[i][2]=snake[i - 1][2];
 }
 snake[0][0]=x;
 snake[0][1]=y;
 snake[0][2]=z;

 for (i=0; i< 8 ; i++) {
 setvoxel(snake[i][2], snake[i][1], snake[i][0]);
 }
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 for (i=0; i< 8 ; i++) {
 clrvoxel(snake[i][2], snake[i][1], snake[i][0]);
 }
 }
 iterations--;
 }
}


void draw_positions_axis(char axis, unsigned char positions[ 8 * 8 ], int invert) {
 int x, y, p;
 fill_cube(0x00);

 for (x=0; x< 8 ; x++) {
 for (y=0; y< 8 ; y++) {
 if (invert) {
 p=( 8  - 1) - positions[x *  8  + y];
 } else {
 p=positions[x *  8  + y];
 }
 if (axis==1)
 setvoxel(p, y, x);

 if (axis==2)
 setvoxel(y, p, x);

 if (axis==3)
 setvoxel(x, y, p);
 }
 }
}


void effect_z_updown_move(unsigned char positions[ 8 * 8 ], unsigned char destinations[ 8 * 8 ], char axis) {
 int px;

 for (px=0; px<( 8  *  8 ); px++) {
 if (positions[px]<destinations[px]) {
 positions[px]++;
 }
 if (positions[px]>destinations[px]) {
 positions[px]--;
 }
 }
 draw_positions_axis(1, positions, 0);
}



void effect_axis_updown_randsuspend(char axis, char delay, int sleep, char invert) {
 unsigned char positions[ 8  *  8 ];
 unsigned char destinations[ 8  *  8 ];
 int i, px;
 fill_cube(0x00);


 for (i=0; i<( 8  *  8 ); i++) {
 positions[i]=0;
 destinations[i]=rand() %  8 ;
 }


 for (i=0; i< 8 ; i++) {

 for (px=0; px<( 8  *  8 ); px++) {
 if (positions[px]<destinations[px]) {
 positions[px]++;
 }
 }

 draw_positions_axis(axis, positions, invert);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }


 for (i=0; i<( 8  *  8 ); i++) {
 destinations[i]= 8  - 1;
 }


 cube_draw(1, sleep);


 for (i=0; i< 8 ; i++) {
 for (px=0; px<( 8  *  8 ); px++) {
 if (positions[px]<destinations[px]) {
 positions[px]++;
 }
 if (positions[px]>destinations[px]) {
 positions[px]--;
 }
 }
 draw_positions_axis(axis, positions, invert);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }


 cube_draw(1, sleep * 2);
}



void effect_z_updown(int iterations) {
 unsigned char positions[ 8  *  8 ];
 unsigned char destinations[ 8  *  8 ];
 int i, y, move, delay;

 for (i=0; i<( 8  *  8 ); i++) {
 positions[i]= 8  / 2;
 destinations[i]=rand() %  8 ;
 }
 for (i=0; i< 8 ; i++) {
 effect_z_updown_move(positions, destinations, 1);
 cube_draw(1, 4);
 if (fl_button) { fl_button=0; return; }
 }
 for (i=0; i<iterations; i++) {
 for (move=0; move< 8 ; move++) {
 effect_z_updown_move(positions, destinations, 1);
 cube_draw(1, 4);
 }
 cube_draw(1, 100);
 if (fl_button) { fl_button=0; return; }
 for (y=0; y<( 8  *  8  / 2); y++) {
 destinations[rand() % ( 8  *  8 )]=rand() %  8 ;
 }
 }
}


void effect_boxside_randsend_parallel(char axis, char origin, char delay, char mode) {
 char i;
 char done;
 unsigned char cubepos[ 8  *  8 ];
 unsigned char pos[ 8  *  8 ];
 char notdone=1;
 char notdone2=1;
 char sent=0;
 fill_cube(0x00);

 for (i=0; i<( 8  *  8 ); i++) {
 pos[i]=0;
 }

 while (notdone) {
 if (mode==1) {
 notdone2=1;
 while (notdone2 && sent<( 8  *  8 )) {
 i=rand() % ( 8  *  8 );
 if (pos[i]==0) {
 sent++;
 pos[i] += 1;
 notdone2=0;
 }
 }
 } else if (mode==2) {
 if (sent<( 8  *  8 )) {
 pos[sent] += 1;
 sent++;
 }
 }
 done=0;
 for (i=0; i<( 8  *  8 ); i++) {
 if (pos[i]>0 && pos[i]<( 8  - 1)) {
 pos[i] += 1;
 }
 if (pos[i]==( 8  - 1))
 done++;
 }
 if (done==( 8  *  8 ))
 notdone=0;

 for (i=0; i<( 8  *  8 ); i++) {
 if (origin==0) {
 cubepos[i]=pos[i];
 } else {
 cubepos[i]=(( 8  - 1) - pos[i]);
 }
 }
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 draw_positions_axis(axis, cubepos, 0);

 }
 cube_draw(1, 80);
}



void effect_loadbar(int delay) {
 int z, y, x;
 fill_cube(0x00);

 for (z=( 8  - 1); z>=0; z--) {
 for (y=( 8  - 1); y>=0; y--) {
 for (x=( 8  - 1); x>=0; x--) {
 setvoxel(z, y, x);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }

 }
 }

 cube_draw(1, 100);

 for (z=0; z< 8 ; z++) {
 for (y=0; y< 8 ; y++) {
 for (x=0; x< 8 ; x++) {
 clrvoxel(z, y, x);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }

 }
 }
}



void effect_random_sparkle_flash(int iterations, int voxels, int delay) {
 int i, v;
 fill_cube(0x00);
 for (i=0; i<iterations; i++) {
 for (v=0; v<=voxels; v++)
 setvoxel(rand() %  8 , rand() %  8 , rand() %  8 );

 cube_draw(1, delay);
 fill_cube(0x00);
 }
}




void effect_random_sparkle(void) {
 int i;

 if (fl_button) { fl_button=0; return; }

 for (i=0; i<( 8  * 3); i++)
 {
 effect_random_sparkle_flash(3, i, 3);
 }
 delay_ms(100);
 for (i=( 8  * 3); i>=0; i--)
 {
 effect_random_sparkle_flash(3, i, 3);
 }
}


void effect_wormsqueeze(int axis, int direction, int iterations, int delay) {
 int x, y, i, j, k, dx, dy, size, cube_size;
 int origin=0;

 if ( 8 ==4)
 size=1;
 if ( 8 ==8)
 size=2;

 if (direction==-1)
 origin=( 8  - 1);

 cube_size= 8  - (size - 1);

 x=rand() %  8 ;
 y=rand() %  8 ;

 for (i=0; i<iterations; i++) {
 dx=((rand() % ( 8  / 2 - 1)) - 1);
 dy=((rand() % ( 8  / 2 - 1)) - 1);

 if ((x + dx)>0 && (x + dx)< 8 )
 x += dx;

 if ((y + dy)>0 && (y + dy)< 8 )
 y += dy;

 shift(axis, direction);

 for (j=0; j<size; j++) {
 for (k=0; k<size; k++) {
 if (axis==1)
 {
 setvoxel(origin, y + k, x + j);
 origin--;
 }

 if (axis==2)
 setvoxel(y + k, origin, x + j);

 if (axis==3)
 setvoxel(x + k, y + j, origin);
 }
 }
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }
}


const unsigned char paths_8[44]={
 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x10, 0x20,
 0x30, 0x40, 0x50, 0x60, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75,
 0x76, 0x77, 0x67, 0x57, 0x47, 0x37, 0x27, 0x17, 0x04, 0x03,
 0x12, 0x21, 0x30, 0x40, 0x51, 0x62, 0x73, 0x74, 0x65, 0x56,
 0x47, 0x37, 0x26, 0x15
};

void font_getpath(unsigned char path, unsigned char *destination, int length) {
 int i;
 int offset=0;

 if (path==1)
 offset=28;

 for (i=0; i<length; i++)
 destination[i]=paths_8[i + offset];
}


void effect_pathmove(unsigned char *path, int length) {
 int i, z;
 unsigned char state;


 for (i=(length - 1); i>=1; i--) {
 for (z=0; z< 8 ; z++) {
 state=getvoxel(z,  8  - 1 - (path[(i - 1)] >> 4 & 0x0f),  8  - 1 - ((path[(i - 1)])&0x0f));
 altervoxel(z,  8  - 1 - (path[i] >> 4 & 0x0f),  8  - 1 - ((path[i])&0x0f), state);
 }
 }
 for (z=0; z< 8 ; z++)
 clrvoxel(z,  8  - 1 - (path[0] >> 4 & 0x0f),  8  - 1 - ((path[0])&0x0f));
#line 1846 "E:/mvi/Programming/GitHub/LED-Cube-8x8x8/software_code/cube8_core.c"
}


void effect_pathspiral(int iterations, int delay) {
 int i;
 unsigned char path[28];
 fill_cube(0x00);
 font_getpath(1, path, 28);

 for (i=0; i<iterations; i++) {
 setvoxel(i % 9, ( 8  - 1),  8  / 2);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 effect_pathmove(path, 28);
 }
}


void effect_rand_patharound(int iterations, int delay) {
 int z=4, dz, i;
 unsigned char path[28];
 fill_cube(0x00);
 font_getpath(0, path, 28);

 for (i=0; i<iterations; i++) {
 dz=((rand() % ( 8  / 2 - 1)) - 1);
 z += dz;

 if (z>( 8  - 1))
 z=( 8  - 1);
 if (z<0)
 z=0;

 effect_pathmove(path, 28);
 setvoxel(z, ( 8  - 1), 0);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }
}


void effect_path_text(int delay, char *str) {
 int z, i, ii;
 unsigned char path[28];
 unsigned char chr[ 8 ];
 unsigned char stripe;
 fill_cube(0x00);
 font_getpath(0, path, 28);

 while (*str) {
 font_getchar(*str++, chr);
 for (ii=0; ii< 8 ; ii++)
 {
 stripe=chr[ii];

 for (z=0; z< 8 ; z++) {
 if ((stripe >> z) & 0x01) {
 setvoxel(z, ( 8  - 1), 0);
 } else {
 clrvoxel(z, ( 8  - 1), 0);
 }
 }
 effect_pathmove(path, 28);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }
 effect_pathmove(path, 28);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }
 for (i=0; i<28; i++) {
 effect_pathmove(path, 28);
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 }
}


volatile const unsigned char bitmaps[13][8]={
 {0xc3, 0xc3, 0x00, 0x18, 0x18, 0x81, 0xff, 0x7e},
 {0x3c, 0x42, 0x81, 0x81, 0xc3, 0x24, 0xa5, 0xe7},
 {0x00, 0x04, 0x06, 0xff, 0xff, 0x06, 0x04, 0x00},
 {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81},
 {0xBD, 0xA1, 0xA1, 0xB9, 0xA1, 0xA1, 0xA1, 0x00},
 {0xEF, 0x48, 0x4B, 0x49, 0x4F, 0x00, 0x00, 0x00},
 {0x38, 0x7f, 0xE6, 0xC0, 0xE6, 0x7f, 0x38, 0x00},
 {0x00, 0x22, 0x77, 0x7f, 0x3e, 0x3e, 0x1c, 0x08},
 {0x1C, 0x22, 0x55, 0x49, 0x5d, 0x22, 0x1c, 0x00},
 {0x37, 0x42, 0x22, 0x12, 0x62, 0x00, 0x7f, 0x00},
 {0x89, 0x4A, 0x2c, 0xF8, 0x1F, 0x34, 0x52, 0x91},
 {0x18, 0x3c, 0x7e, 0xdb, 0xff, 0x24, 0x5a, 0xa5},
 {0x00, 0x9c, 0xa2, 0xc5, 0xc1, 0xa2, 0x9c, 0x00}};


unsigned char font_getbitmappixel(char bitmap, char x, char y) {
 char tmp=bitmaps[bitmap][x];
 return (tmp >> y) & 0x01;
}


void effect_path_bitmap(int delay, char bitmap, int iterations) {
 int z, i, ii;
 unsigned char path[28];
 fill_cube(0x00);
 font_getpath(0, path, 28);

 for (i=0; i<iterations; i++) {
 for (ii=0; ii< 8 ; ii++) {
 for (z=0; z< 8 ; z++) {
 if (font_getbitmappixel(bitmap, z, ii))
 {
 setvoxel(z, ( 8  - 1), 0);
 } else {
 clrvoxel(z, ( 8  - 1), 0);
 }
 }
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 effect_pathmove(path, 28);
 }
 for (ii=0; ii<20; ii++) {
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 effect_pathmove(path, 28);
 }
 }
 for (ii=0; ii<10; ii++) {
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 effect_pathmove(path, 28);
 }
}


void effect_smileyspin(char count, int delay, char bitmap) {
 unsigned char dybde[]={0, 1, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 6, 2, 2, 3, 3, 4, 4, 5, 5, 3, 3, 3, 3, 4, 4, 4, 4};
 char x, y;
 char i, s, d;
 char flip, off;
 fill_cube(0x00);

 for (i=0; i<count; i++) {
 flip=0;
 d=0;
 off=0;

 for (s=0; s<( 8  - 1); s++) {
 if (!flip) {
 off++;
 if (off==( 8  / 2))
 {
 flip=1;
 off=0;
 }
 } else
 off++;
 for (x=0; x< 8 ; x++) {
 d=0;
 for (y=0; y< 8 ; y++) {
 if (font_getbitmappixel(bitmap, ( 8  - 1) - x, y)) {
 if (!flip)
 setvoxel(( 8  - 1) - x, dybde[ 8  * off + d++], y);
 else
 setvoxel(( 8  - 1) - x, dybde[31 -  8  * off - d++], y);
 } else
 d++;
 }
 }
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 fill_cube(0x00);
 }


 off=0;
 flip=0;
 d=0;
 for (s=0; s<( 8  - 1); s++) {
 if (!flip) {
 off++;
 if (off==( 8  / 2))
 {
 flip=1;
 off=0;
 }
 } else
 off++;
 for (x=0; x< 8 ; x++) {
 d=0;
 for (y=0; y< 8 ; y++) {
 if (font_getbitmappixel(bitmap, ( 8  - 1) - x, y))
 {
 if (!flip)
 setvoxel(( 8  - 1) - x, y, ( 8  - 1) - dybde[ 8  * off + d++]);
 else
 setvoxel(( 8  - 1) - x, y, ( 8  - 1) - dybde[31 -  8  * off - d++]);
 } else
 d++;
 }
 }
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 fill_cube(0x00);
 }

 flip=0;
 d=0;
 off=0;

 for (s=0; s<( 8  - 1); s++) {
 if (!flip) {
 off++;
 if (off==( 8  / 2))
 {
 flip=1;
 off=0;
 }
 } else
 off++;
 for (x=0; x< 8 ; x++) {
 d=0;
 for (y=0; y< 8 ; y++) {
 if (font_getbitmappixel(bitmap, ( 8  - 1) - x, ( 8  - 1) - y)) {
 if (!flip)
 setvoxel(( 8  - 1) - x, dybde[ 8  * off + d++], y);
 else
 setvoxel(( 8  - 1) - x, dybde[31 -  8  * off - d++], y);
 } else
 d++;
 }
 }
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 fill_cube(0x00);
 }


 off=0;
 flip=0;
 d=0;
 for (s=0; s<( 8  - 1); s++) {
 if (!flip) {
 off++;
 if (off==( 8  / 2))
 {
 flip=1;
 off=0;
 }
 } else
 off++;
 for (x=0; x< 8 ; x++) {
 d=0;
 for (y=0; y< 8 ; y++) {
 if (font_getbitmappixel(bitmap, ( 8  - 1) - x, ( 8  - 1) - y)) {
 if (!flip)
 setvoxel(( 8  - 1) - x, y, ( 8  - 1) - dybde[ 8  * off + d++]);
 else
 setvoxel(( 8  - 1) - x, y, ( 8  - 1) - dybde[31 -  8  * off - d++]);
 } else
 d++;
 }
 }
 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 fill_cube(0x00);
 }

 }
}


void effect_squarespiral1(int iterations, int delay) {
 int loc=0;
 int iter=0;
 while (iter<=iterations) {
 for (loc=0; loc< 8 ; loc++)
 {
 shift(1, 0);
 setvoxel(( 8  - 1), loc, 0);
 setvoxel(( 8  - 1), ( 8  - 1), loc);
 setvoxel(( 8  - 1), ( 8  - 1) - loc, ( 8  - 1));
 setvoxel(( 8  - 1), 0, ( 8  - 1) - loc);

 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 iter++;
 }
 loc=0;
 }
}


void effect_squarespiral2(int iterations, int delay) {
 int loc=0;
 int iter=0;
 while (iter<=iterations) {
 for (loc=0; loc< 8 ; loc++)
 {
 shift(1, 0);
 setvoxel(( 8  - 1), loc, 0);
 setvoxel(( 8  - 1), ( 8  - 1), loc);
 setvoxel(( 8  - 1), ( 8  - 1) - loc, ( 8  - 1));
 setvoxel(( 8  - 1), 0, ( 8  - 1) - loc);
 setvoxel(( 8  - 1), ( 8  - 1) - loc, 0);
 setvoxel(( 8  - 1), ( 8  - 1), ( 8  - 1) - loc);
 setvoxel(( 8  - 1), loc, ( 8  - 1));
 setvoxel(( 8  - 1), 0, loc);

 cube_draw(1, delay);
 if (fl_button) { fl_button=0; return; }
 iter++;
 }
 loc=0;
 }
}


void effect_fireworks(char iterations) {
 char i, f, z;
 char n=80;
 char origin_x, origin_y, origin_z;



 char particles[80][6];

 fill_cube(0x00);

 for (i=0; i<iterations; i++) {
 if (fl_button) { fl_button=0; return; }

 origin_x=rand() % 4 + 2;
 origin_y=rand() % 4 + 2;

 origin_z=1;


 for (z=( 8  - 1); z>=origin_z; z--) {
 setvoxel(z, origin_y, origin_x);
 cube_draw(1, (5 - TAN((z + 0, 1) / 20)*10));
 fill_cube(0x00);
 }


 for (f=0; f<n; f++) {

 particles[f][0]=origin_x;
 particles[f][1]=origin_y;
 particles[f][2]=origin_z;


 particles[f][3]=1 - rand() % 3;
 particles[f][4]=1 - rand() % 3;
 particles[f][5]=1 + rand() % 2;
 }


 for (z=0; z<5; z++)
 {


 for (f=0; f<n; f++)
 {
 particles[f][0] += particles[f][3];
 particles[f][1] += particles[f][4];
 particles[f][2] += particles[f][5] - rand() % 2;
 if (particles[f][2]==0) {
 particles[f][2]++;
 }
 setvoxel(particles[f][2], particles[f][1], particles[f][0]);
 }
 cube_draw(1, 4);
 fill_cube(0x00);
 }
 delay_ms(600);
 }
}
#line 2235 "E:/mvi/Programming/GitHub/LED-Cube-8x8x8/software_code/cube8_core.c"
void launch_effect(char effect) {
 char i, bitmap;


 switch (effect) {
 case 0:

 break;
 case 1:
 for (i=1; i<4; i++)
 effect_planboing(i,3);
 delay_msec(100);
 break;
 case 2:

 effect_rain(500);
 break;
 case 3:

 effect_box_shrink_grow();
 effect_box_woopwoop(4, 0);
 effect_box_woopwoop(4, 1);
 effect_box_woopwoop(4, 0);
 effect_box_woopwoop(4, 1);
 effect_box_woopwoop(4, 0);
 effect_box_woopwoop(4, 1);
 break;
 case 4:

 effect_sendvoxels_rand_axis(50, 2, 1, 5);
 effect_sendvoxels_rand_axis(50, 3, 1, 5);
 break;
 case 5:

 effect_stringfly("HELLO WORLD");
 break;
 case 6:

 effect_boingboing(200, 4, 0x01, 0x01);
 effect_boingboing(500, 2, 0x01, 0x02);
 effect_boingboing(300, 3, 0x01, 0x03);
 break;
 case 7:

 effect_boxside_randsend_parallel(2, 0, 1, 2);
 effect_boxside_randsend_parallel(2, 1, 1, 2);
 effect_boxside_randsend_parallel(3, 0, 1, 2);
 effect_boxside_randsend_parallel(3, 1, 1, 2);
 break;
 case 8:

 effect_random_sparkle();
 break;
 case 9:

 effect_wormsqueeze(2, 0, 200, 4);
 effect_wormsqueeze(3, 1, 200, 4);
 break;
 case 10:
 effect_rand_patharound(1000, 3);
 break;
 case 11:

 effect_smileyspin(2, 3, 0);
 for (i=7; i<9; i++) {
 effect_smileyspin(5, 3, i);
 }
 break;
 case 12:

 effect_path_text(6, "HELLO WORLD");
 break;
 case 13:

 effect_axis_updown_randsuspend(3, 2, 50, 0);
 effect_axis_updown_randsuspend(3, 2, 50, 1);
 effect_axis_updown_randsuspend(2, 2, 50, 0);
 effect_axis_updown_randsuspend(2, 2, 50, 1);
 break;
 case 14:

 effect_z_updown(10);
 break;
 case 15:

 effect_path_bitmap(5, 12, 2);
 effect_path_bitmap(5, 12, 2);
 effect_path_bitmap(5, 12, 2);
 effect_path_bitmap(5, 12, 2);
 effect_path_bitmap(5, 12, 2);
 effect_path_bitmap(5, 12, 2);
 effect_squarespiral1(200, 4);
 effect_squarespiral2(200, 4);
 break;
 case 16:
 break;
 case 17:

 effect_fireworks(3);
 break;
 case 18:

 effect_pathspiral(64, 4);
 break;
 case 19:

 for (bitmap=0; bitmap<14; bitmap++) {
 effect_path_bitmap(5, bitmap, 1);
 }
 break;
 case 20:
 effect_squarespiral1(200, 4);
 effect_squarespiral2(200, 4);
 break;
 case 21:
 effect_planboing(1, 3);
 break;
 default:
 effect_stringfly("ERROR");
 break;
 }
}
#line 2373 "E:/mvi/Programming/GitHub/LED-Cube-8x8x8/software_code/cube8_core.c"
void main(void) {
 INTCON = 0b11110000;
 INTCON2 = 0b10000101;
 RCON = 0b00000000;
 T0CON = 0b11000101;
 CMCON = 0b00000111;
 PR2 = 0b00000000;
 PIE1 = 0b00000000;
 ADCON0 = 0b00000000;
 ADCON1 = 0b00001110;
 LVDCON = 0b00000000;
 PORTA = 0b00000000;
 TRISA = 0b01000000;
 PORTB = 0b00000000;
 TRISB = 0b00000001;
 PORTC = 0b00000000;
 TRISC = 0b00000000;


 while (1) {

 if (fl_start==0) {
 fl_start=1;
 effect=rand()% 22 ;
 }

 launch_effect(effect);
 }
}

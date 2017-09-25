/*
 * Project: LED Cube | cube8_core
 * Version: v3.0 | 2013.12.11-2017.09.24
 * https://github.com/murach-vi/LED-Cube-8x8x8
 * CC BY-NC-SA 4.0 2017 Murach Vitali (murach.vi@gmail.com)
 * License: https://creativecommons.org/licenses/by-nc-sa/4.0

 * Configuration:
    MCU: PI�18F2550
    Oscillator: 20MHz
    Shift registers: 74HC595 x8
*/

#define SH_CP         PORTA.F0
#define DS            PORTA.F1
#define ST_CP         PORTA.F2
#define CUBE_SIZE     8
#define CUBE_BYTES    512
#define EFFECTS_TOTAL 22


volatile char cube[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE];
volatile char buffer[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE];
char effect=1, fl_button=0, fl_start=0, fl_reset=0;
const double PI=3.14159;

// ��������� ����������

void interrupt(void) {
    INTCON.GIE=0x00;
    // ��������� ���������� �� ������� ������
    if (INTCON.INT0IF) {
        if (effect<EFFECTS_TOTAL) {
            effect++;
        } else {
            effect=1;
        }
        delay_ms(1500); // �������� ��� ���������� ��������
        fl_button=1;
        fl_reset=1;
        INTCON.INT0IF=0x00;
        // asm reset
    }
    // ��������� ���������� �� �������
    //unsigned int schet=0;
    if (INTCON.T0IF) {
        //schet++;
        TMR0L=0x00;
        TMR0H=0x00;
        INTCON.T0IF=0x00;
    }
    INTCON.GIE=0xff;
}

/* ------------------------------------------------------------------------ */
/*                              ������� ���������                           */
/* ------------------------------------------------------------------------ */

// �������� ��������� xyz �� ������������ �������� ����
char inrange(char z, char y, char x) {
    if (x>=0 && x<CUBE_SIZE && y>=0 && y<CUBE_SIZE && z>=0 && z<CUBE_SIZE) {
        return 1;
    } else {
        // ���� �� ��������� �� ��������� ����
        return 0;
    }
}

// ����� �������� ����� ������������� ������: speed=1ms * delay
void delay_msec(int speed) {
    int delay;
    for (delay=1; delay<=speed; delay++) {
        delay_ms(1);
    }
}

// ��������� ������ ����
void cube_draw(int speed, int iterations) {
    int x, y, z, i;

    for (i=0; i<iterations; i++) {
        // ����������� 64 ���� � �������� ��������
        for (z=0; z<CUBE_SIZE; z++) {
            for (y=(CUBE_SIZE - 1); y>=0; y--) {
                for (x=(CUBE_SIZE - 1); x>=0; x--) {
                    DS=cube[z][y][x];
                    // ���������� ��� � �������
                    SH_CP=1;
                    SH_CP=0;
                }
            }
            // ���������� ��������� ������ ���� � ����������� ��������� speed
            // ������� ����� � ����������� ������
            ST_CP=1;
            switch (z) {
                case 0:
                    PORTB=0b10000000; // layer0
                    PORTC=0b00000000;
                    break;
                case 1:
                    PORTB=0b01000000; // layer1
                    PORTC=0b00000000;
                    break;
                case 2:
                    PORTB=0b00100000; // layer2
                    PORTC=0b00000000;
                    break;
                case 3:
                    PORTB=0b00010000; // layer3
                    PORTC=0b00000000;
                    break;
                case 4:
                    PORTB=0b00001000; // layer4
                    PORTC=0b00000000;
                    break;
                case 5:
                    PORTB=0b00000100; // layer5
                    PORTC=0b00000000;
                    break;
                case 6:
                    PORTB=0b00000010; // layer6
                    PORTC=0b00000000;
                    break;
                case 7:
                    PORTB=0b00000000;
                    PORTC=0b00000100; // layer7
                    break;
                default:
                    break;
            }
            // �������� ����� ���������� ������
            delay_msec(speed);
            // ������������ ������
            ST_CP=0;
            PORTB=0b00000000; // layer0-layer6
            PORTC=0b00000000; // layer7
            // ��������� ����������� ������ � ��������� ��������

        }
    }
}

// ��������� ����������
void setvoxel(char z, char y, char x) {
    if (inrange(z, y, x)) {
        cube[z][y][x]=0xff;
    }
}

// ���������� ����������
void clrvoxel(char z, char y, char x) {
    if (inrange(z, y, x))
        cube[z][y][x]=0x00;
}

// ����������� ��������� (�������/��������) ����������
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

// ��������� ��������� ���������� (���������/���������� - state)
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

// �������������� ��������� ����������.
// ���� ��� ������� (1), �� ���������� (0) � ��������.
void flpvoxel(char z, char y, char x) {
    if (inrange(z, y, x))
        cube[z][y][x] ^= cube[z][y][x];
}

// ��������� ������ ���������� �� ��������� ������
void tmpsetvoxel(char z, char y, char x) {
    if (inrange(z, y, x))
        buffer[z][y][x]=0xff;
}

// ���������� ������ ���������� �� ��������� ������
void tmpclrvoxel(int z, int y, int x) {
    if (inrange(z, y, x))
        buffer[z][y][x]=0x00;
}

// ��������� ���������� ��������� �� ��� X
void setplane_x(char x) {
    char z, y;

    if (x>=0 && x<CUBE_SIZE) {
        for (z=0; z<CUBE_SIZE; z++) {
            for (y=0; y<CUBE_SIZE; y++) {
                cube[z][y][x]=0xff;
            }
        }
    }
}

// ��������� ���������� ��������� �� ��� X
void clrplane_x(char x) {
    char z, y;

    if (x>=0 && x<CUBE_SIZE) {
        for (z=0; z<CUBE_SIZE; z++) {
            for (y=0; y<CUBE_SIZE; y++) {
                cube[z][y][x]=0x00;
            }
        }
    }
}

// ��������� ���������� ��������� �� ��� Y
void setplane_y(char y) {
    char z, x;

    if (y>=0 && y<CUBE_SIZE) {
        for (z=0; z<CUBE_SIZE; z++) {
            for (x=0; x<CUBE_SIZE; x++) {
                cube[z][y][x]=0xff;
            }
        }
    }
}

// ��������� ���������� ��������� �� ��� Y
void clrplane_y(char y) {
    char z, x;

    if (y>=0 && y<CUBE_SIZE) {
        for (z=0; z<CUBE_SIZE; z++) {
            for (x=0; x<CUBE_SIZE; x++) {
                cube[z][y][x]=0x00;
            }
        }
    }
}

// ��������� ���� 64 ��� �����
// ������������ ��� ��������� ���� fill(layer, 0x00)
// ��� ��� ��������� ���� ����������� fill(layer, 0xff)
void fill_layer(char layer, char colour) {
    char y, x;

    for (y=0; y<CUBE_SIZE; y++) {
        for (x=0; x<CUBE_SIZE; x++) {
            cube[layer][y][x]=colour;
        }
    }
}

void tmpfill_layer(char layer, char colour) {
    char y, x;

    for (y=0; y<CUBE_SIZE; y++) {
        for (x=0; x<CUBE_SIZE; x++) {
            buffer[layer][y][x]=colour;
        }
    }
}

// ��������� ����� ����
// ������������ ��� ��������� ���� fill_cube(0x00)
// ��� ��� ��������� ���� ����������� fill_cube(0xff)
void fill_cube(char colour) {
    char z, y, x;

    for (z=0; z<CUBE_SIZE; z++) {
        for (y=0; y<CUBE_SIZE; y++) {
            for (x=0; x<CUBE_SIZE; x++) {
                cube[z][y][x]=colour;
            }
        }
    }
}

// ��������� ���������� ��������� �� �������� ���
void setplane(char axis, char i) {
    switch (axis) {
        case 1: //AXIS_Z:
            fill_layer(i, 0xff);
            break;
        case 2: //AXIS_Y:
            setplane_y(i);
            break;
        case 3: //AXIS_X:
            setplane_x(i);
            break;
    }
}

// ��������� ���������� ��������� �� �������� ���
void clrplane(char axis, char i) {
    switch (axis) {
        case 1: //AXIS_Z:
            fill_layer(i, 0x00);
            break;
        case 2: //AXIS_Y:
            clrplane_y(i);
            break;
        case 3: //AXIS_X:
            clrplane_x(i);
            break;
    }
}

// ����������, ��� x1 ������ ������ x2, � ���� ��������, �� ���������.
// ������������ ��� �������������� ������������ ��������� �������
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

// ������ ��������� ���������� ��������������
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

// ������ ����� �������������� �� ����������� �������
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

// ������ ����� �������������� �� ����������� �������
void box_wireframe(char z1, char y1, char x1, char z2, char y2, char x2) {
    char iz, iy, ix;

    argorder(x1, x2, &x1, &x2);
    argorder(y1, y2, &y1, &y2);
    argorder(z1, z2, &z1, &z2);

    for (iz=z1; iz<=z2; iz++) {
        for (iy=y1; iy<=y2; iy++) {
            for (ix=x1; ix<=x2; ix++) {
                if ((iz>=z1 && iz<=z2 && iy==y1 && ix==x1) || // ������ (z1,y1,x1)-(z2,y1,x1)
                        (iz>=z1 && iz<=z2 && iy==y2 && ix==x1) || // ������ (z1,y2,x1)-(z2,y2,x1)
                        (iz>=z1 && iz<=z2 && iy==y1 && ix==x2) || // ������ (z1,y1,x2)-(z2,y1,x2)
                        (iz>=z1 && iz<=z2 && iy==y2 && ix==x2) || // ������ (z1,y2,x2)-(z2,y2,x2)

                        (iz==z1 && iy==y1 && ix>=x1 && ix<=x2) || // ������ (z1,y1,x1)-(z1,y1,x2)
                        (iz==z1 && iy==y2 && ix>=x1 && ix<=x2) || // ������ (z1,y2,x1)-(z1,y2,x2)
                        (iz==z2 && iy==y1 && ix>=x1 && ix<=x2) || // ������ (z2,y1,x1)-(z1,y1,x2)
                        (iz==z2 && iy==y2 && ix>=x1 && ix<=x2) || // ������ (z2,y2,x1)-(z1,y2,x2)

                        (iz==z1 && iy>=y1 && iy<=y2 && ix==x1) || // ������ (z1,y1,x1)-(z1,y2,x1)
                        (iz==z1 && iy>=y1 && iy<=y2 && ix==x2) || // ������ (z1,y1,x2)-(z1,y2,x2)
                        (iz==z2 && iy>=y1 && iy<=y2 && ix==x1) || // ������ (z2,y1,x1)-(z2,y2,x1)
                        (iz==z2 && iy>=y1 && iy<=y2 && ix==x2)) // ������ (z2,y1,x2)-(z2,y2,x2)
                {
                    cube[iz][iy][ix]=0xff;
                } else {
                    cube[iz][iy][ix]=0x00;
                }
            }
        }
    }
}

// ������ ����� � �� ��� ����������� � 3d ������������
// ������������ ����� ��������, ��-�� ���� ����� �� ������ �������
void line(char z1, char y1, char x1, char z2, char y2, char x2) {
    char x, y, z, last_y, last_z;
    float xy, xz; // ���������� ������� �����, ��� ����������� �� "y" � "x"
    // ��� float �������� �������� ��������� (�� ������, ����������
    // �� ����� ����), �� ���������� ROM �� 7% �������, ��� ��� char

    if (inrange(z1, y1, x1) & inrange(z2, y2, x2)) {
        // �� ������ ������ ����� �� x=0 � x=7
        // ���� x1 ������ x2, �� ���������� �������� ���������� �������
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

        // ���������� "y" ��� ������� ���� "x"
        for (x=x1; x<=x2; x++) {
            y=(xy * (x - x1)) + y1;
            z=(xz * (x - x1)) + z1;
            setvoxel(z, y, x);
        }
    }
}

// �������� ���������� ���� � �����
void cube2buffer(void) {
    unsigned char z, y, x;

    for (z=0; z<CUBE_SIZE; z++) {
        for (y=0; y<CUBE_SIZE; y++) {
            for (x=0; x<CUBE_SIZE; x++) {
                buffer[z][y][x]=cube[z][y][x];
            }
        }
    }
}

// �������� ���������� ������ � ���
void buffer2cube(void) {
    unsigned char z, y, x;

    for (z=0; z<CUBE_SIZE; z++) {
        for (y=0; y<CUBE_SIZE; y++) {
            for (x=0; x<CUBE_SIZE; x++) {
                cube[z][y][x]=buffer[z][y][x];
            }
        }
    }
}

// ��������� ���� �� 180 �������� ����� ��� "z"
void mirror_z(void) {
    char z, y, x;
    cube2buffer(); // �������� ���������� ���� � �����
    fill_cube(0x00);

    for (z=0; z<CUBE_SIZE; z++) {
        for (y=0; y<CUBE_SIZE; y++) {
            for (x=0; x<CUBE_SIZE; x++) {
                if (buffer[z][y][x]==0xff)
                    setvoxel(CUBE_SIZE - 1 - z, y, x);
            }
        }
    }
}

// ��������� ���� �� 180 �������� ����� ��� "y"
void mirror_y(void) {
    char z, y, x;
    cube2buffer(); // �������� ���������� ���� � �����
    fill_cube(0x00);

    for (z=0; z<CUBE_SIZE; z++) {
        for (y=0; y<CUBE_SIZE; y++) {
            for (x=0; x<CUBE_SIZE; x++) {
                if (buffer[z][y][x]==0xff)
                    setvoxel(z, CUBE_SIZE - 1 - y, x);
            }
        }
    }
}

// ��������� ���� �� 180 �������� ����� ��� "x"
void mirror_x(void) {
    char z, y, x;
    cube2buffer(); // �������� ���������� ���� � �����
    fill_cube(0x00);

    for (z=0; z<CUBE_SIZE; z++) {
        for (y=0; y<CUBE_SIZE; y++) {
            for (x=0; x<CUBE_SIZE; x++) {
                if (buffer[z][y][x]==0xff)
                    setvoxel(z, y, CUBE_SIZE - 1 - x);
            }
        }
    }
}

// ����������� ����� ����������� ���� ����� ����� ���.
// ������������ ��� �������� � ������������� �� ����� ������� ���� � ������.
// ��������, ��� ������� ����� ������������ shift(1,0).
void shift(char axis, char direction) {
    char z, y, x, ii, iii, state;

    if (axis==1) // AXIS_Z - 1
    {
        for (z=0; z<CUBE_SIZE; z++) {
            for (y=0; y<CUBE_SIZE; y++) {
                for (x=0; x<CUBE_SIZE; x++) {
                    if (direction==0) // direction=0 (-1) - �� AXIS_Z ���������� ����
                    { // direction=1 - �� AXIS_Z ���������� �����
                        ii=z;
                        iii=ii + 1;
                    } else {
                        ii=(CUBE_SIZE - 1) - z;
                        iii=ii - 1;
                    }
                    state=getvoxel(iii, y, x);
                    altervoxel(ii, y, x, state);
                }
            }
        }
    }
    if (axis==2) // AXIS_Y - 2
    {
        for (z=0; z<CUBE_SIZE; z++) {
            for (y=0; y<CUBE_SIZE; y++) {
                for (x=0; x<CUBE_SIZE; x++) {
                    if (direction==0) // direction=0 (-1) - �� AXIS_Y �������� �����
                    { // direction=1 - �� AXIS_Y �������� ������
                        ii=(CUBE_SIZE - 1) - y;
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
    if (axis==3) // AXIS_X - 3
    {
        for (z=0; z<CUBE_SIZE; z++) {
            for (y=0; y<CUBE_SIZE; y++) {
                for (x=0; x<CUBE_SIZE; x++) {
                    if (direction==0) // direction=0 (-1) - �� AXIS_X �������� �����
                    { // direction=1 - �� AXIS_X �������� ������
                        ii=x;
                        iii=ii + 1;
                    } else {
                        ii=(CUBE_SIZE - 1) - x;
                        iii=ii - 1;
                    }
                    state=getvoxel(z, y, iii);
                    altervoxel(z, y, ii, state);
                }
            }
        }
    }
}

// ���������� �������, ����� � ����� ��� ���� 8�8�8
volatile const unsigned char font8eng[728][8]={
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //    0
    {0x00, 0x00, 0x5f, 0x5f, 0x00, 0x00, 0x00, 0x00}, // !  1
    {0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00}, // "
    {0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14, 0x00, 0x00}, // #
    {0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12, 0x00, 0x00}, // $
    {0x00, 0x23, 0x13, 0x08, 0x64, 0x62, 0x00, 0x00}, // %
    {0x00, 0x36, 0x49, 0x55, 0x22, 0x50, 0x00, 0x00}, // &
    {0x00, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00}, // '
    {0x00, 0x00, 0x1c, 0x22, 0x41, 0x00, 0x00, 0x00}, // (  8
    {0x00, 0x00, 0x41, 0x22, 0x1c, 0x00, 0x00, 0x00}, // )  9
    {0x00, 0x14, 0x08, 0x3e, 0x08, 0x14, 0x00, 0x00}, // *
    {0x00, 0x08, 0x08, 0x3e, 0x08, 0x08, 0x00, 0x00}, // +
    {0x00, 0x00, 0x50, 0x30, 0x00, 0x00, 0x00, 0x00}, // ,
    {0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00}, // -
    {0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00}, // .
    {0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00}, // /
    {0x00, 0x3e, 0x51, 0x49, 0x45, 0x3e, 0x00, 0x00}, // 0
    {0x00, 0x00, 0x42, 0x7f, 0x40, 0x00, 0x00, 0x00}, // 1
    {0x00, 0x42, 0x61, 0x51, 0x49, 0x46, 0x00, 0x00}, // 2  18
    {0x00, 0x21, 0x41, 0x45, 0x4b, 0x31, 0x00, 0x00}, // 3  19
    {0x00, 0x18, 0x14, 0x12, 0x7f, 0x10, 0x00, 0x00}, // 4
    {0x00, 0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00}, // 5
    {0x00, 0x3c, 0x4a, 0x49, 0x49, 0x30, 0x00, 0x00}, // 6
    {0x00, 0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00}, // 7
    {0x00, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00}, // 8
    {0x00, 0x06, 0x49, 0x49, 0x29, 0x1e, 0x00, 0x00}, // 9
    {0x00, 0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00}, // :
    {0x00, 0x00, 0x56, 0x36, 0x00, 0x00, 0x00, 0x00}, // ;
    {0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00, 0x00}, // <
    {0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 0x00}, // =
    {0x00, 0x00, 0x41, 0x22, 0x14, 0x08, 0x00, 0x00}, // >
    {0x00, 0x02, 0x01, 0x51, 0x09, 0x06, 0x00, 0x00}, // ?
    {0x00, 0x32, 0x49, 0x79, 0x41, 0x3e, 0x00, 0x00}, // @
    {0x00, 0x7e, 0x11, 0x11, 0x11, 0x7e, 0x00, 0x00}, // A
    {0x00, 0x7f, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00}, // B
    {0x00, 0x3e, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00}, // C
    {0x00, 0x7f, 0x41, 0x41, 0x22, 0x1c, 0x00, 0x00}, // D
    {0x00, 0x7f, 0x49, 0x49, 0x49, 0x41, 0x00, 0x00}, // E
    {0x00, 0x7f, 0x09, 0x09, 0x09, 0x01, 0x00, 0x00}, // F  38
    {0x00, 0x3e, 0x41, 0x49, 0x49, 0x7a, 0x00, 0x00}, // G  39
    {0x00, 0x7f, 0x08, 0x08, 0x08, 0x7f, 0x00, 0x00}, // H
    {0x00, 0x00, 0x41, 0x7f, 0x41, 0x00, 0x00, 0x00}, // I
    {0x00, 0x20, 0x40, 0x41, 0x3f, 0x01, 0x00, 0x00}, // J
    {0x00, 0x7f, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00}, // K
    {0x00, 0x7f, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00}, // L
    {0x00, 0x7f, 0x02, 0x0c, 0x02, 0x7f, 0x00, 0x00}, // M
    {0x00, 0x7f, 0x04, 0x08, 0x10, 0x7f, 0x00, 0x00}, // N
    {0x00, 0x3e, 0x41, 0x41, 0x41, 0x3e, 0x00, 0x00}, // O
    {0x00, 0x7f, 0x09, 0x09, 0x09, 0x06, 0x00, 0x00}, // P
    {0x00, 0x3e, 0x41, 0x51, 0x21, 0x5e, 0x00, 0x00}, // Q
    {0x00, 0x7f, 0x09, 0x19, 0x29, 0x46, 0x00, 0x00}, // R
    {0x00, 0x46, 0x49, 0x49, 0x49, 0x31, 0x00, 0x00}, // S
    {0x00, 0x01, 0x01, 0x7f, 0x01, 0x01, 0x00, 0x00}, // T  52
    {0x00, 0x3f, 0x40, 0x40, 0x40, 0x3f, 0x00, 0x00}, // U  53
    {0x00, 0x1f, 0x20, 0x40, 0x20, 0x1f, 0x00, 0x00}, // V  54
    {0x00, 0x3f, 0x40, 0x38, 0x40, 0x3f, 0x00, 0x00}, // W  55
    {0x00, 0x63, 0x14, 0x08, 0x14, 0x63, 0x00, 0x00}, // X  56
    {0x00, 0x07, 0x08, 0x70, 0x08, 0x07, 0x00, 0x00}, // Y  57
    {0x00, 0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 0x00}, // Z  58
    {0x00, 0x00, 0x7f, 0x41, 0x41, 0x00, 0x00, 0x00}, // [  59
    {0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00}, // \  60
    {0x00, 0x00, 0x41, 0x41, 0x7f, 0x00, 0x00, 0x00}, // ]  61
    {0x00, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00, 0x00}, // ^  62
    {0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00}, // _  63
    {0x00, 0x00, 0x01, 0x02, 0x04, 0x00, 0x00, 0x00}, // `  64
    {0x00, 0x20, 0x54, 0x54, 0x54, 0x78, 0x00, 0x00}, // a  65
    {0x00, 0x7f, 0x48, 0x44, 0x44, 0x38, 0x00, 0x00}, // b  66
    {0x00, 0x38, 0x44, 0x44, 0x44, 0x20, 0x00, 0x00}, // c  67
    {0x00, 0x38, 0x44, 0x44, 0x48, 0x7f, 0x00, 0x00}, // d  68
    {0x00, 0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 0x00}, // e  69
    {0x00, 0x08, 0x7e, 0x09, 0x01, 0x02, 0x00, 0x00}, // f  70
    {0x00, 0x0c, 0x52, 0x52, 0x52, 0x3e, 0x00, 0x00}, // g  71
    {0x00, 0x7f, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00}, // h  72
    {0x00, 0x00, 0x44, 0x7d, 0x40, 0x00, 0x00, 0x00}, // i  73
    {0x00, 0x20, 0x40, 0x44, 0x3d, 0x00, 0x00, 0x00}, // j  74
    {0x00, 0x7f, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00}, // k  75
    {0x00, 0x00, 0x41, 0x7f, 0x40, 0x00, 0x00, 0x00}, // l  76
    {0x00, 0x7c, 0x04, 0x18, 0x04, 0x78, 0x00, 0x00}, // m  77
    {0x00, 0x7c, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00}, // n  78
    {0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00}, // o  79
    {0x00, 0x7c, 0x14, 0x14, 0x14, 0x08, 0x00, 0x00}, // p  80
    {0x00, 0x08, 0x14, 0x14, 0x18, 0x7c, 0x00, 0x00}, // q  81
    {0x00, 0x7c, 0x08, 0x04, 0x04, 0x08, 0x00, 0x00}, // r  82
    {0x00, 0x48, 0x54, 0x54, 0x54, 0x20, 0x00, 0x00}, // s  83
    {0x00, 0x04, 0x3f, 0x44, 0x40, 0x20, 0x00, 0x00}, // t  84
    {0x00, 0x3c, 0x40, 0x40, 0x20, 0x7c, 0x00, 0x00}, // u  85
    {0x00, 0x1c, 0x20, 0x40, 0x20, 0x1c, 0x00, 0x00}, // v  86
    {0x00, 0x3c, 0x40, 0x30, 0x40, 0x3c, 0x00, 0x00}, // w  87
    {0x00, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00, 0x00}, // x  88
    {0x00, 0x0c, 0x50, 0x50, 0x50, 0x3c, 0x00, 0x00}, // y  89
    {0x00, 0x44, 0x64, 0x54, 0x4c, 0x44, 0x00, 0x00}
}; // z  90        {0xff,0xff,0x00,0xff,0xff,0x00,0xff,0xff}

/*
// ���������� �������, ����� � ����� ��� ���� 4�4�4
volatile const unsigned char font4eng[90][4]={
    {0x00,0x00,0x00,0x00},	//    0
    {0x00,0x0b,0x00,0x00},	// !
    {0x00,0x00,0x00,0x00},	// "
    {0x00,0x00,0x00,0x00},	// #
    {0x00,0x00,0x00,0x00},	// $
    {0x00,0x00,0x00,0x00},	// %
    {0x00,0x00,0x00,0x00},	// &
    {0x00,0x00,0x00,0x00},	// '
    {0x00,0x00,0x00,0x00},	// (
    {0x00,0x00,0x00,0x00},	// )  9
    {0x00,0x00,0x00,0x00},	// *
    {0x00,0x00,0x00,0x00},	// +
    {0x00,0x00,0x00,0x00},	// ,
    {0x00,0x00,0x00,0x00},	// -
    {0x00,0x00,0x00,0x00},	// .
    {0x00,0x00,0x00,0x00},	// /
    {0x0f,0x09,0x0f,0x00},	// 0
    {0x02,0x0f,0x00,0x00},	// 1
    {0x09,0x0d,0x0b,0x00},	// 2
    {0x09,0x0d,0x0f,0x00},	// 3  19
    {0x07,0x04,0x0f,0x00},	// 4
    {0x0b,0x0b,0x0d,0x00},	// 5
    {0x0f,0x0d,0x0d,0x00},	// 6
    {0x01,0x01,0x0f,0x00},	// 7
    {0x0f,0x0d,0x0f,0x00},	// 8
    {0x0b,0x0b,0x0f,0x00},	// 9
    {0x00,0x00,0x00,0x00},	// :
    {0x00,0x00,0x00,0x00},	// ;
    {0x00,0x00,0x00,0x00},	// <
    {0x00,0x00,0x00,0x00},	//= 29
    {0x00,0x00,0x00,0x00},	// >
    {0x00,0x00,0x00,0x00},	// ?
    {0x00,0x00,0x00,0x00},	// @
    {0x0e,0x05,0x0e,0x00},	// A
    {0x0f,0x0a,0x0e,0x00},	// B
    {0x0f,0x09,0x09,0x00},	// C
    {0x0f,0x09,0x06,0x00},	// D
    {0x0f,0x0d,0x09,0x00},	// E
    {0x0f,0x05,0x01,0x00},	// F
    {0x0f,0x09,0x0d,0x00},	// G  39
    {0x0f,0x04,0x0f,0x00},	// H
    {0x09,0x0f,0x09,0x00},	// I
    {0x08,0x09,0x07,0x00},	// J
    {0x0f,0x06,0x09,0x00},	// K
    {0x0f,0x08,0x08,0x00},	// L
    {0x0f,0x03,0x0f,0x00},	// M
    {0x0f,0x02,0x0f,0x00},	// N
    {0x0f,0x09,0x0f,0x00},	// O
    {0x0f,0x05,0x07,0x00},	// P
    {0x07,0x05,0x0f,0x00},	// Q  49
    {0x0f,0x05,0x0b,0x00},	// R
    {0x0b,0x08,0x0d,0x00},	// S
    {0x01,0x0f,0x01,0x00},	// T
    {0x0f,0x08,0x0f,0x00},	// U
    {0x07,0x08,0x07,0x00},	// V
    {0x07,0x0e,0x07,0x00},	// W
    {0x09,0x06,0x09,0x00},	// X
    {0x0b,0x0a,0x0f,0x00},	// Y
    {0x0d,0x0b,0x09,0x00},	// Z  58
  {0x00,0x00,0x00,0x00},	// [  59
    {0x00,0x00,0x00,0x00},	// \
    {0x00,0x00,0x00,0x00},	// ]
    {0x00,0x00,0x00,0x00},	// ^
    {0x00,0x00,0x00,0x00},	// _
    {0x00,0x00,0x00,0x00},	// `
    {0x00,0x00,0x00,0x00},	// a
  {0x00,0x00,0x00,0x00},	// b
  {0x00,0x00,0x00,0x00},	// c
    {0x00,0x00,0x00,0x00},	// d
    {0x00,0x00,0x00,0x00},	// e  69
    {0x00,0x00,0x00,0x00},	// f
    {0x00,0x00,0x00,0x00},	// g
    {0x00,0x00,0x00,0x00},	// h
    {0x00,0x00,0x00,0x00},	// i
    {0x00,0x00,0x00,0x00},	// j
    {0x00,0x00,0x00,0x00},	// k
    {0x00,0x00,0x00,0x00},	// l
    {0x00,0x00,0x00,0x00},	// m
    {0x00,0x00,0x00,0x00},	// n
    {0x00,0x00,0x00,0x00},	// o  79
    {0x00,0x00,0x00,0x00},	// p
    {0x00,0x00,0x00,0x00},	// q
    {0x00,0x00,0x00,0x00},	// r
    {0x00,0x00,0x00,0x00},	// s
    {0x00,0x00,0x00,0x00},	// t
    {0x00,0x00,0x00,0x00},	// u
    {0x00,0x00,0x00,0x00},	// v
    {0x00,0x00,0x00,0x00},	// w
  {0x09,0x06,0x09,0x00},  // x  88
  {0x0b,0x0a,0x0f,0x00},  // y  89
  {0x0d,0x0b,0x09,0x00}   // z  90
};

// ������� �������, ����� � ����� ��� ���� 4�4�4
volatile const unsigned char font4rus[90][4]={
    {0x00,0x00,0x00,0x00},	//    0
    {0x00,0x0b,0x00,0x00},	// !
    {0x00,0x00,0x00,0x00},	// "
    {0x00,0x00,0x00,0x00},	// #
    {0x00,0x00,0x00,0x00},	// $
    {0x00,0x00,0x00,0x00},	// %
    {0x00,0x00,0x00,0x00},	// &
    {0x00,0x00,0x00,0x00},	// '
    {0x00,0x00,0x00,0x00},	// (
    {0x00,0x00,0x00,0x00},	// )  9
    {0x00,0x00,0x00,0x00},	// *
    {0x00,0x00,0x00,0x00},	// +
    {0x00,0x00,0x00,0x00},	// ,
    {0x00,0x00,0x00,0x00},	// -
    {0x00,0x00,0x00,0x00},	// .
    {0x00,0x00,0x00,0x00},	// /
    {0x00,0x00,0x00,0x00},	// 0
    {0x00,0x00,0x00,0x00},	// 1
    {0x00,0x00,0x00,0x00},	// 2
    {0x00,0x00,0x00,0x00},	// 3  19
    {0x00,0x00,0x00,0x00},	// 4
    {0x00,0x00,0x00,0x00},	// 5
    {0x00,0x00,0x00,0x00},	// 6
    {0x00,0x00,0x00,0x00},	// 7
    {0x00,0x00,0x00,0x00},	// 8
    {0x00,0x00,0x00,0x00},	// 9
    {0x00,0x00,0x00,0x00},	// :
    {0x00,0x00,0x00,0x00},	// ;
    {0x00,0x00,0x00,0x00},	// <
    {0x00,0x00,0x00,0x00},	//= 29
    {0x00,0x00,0x00,0x00},	// >
    {0x00,0x00,0x00,0x00},	// ?
    {0x00,0x00,0x00,0x00},	// @
    {0x00,0x00,0x00,0x00},	// A
    {0x00,0x00,0x00,0x00},	// B
    {0x00,0x00,0x00,0x00},	// C
    {0x00,0x00,0x00,0x00},	// D
    {0x00,0x00,0x00,0x00},	// E
    {0x00,0x00,0x00,0x00},	// F
    {0x00,0x00,0x00,0x00},	// G  39
    {0x00,0x00,0x00,0x00},	// H
    {0x00,0x00,0x00,0x00},	// I
    {0x00,0x00,0x00,0x00},	// J
    {0x00,0x00,0x00,0x00},	// K
    {0x00,0x00,0x00,0x00},	// L
    {0x00,0x00,0x00,0x00},	// M
    {0x00,0x00,0x00,0x00},	// N
    {0x00,0x00,0x00,0x00},	// O
    {0x00,0x00,0x00,0x00},	// P
    {0x00,0x00,0x00,0x00},	// Q  49
    {0x00,0x00,0x00,0x00},	// R
    {0x00,0x00,0x00,0x00},	// S
    {0x00,0x00,0x00,0x00},	// T
    {0x00,0x00,0x00,0x00},	// U
    {0x00,0x00,0x00,0x00},	// V
    {0x00,0x00,0x00,0x00},	// W
    {0x00,0x00,0x00,0x00},	// X
    {0x00,0x00,0x00,0x00},	// Y
    {0x00,0x00,0x00,0x00},	// Z  58
  {0x00,0x00,0x00,0x00},	// [  59
    {0x00,0x00,0x00,0x00},	// \
    {0x00,0x00,0x00,0x00},	// ]
    {0x00,0x00,0x00,0x00},	// ^
    {0x00,0x00,0x00,0x00},	// _
    {0x00,0x00,0x00,0x00},	// `
    {0x00,0x00,0x00,0x00},	// a
  {0x00,0x00,0x00,0x00},	// b
  {0x00,0x00,0x00,0x00},	// c
    {0x00,0x00,0x00,0x00},	// d
    {0x00,0x00,0x00,0x00},	// e  69
    {0x00,0x00,0x00,0x00},	// f
    {0x00,0x00,0x00,0x00},	// g
    {0x00,0x00,0x00,0x00},	// h
    {0x00,0x00,0x00,0x00},	// i
    {0x00,0x00,0x00,0x00},	// j
    {0x00,0x00,0x00,0x00},	// k
    {0x00,0x00,0x00,0x00},	// l
    {0x00,0x00,0x00,0x00},	// m
    {0x00,0x00,0x00,0x00},	// n
    {0x00,0x00,0x00,0x00},	// o  79
    {0x00,0x00,0x00,0x00},	// p
    {0x00,0x00,0x00,0x00},	// q
    {0x00,0x00,0x00,0x00},	// r
    {0x00,0x00,0x00,0x00},	// s
    {0x00,0x00,0x00,0x00},	// t
    {0x00,0x00,0x00,0x00},	// u
    {0x00,0x00,0x00,0x00},	// v
    {0x00,0x00,0x00,0x00},	// w
  {0x00,0x00,0x00,0x00},  // x  88
  {0x00,0x00,0x00,0x00},  // y  89
  {0x00,0x00,0x00,0x00}   // z  90
};*/

// ���������� ���� �� ������� font4(8) � ������������� �� ������������������ ����
void font_getchar(char chr, unsigned char dst[]) {
    int i;
    chr -= 32; // our bitmap font starts at ascii char 32                   33
    /* ������ ����� ��� �������� ������
      if (CUBE_SIZE==4)
        {
        for (i=0; i<CUBE_SIZE; i++)
        {
          dst[i]=font4eng[chr][i];
        }
      }
     */
    if (CUBE_SIZE==8) {
        for (i=0; i<CUBE_SIZE; i++) {
            dst[i]=font8eng[chr][i];
        }
    }
}

/* ------------------------------------------------------------------------ */
/*                           ����� ������� ���������                        */
/* ------------------------------------------------------------------------ */





/* ------------------------------------------------------------------------ */
/*                             ������� ��������                             */
/* ------------------------------------------------------------------------ */

// �����
void effect_rain(int iterations) {
    int i, ii;
    int rnd_x;
    int rnd_y;
    int rnd_num;
    fill_cube(0x00);

    for (ii=0; ii<iterations; ii++) {
        rnd_num=rand() % (CUBE_SIZE / 2);
        for (i=0; i<rnd_num; i++) {
            rnd_x=rand() % CUBE_SIZE;
            rnd_y=rand() % CUBE_SIZE;
            setvoxel(0, rnd_y, rnd_x);
        }
        cube_draw(1, 10);
        shift(1, 1);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
    }
}

// �������� ���������� ����� � �����
void effect_planboing(char axis, int speed) {
    int i;

    for (i=0; i<CUBE_SIZE; i++) {
        fill_cube(0x00);
        setplane(axis, i);
        cube_draw(1, speed);
        if (fl_button==1) {
            fl_button=0;
            return;
        }
    }
    for (i=(CUBE_SIZE - 1); i>=0; i--) {
        fill_cube(0x00);
        setplane(axis, i);
        cube_draw(1, speed);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
    }
}

// ���������� � ����������� ������ ���� �� ������ �����
void effect_box_shrink_grow(void) {
    int x, i, ii, xyz;
    int iterations=1;
    int flip=ii & (CUBE_SIZE / 2);
    int delay=2;

    for (ii=0; ii<CUBE_SIZE; ii++) {
        for (x=0; x<iterations; x++) {
            for (i=0; i<(CUBE_SIZE * 2); i++) {
                xyz=(CUBE_SIZE - 1) - i; // ������������� �������� i ����� 0 and (CUBE_SIZE-1)
                if (i>(CUBE_SIZE - 1))
                    xyz=i - CUBE_SIZE; // ���� i>7, i 8-15 ���������� xyz 0-7

                fill_cube(0x00);
                cube_draw(1, 1);
                if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������

                box_wireframe(0, 0, 0, xyz, xyz, xyz);

                if (flip>0) // ����� ����
                    mirror_z();
                if (ii==(CUBE_SIZE - 3) || ii==(CUBE_SIZE - 1))
                    mirror_y();
                if (ii==(CUBE_SIZE - 2) || ii==(CUBE_SIZE - 1))
                    mirror_x();

                cube_draw(1, delay);
                fill_cube(0x00);
                if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            }
        }
    }
}

// ������� ����� ���, ������� ��������� ��� ������ �� ������ ����
void effect_box_woopwoop(int delay, int grow) {
    int i, ii;
    char ci=CUBE_SIZE / 2;
    fill_cube(0x00);

    for (i=0; i<ci; i++) {
        ii=i;
        if (grow>0)
            ii=(ci - 1) - i;

        box_wireframe(ci + ii, ci + ii, ci + ii, (ci - 1) - ii, (ci - 1) - ii, (ci - 1) - ii);
        cube_draw(1, delay);
        fill_cube(0x00);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
    }
}

// �������� ��������� �� ����� ������� ���� �� ������,
// ���� ��� �����, �� ���������� �����...
void sendvoxel_axis(unsigned char z, unsigned char y, unsigned char x, char axis, int delay) {
    int i, ii;

    if (axis==1) {
        for (i=0; i<CUBE_SIZE; i++) {
            if (z==(CUBE_SIZE - 1)) {
                ii=(CUBE_SIZE - 1) - i;
                clrvoxel(ii + 1, y, x);
            } else {
                ii=i;
                clrvoxel(ii - 1, y, x);
            }
            setvoxel(ii, y, x);
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
        }
    }

    if (axis==2) {
        for (i=0; i<CUBE_SIZE; i++) {
            if (y==(CUBE_SIZE - 1)) {
                ii=(CUBE_SIZE - 1) - i;
                clrvoxel(z, ii + 1, x);
            } else {
                ii=i;
                clrvoxel(z, ii - 1, x);
            }
            setvoxel(z, ii, x);
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
        }
    }

    if (axis==3) {
        for (i=0; i<CUBE_SIZE; i++) {
            if (x==(CUBE_SIZE - 1)) {
                ii=(CUBE_SIZE - 1) - i;
                clrvoxel(z, y, ii + 1);
            } else {
                ii=i;
                clrvoxel(z, y, ii - 1);
            }
            setvoxel(z, y, ii);
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
        }
    }
}

// �������� ��� ���������� �� ����� ������� ���� �� ������
// ������ � z � ���������� � ������������ ������� �� ��������������� �������
void effect_sendplane_rand_z(unsigned char z, int delay, int wait) {
    unsigned char x, y, axis=1, loop=CUBE_SIZE * 2; // loop=16
    fill_cube(0x00);
    fill_layer(z, 0xff);

    // �������� �������� �������� �����������, ��� ��� �� ��� loop=16 ��������� ���
    while (loop) {
        x=rand() % CUBE_SIZE; // rand()%(CUBE_SIZE/2)
        y=rand() % CUBE_SIZE; // rand()%(CUBE_SIZE/2)
        if (getvoxel(z, y, x)) {
            // �������� �������� ������� � ����
            sendvoxel_axis(z, y, x, axis, delay);
            cube_draw(1, wait); // �������� � ������� ������� ���������� �������������� ���
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            loop--; // ���� ��� ���� (����--), ����� ���������� ����� �� 0, ����� �� �����
        }
    }
}

// ��� ������ ���������� ����� ���� X � Y, ���������� ���������
// ��������������� ��� �� ������ 0 ��� �� ������ CUBE_SIZE ��� n ��������.
// ����� ���� ������������ ��������� ���������� �� ��������������� ������� ����.
void effect_sendvoxels_rand_axis(int iterations, char axis, char delay, char wait) {
    unsigned char x, y, z, i, last_x=0, last_y=0, last_z=0;
    fill_cube(0x00);

    if (axis==1) // ������ ����
    {
        // ���������� ����� ��� x,y ����������
        for (x=0; x<CUBE_SIZE; x++) {
            for (y=0; y<CUBE_SIZE; y++) {
                // ����� ��������������� �������� ������� ��� �� ��� ��� �� ���� ����.
                // rand()%2 ���������� 0 ��� 1, ��������� ��� 0 ��� 7.
                setvoxel(((rand() % 2)*(CUBE_SIZE - 1)), y, x);
            }
        }
        for (i=0; i<iterations; i++) {
            // ����� ������������ x,y �������
            x=rand() % CUBE_SIZE;
            y=rand() % CUBE_SIZE;
            // �� �� ���� � ��� �� ������ ������
            if (y != last_y && x != last_x) {
                // ���� �������� ������� x,y �����,
                if (getvoxel(0, y, x)) {
                    // �� �� ���������� ������
                    sendvoxel_axis(0, y, x, axis, delay);
                } else {
                    // ���� �������� ������� ������, �� �� ���������� ������
                    sendvoxel_axis((CUBE_SIZE - 1), y, x, axis, delay);
                }
                cube_draw(1, wait); // �������� � ������� ������� ���������� �������������� ���
                if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
                // ����������� ���������� �����������
                last_y=y;
                last_x=x;
            }
        }
    }

    if (axis==2) {
        // ���������� ����� ��� x,y ����������
        for (x=0; x<CUBE_SIZE; x++) {
            for (y=0; y<CUBE_SIZE; y++) {
                // ����� ��������������� �������� ������� ��� �� ��� ��� �� ���� ����.
                // rand()%2 ���������� 0 ��� 1, ��������� ��� 0 ��� 7.
                setvoxel(y, ((rand() % 2)*(CUBE_SIZE - 1)), x);
            }
        }
        for (i=0; i<iterations; i++) {
            // ����� ������������ x,y �������
            x=rand() % CUBE_SIZE;
            y=rand() % CUBE_SIZE;
            // �� �� ���� � ��� �� ������ ������
            if (y != last_y && x != last_x) {
                // ���� �������� ������� x,y �����,
                if (getvoxel(y, 0, x)) {
                    // �� �� ���������� ������
                    sendvoxel_axis(y, 0, x, axis, delay);
                } else {
                    // ���� �������� ������� ������, �� �� ���������� ������
                    sendvoxel_axis(y, (CUBE_SIZE - 1), x, axis, delay);
                }
                cube_draw(1, wait); // �������� � ������� ������� ���������� �������������� ���
                if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
                // ����������� ���������� �����������
                last_y=y;
                last_x=x;
            }
        }
    }

    if (axis==3) {
        // ���������� ����� ��� x,y ����������
        for (x=0; x<CUBE_SIZE; x++) {
            for (y=0; y<CUBE_SIZE; y++) {
                // ����� ��������������� �������� ������� ��� �� ��� ��� �� ���� ����.
                // rand()%2 ���������� 0 ��� 1, ��������� ��� 0 ��� 7.
                setvoxel(y, x, ((rand() % 2)*(CUBE_SIZE - 1)));
            }
        }
        for (i=0; i<iterations; i++) {
            // ����� ������������ x,y �������
            x=rand() % CUBE_SIZE;
            y=rand() % CUBE_SIZE;
            // �� �� ���� � ��� �� ������ ������
            if (y != last_y && x != last_x) {
                // ���� �������� ������� x,y �����,
                if (getvoxel(y, x, 0)) {
                    // �� �� ���������� ������
                    sendvoxel_axis(y, x, 0, axis, delay);
                } else {
                    // ���� �������� ������� ������, �� �� ���������� ������
                    sendvoxel_axis(y, x, (CUBE_SIZE - 1), axis, delay);
                }
                cube_draw(1, wait); // �������� � ������� ������� ���������� �������������� ���
                if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
                // ����������� ���������� �����������
                last_y=y;
                last_x=x;
            }
        }
    }
}

// ���������� ����� �����
void effect_stringfly(char *str) {
    int x, y, z, i, ii;
    unsigned char chr[];
    fill_cube(0x00);

    while (*str) {
        font_getchar(*str++, chr);

        for (z=(CUBE_SIZE - 1); z>=0; z--) {
            for (y=(CUBE_SIZE - 1); y>=0; y--) {
                if (chr[z] >> y & 0x01) {
                    setvoxel(y, z, 0);
                }
            }
        }

        // Shift the entire contents of the cube forward by 6 steps
        // before placing the next character
        for (i=0; i<CUBE_SIZE; i++) {
            cube_draw(1, 3);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            shift(3, 1);
        }
        delay_ms(100); // �������� ����� �������
    }
}

// ���������� ����� (�� ������ ��� ���) ��� ������ �������������� �� ����
void effect_boingboing(int iterations, char delay, unsigned char mode, unsigned char drawmode) {
    int x, y, z; // Current coordinates for the point                  // mode=0x02;                 // �� ��������
    int dx, dy, dz; // Direction of movement                     // delay=4; mode=0x01; drawmode=0x01;  // �����, �������������� �� ����
    int lol, i; // lol?                                      // delay=2; mode=0x01; drawmode=0x02;  // �����, �������������� �� ���� ����������� ���� � ����������� ���� ���
    unsigned char crash_x, crash_y, crash_z; // delay=3; mode=0x01; drawmode=0x03;  // ������, �������������� �� ����
    int snake[CUBE_SIZE][3];

    fill_cube(0x00); // Blank the cube

    y=rand() % CUBE_SIZE;
    x=rand() % CUBE_SIZE;
    z=rand() % CUBE_SIZE;

    // Coordinate array for the snake
    for (i=0; i<CUBE_SIZE; i++) {
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

        // Let's mix things up a little:
        if (rand() % 3==0) {
            // Pick a random axis, and set the speed to a random number.
            lol=rand() % 3;
            if (lol==0)
                dx=rand() % 3 - 1;

            if (lol==1)
                dy=rand() % 3 - 1;

            if (lol==2)
                dz=rand() % 3 - 1;
        }

        // The point has reached 0 on the x-axis and is trying to go to -1
        // aka a crash
        if (dx==-1 && x==0) {
            crash_x=0x01;
            if (rand() % 3==1) {
                dx=1;
            } else {
                dx=0;
            }
        }

        // y axis 0 crash
        if (dy==-1 && y==0) {
            crash_y=0x01;
            if (rand() % 3==1) {
                dy=1;
            } else {
                dy=0;
            }
        }

        // z axis 0 crash
        if (dz==-1 && z==0) {
            crash_z=0x01;
            if (rand() % 3==1) {
                dz=1;
            } else {
                dz=0;
            }
        }

        // x axis 7 crash
        if (dx==1 && x==(CUBE_SIZE - 1)) {
            crash_x=0x01;
            if (rand() % 3==1) {
                dx=-1;
            } else {
                dx=0;
            }
        }

        // y axis 7 crash
        if (dy==1 && y==(CUBE_SIZE - 1)) {
            crash_y=0x01;
            if (rand() % 3==1) {
                dy=-1;
            } else {
                dy=0;
            }
        }

        // z azis 7 crash
        if (dz==1 && z==(CUBE_SIZE - 1)) {
            crash_z=0x01;
            if (rand() % 3==1) {
                dz=-1;
            } else {
                dz=0;
            }
        }

        // mode bit 0 sets crash action enable
        if (mode | 0x01) {
            if (crash_x) {
                if (dy==0) {
                    if (y==(CUBE_SIZE - 1)) {
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
                    if (z==(CUBE_SIZE - 1)) {
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
                    if (x==(CUBE_SIZE - 1)) {
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
                    if (y==(CUBE_SIZE - 1)) {
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
                    if (x==(CUBE_SIZE - 1)) {
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

        // mode bit 1 sets corner avoid enable       (CUBE_SIZE-1)
        if (mode | 0x02) {
            if (// We are in one of 8 corner positions
                    (x==0 && y==0 && z==0) ||
                    (x==0 && y==0 && z==(CUBE_SIZE - 1)) ||
                    (x==0 && y==(CUBE_SIZE - 1) && z==0) ||
                    (x==0 && y==(CUBE_SIZE - 1) && z==(CUBE_SIZE - 1)) ||
                    (x==(CUBE_SIZE - 1) && y==0 && z==0) ||
                    (x==(CUBE_SIZE - 1) && y==0 && z==(CUBE_SIZE - 1)) ||
                    (x==(CUBE_SIZE - 1) && y==(CUBE_SIZE - 1) && z==0) ||
                    (x==(CUBE_SIZE - 1) && y==(CUBE_SIZE - 1) && z==(CUBE_SIZE - 1))
                    ) {
                // At this point, the voxel would bounce
                // back and forth between this corner,
                // and the exact opposite corner
                // We don't want that!

                // So we alter the trajectory a bit,
                // to avoid corner stickyness
                lol=rand() % 3;
                if (lol==0)
                    dx=0;

                if (lol==1)
                    dy=0;

                if (lol==2)
                    dz=0;
            }
        }

        // one last sanity check
        if (x==0 && dx==-1)
            dx=1;

        if (y==0 && dy==-1)
            dy=1;

        if (z==0 && dz==-1)
            dz=1;

        if (x==(CUBE_SIZE - 1) && dx==1)
            dx=-1;

        if (y==(CUBE_SIZE - 1) && dy==1)
            dy=-1;

        if (z==(CUBE_SIZE - 1) && dz==1)
            dz=-1;


        // Finally, move the voxel.
        x=x + dx;
        y=y + dy;
        z=z + dz;

        // show one voxel at time (���� ���������)
        if (drawmode==0x01)
        {
            setvoxel(z, y, x);
            cube_draw(1, delay);
            clrvoxel(z, y, x);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
        }
        // flip the voxel in question (���������� � ���������� ���������, ����������� ����)
        if (drawmode==0x02)
        {
            // flpvoxel(z,y,x); // �������� �������
            setvoxel(z, y, x);
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
        }
        // draw a snake (������)
        if (drawmode==0x03)
        {
            for (i=(CUBE_SIZE - 1); i>=0; i--) {
                snake[i][0]=snake[i - 1][0];
                snake[i][1]=snake[i - 1][1];
                snake[i][2]=snake[i - 1][2];
            }
            snake[0][0]=x;
            snake[0][1]=y;
            snake[0][2]=z;

            for (i=0; i<CUBE_SIZE; i++) {
                setvoxel(snake[i][2], snake[i][1], snake[i][0]);
            }
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            for (i=0; i<CUBE_SIZE; i++) {
                clrvoxel(snake[i][2], snake[i][1], snake[i][0]);
            }
        }
        iterations--;
    }
}

// ���������������� ������ ��� ������� effect_z_updown_move � effect_axis_updown_randsuspend
void draw_positions_axis(char axis, unsigned char positions[CUBE_SIZE*CUBE_SIZE], int invert) {
    int x, y, p;
    fill_cube(0x00);

    for (x=0; x<CUBE_SIZE; x++) {
        for (y=0; y<CUBE_SIZE; y++) {
            if (invert) {
                p=(CUBE_SIZE - 1) - positions[x * CUBE_SIZE + y];
            } else {
                p=positions[x * CUBE_SIZE + y];
            }
            if (axis==1) // AXIS_Z
                setvoxel(p, y, x); // (x,y,p)

            if (axis==2) // AXIS_Y
                setvoxel(y, p, x); // (x,p,y)

            if (axis==3) // AXIS_X
                setvoxel(x, y, p); // (p,y,x)
        }
    }
}

// ���������������� ������ ��� ������� effect_z_updown
void effect_z_updown_move(unsigned char positions[CUBE_SIZE*CUBE_SIZE], unsigned char destinations[CUBE_SIZE*CUBE_SIZE], char axis) {
    int px;

    for (px=0; px<(CUBE_SIZE * CUBE_SIZE); px++) {
        if (positions[px]<destinations[px]) {
            positions[px]++;
        }
        if (positions[px]>destinations[px]) {
            positions[px]--;
        }
    }
    draw_positions_axis(1, positions, 0); // (AXIS_Z, positions,0)
}

// ��������� ���������, ��������� ������������� � �� ����
// � ������������ �� ��������������� �������
void effect_axis_updown_randsuspend(char axis, char delay, int sleep, char invert) {
    unsigned char positions[CUBE_SIZE * CUBE_SIZE];
    unsigned char destinations[CUBE_SIZE * CUBE_SIZE];
    int i, px;
    fill_cube(0x00);

    // Set 64 random positions
    for (i=0; i<(CUBE_SIZE * CUBE_SIZE); i++) {
        positions[i]=0; // Set all starting positions to 0
        destinations[i]=rand() % CUBE_SIZE;
    }

    // Loop 8 times to allow destination 7 to reach all the way
    for (i=0; i<CUBE_SIZE; i++) {
        // For every iteration, move all position one step closer to their destination
        for (px=0; px<(CUBE_SIZE * CUBE_SIZE); px++) {
            if (positions[px]<destinations[px]) {
                positions[px]++;
            }
        }
        // Draw the positions and take a nap
        draw_positions_axis(axis, positions, invert);
        cube_draw(1, delay); // delay_ms(delay);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
    }

    // Set all destinations to 7 (opposite from the side they started out)
    for (i=0; i<(CUBE_SIZE * CUBE_SIZE); i++) {
        destinations[i]=CUBE_SIZE - 1;
    }

    // Suspend the positions in mid-air for a while
    cube_draw(1, sleep); // delay_ms(sleep);

    // Then do the same thing one more time
    for (i=0; i<CUBE_SIZE; i++) {
        for (px=0; px<(CUBE_SIZE * CUBE_SIZE); px++) {
            if (positions[px]<destinations[px]) {
                positions[px]++;
            }
            if (positions[px]>destinations[px]) {
                positions[px]--;
            }
        }
        draw_positions_axis(axis, positions, invert);
        cube_draw(1, delay); // delay_ms(delay);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
    }

    // Suspend the positions in top-bottom for a while
    cube_draw(1, sleep * 2);
}

// ������������� ��������� ����������� � ������ ����
// � ����������� �� ����� ����� � ������ � ���
void effect_z_updown(int iterations) {
    unsigned char positions[CUBE_SIZE * CUBE_SIZE];
    unsigned char destinations[CUBE_SIZE * CUBE_SIZE];
    int i, y, move, delay;

    for (i=0; i<(CUBE_SIZE * CUBE_SIZE); i++) {
        positions[i]=CUBE_SIZE / 2;
        destinations[i]=rand() % CUBE_SIZE;
    }
    for (i=0; i<CUBE_SIZE; i++) {
        effect_z_updown_move(positions, destinations, 1); // (positions, destinations, AXIS_Z)
        cube_draw(1, 4); //  cube_draw(1,delay);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
    }
    for (i=0; i<iterations; i++) {
        for (move=0; move<CUBE_SIZE; move++) {
            effect_z_updown_move(positions, destinations, 1); // (positions, destinations, AXIS_Z)
            cube_draw(1, 4); //  cube_draw(1,delay);
        }
        cube_draw(1, 100); //  cube_draw(1,delay*CUBE_SIZE/2);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
        for (y=0; y<(CUBE_SIZE * CUBE_SIZE / 2); y++) {
            destinations[rand() % (CUBE_SIZE * CUBE_SIZE)]=rand() % CUBE_SIZE;
        }
    }
}

// ���������� ����� ����� ��� ������ � ����������� �� ��������������� �������
void effect_boxside_randsend_parallel(char axis, char origin, char delay, char mode) {
    char i;
    char done;
    unsigned char cubepos[CUBE_SIZE * CUBE_SIZE];
    unsigned char pos[CUBE_SIZE * CUBE_SIZE];
    char notdone=1;
    char notdone2=1;
    char sent=0;
    fill_cube(0x00);

    for (i=0; i<(CUBE_SIZE * CUBE_SIZE); i++) {
        pos[i]=0;
    }

    while (notdone) {
        if (mode==1) {
            notdone2=1;
            while (notdone2 && sent<(CUBE_SIZE * CUBE_SIZE)) {
                i=rand() % (CUBE_SIZE * CUBE_SIZE);
                if (pos[i]==0) {
                    sent++;
                    pos[i] += 1;
                    notdone2=0;
                }
            }
        } else if (mode==2) {
            if (sent<(CUBE_SIZE * CUBE_SIZE)) {
                pos[sent] += 1;
                sent++;
            }
        }
        done=0;
        for (i=0; i<(CUBE_SIZE * CUBE_SIZE); i++) {
            if (pos[i]>0 && pos[i]<(CUBE_SIZE - 1)) {
                pos[i] += 1;
            }
            if (pos[i]==(CUBE_SIZE - 1))
                done++;
        }
        if (done==(CUBE_SIZE * CUBE_SIZE))
            notdone=0;

        for (i=0; i<(CUBE_SIZE * CUBE_SIZE); i++) {
            if (origin==0) {
                cubepos[i]=pos[i];
            } else {
                cubepos[i]=((CUBE_SIZE - 1) - pos[i]);
            }
        }
        cube_draw(1, delay); // delay_ms(delay);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
        draw_positions_axis(axis, cubepos, 0);
        // LED_PORT ^= LED_RED;
    }
    cube_draw(1, 80); // ����� � ������� ����������
}

// ���������� ��������� ���� �����������,
// ����� ���������������� �� ���������
void effect_loadbar(int delay) {
    int z, y, x;
    fill_cube(0x00);

    for (z=(CUBE_SIZE - 1); z>=0; z--) {
        for (y=(CUBE_SIZE - 1); y>=0; y--) {
            for (x=(CUBE_SIZE - 1); x>=0; x--) {
                setvoxel(z, y, x);
                cube_draw(1, delay);
                if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            }
            // cube_draw(1,delay); // delay_msec(delay);
        }
    }

    cube_draw(1, 100);

    for (z=0; z<CUBE_SIZE; z++) {
        for (y=0; y<CUBE_SIZE; y++) {
            for (x=0; x<CUBE_SIZE; x++) {
                clrvoxel(z, y, x);
                cube_draw(1, delay);
                if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            }
            // cube_draw(1,delay);  // delay_ms(delay);
        }
    }
}

// ��������� n (int voxels) ����������� � ��������� �������
// (���������������� ������ ��� ������� effect_random_sparkle)
void effect_random_sparkle_flash(int iterations, int voxels, int delay) {
    int i, v;
    fill_cube(0x00);
    for (i=0; i<iterations; i++) {
        for (v=0; v<=voxels; v++)
            setvoxel(rand() % CUBE_SIZE, rand() % CUBE_SIZE, rand() % CUBE_SIZE);

        cube_draw(1, delay);
        fill_cube(0x00);
    }
}

// ������������ ��������� �����������
// ��������� 1 ���������� ����������, 2 ��������� �����������...
// � ��� ����� �� (CUBE_SIZE*x) � ������� �� ������ ����������
void effect_random_sparkle(void) {
    int i;

    if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
    
    for (i=0; i<(CUBE_SIZE * 3); i++) // i<20 - CUBE_SIZE*2.5
    {
        effect_random_sparkle_flash(3, i, 3);
    }
    delay_ms(100);
    for (i=(CUBE_SIZE * 3); i>=0; i--) // i=20 - CUBE_SIZE*2.5
    {
        effect_random_sparkle_flash(3, i, 3);
    }
}

// ����������� �����
void effect_wormsqueeze(int axis, int direction, int iterations, int delay) {
    int x, y, i, j, k, dx, dy, size, cube_size;
    int origin=0;

    if (CUBE_SIZE==4)
        size=1;
    if (CUBE_SIZE==8)
        size=2;

    if (direction==-1)
        origin=(CUBE_SIZE - 1);

    cube_size=CUBE_SIZE - (size - 1);

    x=rand() % CUBE_SIZE;
    y=rand() % CUBE_SIZE;

    for (i=0; i<iterations; i++) {
        dx=((rand() % (CUBE_SIZE / 2 - 1)) - 1); // %3
        dy=((rand() % (CUBE_SIZE / 2 - 1)) - 1); // %3

        if ((x + dx)>0 && (x + dx)<CUBE_SIZE)
            x += dx;

        if ((y + dy)>0 && (y + dy)<CUBE_SIZE)
            y += dy;

        shift(axis, direction);

        for (j=0; j<size; j++) {
            for (k=0; k<size; k++) {
                if (axis==1) // AXIS_Z
                {
                    setvoxel(origin, y + k, x + j);
                    origin--;
                } // setvoxel(x+j,y+k,origin);

                if (axis==2) // AXIS_Y
                    setvoxel(y + k, origin, x + j); // setvoxel(x+j,origin,y+k);

                if (axis==3) // AXIS_X
                    setvoxel(x + k, y + j, origin); // setvoxel(origin,y+j,x+k);
            }
        }
        cube_draw(1, delay); // delay_ms(delay);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
    }
}

// ...
const unsigned char paths_8[44]={// circle, len 16, offset 28
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

// ��������������� ������ ��� effect_pathspiral � effect_path_bitmap
void effect_pathmove(unsigned char *path, int length) {
    int i, z;
    unsigned char state;

    // �������� ������ ������
    for (i=(length - 1); i>=1; i--) {
        for (z=0; z<CUBE_SIZE; z++) {
            state=getvoxel(z, CUBE_SIZE - 1 - (path[(i - 1)] >> 4 & 0x0f), CUBE_SIZE - 1 - ((path[(i - 1)])&0x0f));
            altervoxel(z, CUBE_SIZE - 1 - (path[i] >> 4 & 0x0f), CUBE_SIZE - 1 - ((path[i])&0x0f), state);
        }
    }
    for (z=0; z<CUBE_SIZE; z++)
        clrvoxel(z, CUBE_SIZE - 1 - (path[0] >> 4 & 0x0f), CUBE_SIZE - 1 - ((path[0])&0x0f));
    /*
      // �������� ����� ������� (��� �������� ���������, � ����� �����������)
        for (i=(length-1); i>=1; i--)
        {
            for (z=0; z<CUBE_SIZE; z++)
            {
                state=getvoxel(z, (path[(i-1)]&0x0f), ((path[(i-1)]>>4)&0x0f));
                altervoxel(z, (path[i]&0x0f), ((path[i]>>4)&0x0f), state);
            }
        }
        for (i=0; i<CUBE_SIZE; i++)
            clrvoxel(i, (path[0]&0x0f), ((path[0]>>4)&0x0f));
     */
}

// ����������� �������
void effect_pathspiral(int iterations, int delay) {
    int i;
    unsigned char path[28];
    fill_cube(0x00);
    font_getpath(1, path, 28);

    for (i=0; i<iterations; i++) {
        setvoxel(i % 9, (CUBE_SIZE - 1), CUBE_SIZE / 2); // setvoxel(4,0,i%8);
        cube_draw(1, delay);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
        effect_pathmove(path, 28);
    }
}

// ���������� ��������� ����� �� ��������� ����
void effect_rand_patharound(int iterations, int delay) {
    int z=4, dz, i;
    unsigned char path[28];
    fill_cube(0x00);
    font_getpath(0, path, 28);

    for (i=0; i<iterations; i++) {
        dz=((rand() % (CUBE_SIZE / 2 - 1)) - 1);
        z += dz;

        if (z>(CUBE_SIZE - 1))
            z=(CUBE_SIZE - 1);
        if (z<0)
            z=0;

        effect_pathmove(path, 28);
        setvoxel(z, (CUBE_SIZE - 1), 0);
        cube_draw(1, delay);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
    }
}

// ������������ �� ��������� ���� �����
void effect_path_text(int delay, char *str) {
    int z, i, ii;
    unsigned char path[28]; // 28 - �������� ���� 8x8x8
    unsigned char chr[CUBE_SIZE];
    unsigned char stripe;
    fill_cube(0x00);
    font_getpath(0, path, 28);

    while (*str) {
        font_getchar(*str++, chr);
        for (ii=0; ii<CUBE_SIZE; ii++) // ii<5
        {
            stripe=chr[ii];

            for (z=0; z<CUBE_SIZE; z++) {
                if ((stripe >> z) & 0x01) {
                    setvoxel(z, (CUBE_SIZE - 1), 0);
                } else {
                    clrvoxel(z, (CUBE_SIZE - 1), 0);
                }
            }
            effect_pathmove(path, 28);
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
        }
        effect_pathmove(path, 28);
        cube_draw(1, delay);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
    }
    for (i=0; i<28; i++) {
        effect_pathmove(path, 28);
        cube_draw(1, delay);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
    }
}

// ����������� ��� ������� effect_path_bitmap
volatile const unsigned char bitmaps[13][8]={
    {0xc3, 0xc3, 0x00, 0x18, 0x18, 0x81, 0xff, 0x7e}, // smiley 3 small
    {0x3c, 0x42, 0x81, 0x81, 0xc3, 0x24, 0xa5, 0xe7}, // Omega
    {0x00, 0x04, 0x06, 0xff, 0xff, 0x06, 0x04, 0x00}, // Arrow
    {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81}, // X
    {0xBD, 0xA1, 0xA1, 0xB9, 0xA1, 0xA1, 0xA1, 0x00}, // ifi
    {0xEF, 0x48, 0x4B, 0x49, 0x4F, 0x00, 0x00, 0x00}, // TG
    {0x38, 0x7f, 0xE6, 0xC0, 0xE6, 0x7f, 0x38, 0x00}, // Commodore symbol
    {0x00, 0x22, 0x77, 0x7f, 0x3e, 0x3e, 0x1c, 0x08}, // Heart
    {0x1C, 0x22, 0x55, 0x49, 0x5d, 0x22, 0x1c, 0x00}, // face
    {0x37, 0x42, 0x22, 0x12, 0x62, 0x00, 0x7f, 0x00}, // ST
    {0x89, 0x4A, 0x2c, 0xF8, 0x1F, 0x34, 0x52, 0x91}, // STAR
    {0x18, 0x3c, 0x7e, 0xdb, 0xff, 0x24, 0x5a, 0xa5}, // Space Invader
    {0x00, 0x9c, 0xa2, 0xc5, 0xc1, 0xa2, 0x9c, 0x00}};// Fish

// ��������������� ������ ��� effect_path_bitmap
unsigned char font_getbitmappixel(char bitmap, char x, char y) {
    char tmp=bitmaps[bitmap][x];
    return (tmp >> y) & 0x01;
}

// ������������ �� ��������� ���� �����������
void effect_path_bitmap(int delay, char bitmap, int iterations) {
    int z, i, ii;
    unsigned char path[28]; // 28 - �������� ���� 8x8x8
    fill_cube(0x00);
    font_getpath(0, path, 28);

    for (i=0; i<iterations; i++) {
        for (ii=0; ii<CUBE_SIZE; ii++) {
            for (z=0; z<CUBE_SIZE; z++) {
                if (font_getbitmappixel(bitmap, z, ii)) // (bitmap,(7-z),ii))    ((CUBE_SIZE-1)-z),ii))
                {
                    setvoxel(z, (CUBE_SIZE - 1), 0);
                } else {
                    clrvoxel(z, (CUBE_SIZE - 1), 0);
                }
            }
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            effect_pathmove(path, 28);
        }
        for (ii=0; ii<20; ii++) {
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            effect_pathmove(path, 28);
        }
    }
    for (ii=0; ii<10; ii++) {
        cube_draw(1, delay);
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
        effect_pathmove(path, 28);
    }
}

// ����������� ����������� bitmaps[]
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
        // front:
        for (s=0; s<(CUBE_SIZE - 1); s++) {
            if (!flip) {
                off++;
                if (off==(CUBE_SIZE / 2)) // off==4
                {
                    flip=1;
                    off=0;
                }
            } else
                off++;
            for (x=0; x<CUBE_SIZE; x++) {
                d=0;
                for (y=0; y<CUBE_SIZE; y++) {
                    if (font_getbitmappixel(bitmap, (CUBE_SIZE - 1) - x, y)) {
                        if (!flip)
                            setvoxel((CUBE_SIZE - 1) - x, dybde[CUBE_SIZE * off + d++], y); // setvoxel(y,dybde[CUBE_SIZE*off+d++],x);
                        else
                            setvoxel((CUBE_SIZE - 1) - x, dybde[31 - CUBE_SIZE * off - d++], y); // setvoxel(y,dybde[31-CUBE_SIZE*off-d++],x);
                    } else
                        d++;
                }
            }
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            fill_cube(0x00);
        }

        // side:
        off=0;
        flip=0;
        d=0;
        for (s=0; s<(CUBE_SIZE - 1); s++) {
            if (!flip) {
                off++;
                if (off==(CUBE_SIZE / 2)) // off==4
                {
                    flip=1;
                    off=0;
                }
            } else
                off++;
            for (x=0; x<CUBE_SIZE; x++) {
                d=0;
                for (y=0; y<CUBE_SIZE; y++) {
                    if (font_getbitmappixel(bitmap, (CUBE_SIZE - 1) - x, y)) // (font_getbitmappixel(bitmap,(CUBE_SIZE-1)-x,y))
                    {
                        if (!flip)
                            setvoxel((CUBE_SIZE - 1) - x, y, (CUBE_SIZE - 1) - dybde[CUBE_SIZE * off + d++]); // setvoxel(dybde[CUBE_SIZE*off+d++],(CUBE_SIZE-1)-y,x);
                        else
                            setvoxel((CUBE_SIZE - 1) - x, y, (CUBE_SIZE - 1) - dybde[31 - CUBE_SIZE * off - d++]); // setvoxel(dybde[31-CUBE_SIZE*off-d++],(CUBE_SIZE-1)-y,x);
                    } else
                        d++;
                }
            }
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            fill_cube(0x00);
        }

        flip=0;
        d=0;
        off=0;
        // back:
        for (s=0; s<(CUBE_SIZE - 1); s++) {
            if (!flip) {
                off++;
                if (off==(CUBE_SIZE / 2)) // off==4
                {
                    flip=1;
                    off=0;
                }
            } else
                off++;
            for (x=0; x<CUBE_SIZE; x++) {
                d=0;
                for (y=0; y<CUBE_SIZE; y++) {
                    if (font_getbitmappixel(bitmap, (CUBE_SIZE - 1) - x, (CUBE_SIZE - 1) - y)) {
                        if (!flip)
                            setvoxel((CUBE_SIZE - 1) - x, dybde[CUBE_SIZE * off + d++], y); // setvoxel(y,dybde[CUBE_SIZE*off+d++],x);
                        else
                            setvoxel((CUBE_SIZE - 1) - x, dybde[31 - CUBE_SIZE * off - d++], y); // setvoxel(y,dybde[31-CUBE_SIZE*off-d++],x);
                    } else
                        d++;
                }
            }
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            fill_cube(0x00);
        }

        // other side:
        off=0;
        flip=0;
        d=0;
        for (s=0; s<(CUBE_SIZE - 1); s++) {
            if (!flip) {
                off++;
                if (off==(CUBE_SIZE / 2)) // off==4
                {
                    flip=1;
                    off=0;
                }
            } else
                off++;
            for (x=0; x<CUBE_SIZE; x++) {
                d=0;
                for (y=0; y<CUBE_SIZE; y++) {
                    if (font_getbitmappixel(bitmap, (CUBE_SIZE - 1) - x, (CUBE_SIZE - 1) - y)) {
                        if (!flip)
                            setvoxel((CUBE_SIZE - 1) - x, y, (CUBE_SIZE - 1) - dybde[CUBE_SIZE * off + d++]); // setvoxel(dybde[CUBE_SIZE*off+d++],(CUBE_SIZE-1)-y,x);
                        else
                            setvoxel((CUBE_SIZE - 1) - x, y, (CUBE_SIZE - 1) - dybde[31 - CUBE_SIZE * off - d++]); // setvoxel(dybde[31-CUBE_SIZE*off-d++],(CUBE_SIZE-1)-y,x);
                    } else
                        d++;
                }
            }
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            fill_cube(0x00);
        }

    }
}

// ����������� ���������� �������
void effect_squarespiral1(int iterations, int delay) {
    int loc=0;
    int iter=0;
    while (iter<=iterations) {
        for (loc=0; loc<CUBE_SIZE; loc++) // loc<(CUBE_SIZE-1)
        {
            shift(1, 0); // shift(AXIS_Z,-1);
            setvoxel((CUBE_SIZE - 1), loc, 0);
            setvoxel((CUBE_SIZE - 1), (CUBE_SIZE - 1), loc);
            setvoxel((CUBE_SIZE - 1), (CUBE_SIZE - 1) - loc, (CUBE_SIZE - 1));
            setvoxel((CUBE_SIZE - 1), 0, (CUBE_SIZE - 1) - loc);

            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            iter++;
        }
        loc=0;
    }
}

// ����������� ������� ���������� �������
void effect_squarespiral2(int iterations, int delay) {
    int loc=0;
    int iter=0;
    while (iter<=iterations) {
        for (loc=0; loc<CUBE_SIZE; loc++) // loc<(CUBE_SIZE-1);
        {
            shift(1, 0); // shift(AXIS_Z,-1);
            setvoxel((CUBE_SIZE - 1), loc, 0);
            setvoxel((CUBE_SIZE - 1), (CUBE_SIZE - 1), loc);
            setvoxel((CUBE_SIZE - 1), (CUBE_SIZE - 1) - loc, (CUBE_SIZE - 1));
            setvoxel((CUBE_SIZE - 1), 0, (CUBE_SIZE - 1) - loc);
            setvoxel((CUBE_SIZE - 1), (CUBE_SIZE - 1) - loc, 0);
            setvoxel((CUBE_SIZE - 1), (CUBE_SIZE - 1), (CUBE_SIZE - 1) - loc);
            setvoxel((CUBE_SIZE - 1), loc, (CUBE_SIZE - 1));
            setvoxel((CUBE_SIZE - 1), 0, loc);

            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
            iter++;
        }
        loc=0;
    }
}

// ���������
void effect_fireworks(char iterations) {
    char i, f, z;
    char n=80; // ���������� ��������
    char origin_x, origin_y, origin_z;
    //float slowrate, gravity;  // ���������� � ����������

    // Particles and their position, x,y,z and their movement, dx, dy, dz
    char particles[80][6]; // float particles[n][6]

    fill_cube(0x00);

    for (i=0; i<iterations; i++) {
        if (fl_button) { fl_button=0; return; }  // ���������� �� ������� ������
        // ����������� ��������� ��������� ����� ��������
        origin_x=rand() % 4 + 2; // (0..3)+2=2..5
        origin_y=rand() % 4 + 2; // (0..3)+2=2..5
        // ������ ������� ��������
        origin_z=1;

        // ������������� ������� �� ���������� �������������� �� ������ ���������
        for (z=(CUBE_SIZE - 1); z>=origin_z; z--) {
            setvoxel(z, origin_y, origin_x);
            cube_draw(1, (5 - TAN((z + 0, 1) / 20)*10)); // ��������� ������� ������� � �����������
            fill_cube(0x00);
        }

        // ���������� ������� ������
        for (f=0; f<n; f++) {
            // �������
            particles[f][0]=origin_x;
            particles[f][1]=origin_y;
            particles[f][2]=origin_z;

            // �����������
            particles[f][3]=1 - rand() % 3; // dx  //=1-(float)rand_x/100;
            particles[f][4]=1 - rand() % 3; // dy
            particles[f][5]=1 + rand() % 2; // dz
        }

        // �����
        for (z=0; z<5; z++) // ���������� �������
        {
            // slowrate=1+tan((z+0.1)/20)*10;
            // gravity =tan((z+0.1)/20)/2;
            for (f=0; f<n; f++) // ���������� ��������
            {
                particles[f][0] += particles[f][3];
                particles[f][1] += particles[f][4];
                particles[f][2] += particles[f][5] - rand() % 2;
                if (particles[f][2]==0) {
                    particles[f][2]++;
                }
                setvoxel(particles[f][2], particles[f][1], particles[f][0]);
            }
            cube_draw(1, 4); // ��������� ������
            fill_cube(0x00);
        }
        delay_ms(600); // ����� ����� ��������
    }
}

/* ------------------------------------------------------------------------ */
/*                          ����� ������� ��������                          */
/* ------------------------------------------------------------------------ */




/* ------------------------------------------------------------------------ */
/*                              ������ ��������                             */
/* ------------------------------------------------------------------------ */

void launch_effect(char effect) {
    char i, bitmap;

    // ����� ������� ��� �������
    switch (effect) {
        case 0: // ��������
            // effect_loadbar(1);  // ���������, ����� ������� �� �������� ����
            break;
        case 1: // ��������
            for (i=1; i<4; i++)
                effect_planboing(i,3);
            delay_msec(100);
            break;
        case 2:
            // ��������
            effect_rain(500);
            break;
        case 3:
            // ��������
            effect_box_shrink_grow();
            effect_box_woopwoop(4, 0);
            effect_box_woopwoop(4, 1);
            effect_box_woopwoop(4, 0);
            effect_box_woopwoop(4, 1);
            effect_box_woopwoop(4, 0);
            effect_box_woopwoop(4, 1);
            break;
        case 4:
            // ��������
            effect_sendvoxels_rand_axis(50, 2, 1, 5);  // �����-�������  // (int iterations, char axis, int delay, int wait)
            effect_sendvoxels_rand_axis(50, 3, 1, 5);  // �����-�����
            break;
        case 5:
            // ��������
            effect_stringfly("HELLO WORLD");
            break;
        case 6:
            // ��������
            effect_boingboing(200, 4, 0x01, 0x01); // �����, �������������� �� ����
            effect_boingboing(500, 2, 0x01, 0x02); // �����, �������������� �� ���� ����������� ���� � ����������� ���� ���
            effect_boingboing(300, 3, 0x01, 0x03); // ������, �������������� �� ����
            break;
        case 7:
            // ��������
            effect_boxside_randsend_parallel(2, 0, 1, 2);  // (char axis, int origin, int delay, int mode)
            effect_boxside_randsend_parallel(2, 1, 1, 2);
            effect_boxside_randsend_parallel(3, 0, 1, 2);
            effect_boxside_randsend_parallel(3, 1, 1, 2);
            break;
        case 8:
            // ��������
            effect_random_sparkle();
            break;
        case 9:
            // ��������
            effect_wormsqueeze(2, 0, 200, 4);
            effect_wormsqueeze(3, 1, 200, 4);
            break;
        case 10:
            effect_rand_patharound(1000, 3);
            break;
        case 11:
            // ��������
            effect_smileyspin(2, 3, 0); // ��������� ��������
            for (i=7; i<9; i++) {
                effect_smileyspin(5, 3, i);
            } // (int count, int delay, char bitmap)
            break;
        case 12:
            // ��������
            effect_path_text(6, "HELLO WORLD");
            break;
        case 13:
            // ��������
            effect_axis_updown_randsuspend(3, 2, 50, 0); // (char axis, int delay, int sleep, int invert)
            effect_axis_updown_randsuspend(3, 2, 50, 1);
            effect_axis_updown_randsuspend(2, 2, 50, 0);
            effect_axis_updown_randsuspend(2, 2, 50, 1);
            break;
        case 14:
            // ��������
            effect_z_updown(10);
            break;
        case 15:
            // ��������
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
            // ��������
            effect_fireworks(3);
            break;
        case 18:
            // ��������
            effect_pathspiral(64, 4);
            break;
        case 19:
            // ��������
            for (bitmap=0; bitmap<14; bitmap++) {
                effect_path_bitmap(5, bitmap, 1); // (int delay, char bitmap, int iterations)
            }
            break;
        case 20: // ��������
            effect_squarespiral1(200, 4);
            effect_squarespiral2(200, 4);
            break;
        case 21:
            effect_planboing(1, 3);
            break;
        default: // � ������, ���� ������ ����� �� ������������� �������
            effect_stringfly("ERROR");
            break;
    }
}



/* ------------------------------------------------------------------------ */
/*                          ����� ������� ��������                          */
/* ------------------------------------------------------------------------ */





/* ------------------------------------------------------------------------ */
/*                          �������� ���� ���������                         */
/* ------------------------------------------------------------------------ */

// PIC ��������� ��� ������� ����� �������
void main(void) {
    INTCON  = 0b11110000; // ���������� ����������
    INTCON2 = 0b10000101; // ���������� ����������
    RCON    = 0b00000000; // ���������� ����������� ������� ����������
    T0CON   = 0b11000101; // ���������� �������� TMR0
    CMCON   = 0b00000111;
    PR2     = 0b00000000;
    PIE1    = 0b00000000;
    ADCON0  = 0b00000000; // ���� ���������� ������� ��� (��������)
    ADCON1  = 0b00001110; // ���� ���������� ������� ��� (AN7-AN1 - digital, AN0 - analog)
    LVDCON  = 0b00000000; // ���� ���������� ������� LVD (��������)
    PORTA   = 0b00000000; // 8-��������� ���� ����� ������ PORTA
    TRISA   = 0b01000000; // ���� ���������� ������������ PORTA (RA6 - ���� �� ���������� ����������)
    PORTB   = 0b00000000; // 8-��������� ���� ����� ������ PORTB
    TRISB   = 0b00000001; // ���� ���������� ������������ PORTB (RB0 - ���� �� ������)
    PORTC   = 0b00000000; // 8-��������� ���� ����� ������ PORTC
    TRISC   = 0b00000000; // ���� ���������� ������������ PORTC
    // 76543210   // ��������� ������   // ��������� �����������!

    while (1) {
        // ������ ���������� ������� ��� ������ ������� ����
        if (fl_start==0) {                                                        // ((fl_start==0) & EEprom_Read(0)==0xff )
            fl_start=1;
            effect=rand()%EFFECTS_TOTAL;
        }

        launch_effect(effect);
    }
}

/* ------------------------------------------------------------------------ */
/*                     ����� ��������� ����� ���������                      */
/* ------------------------------------------------------------------------ */

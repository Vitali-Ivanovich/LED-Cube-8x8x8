
// !!! BETTA VERSION !!!
 
/*
 * Project: LED Cube | cube8_core
 * Version: v3.0 | 2013.12.11-2017.09.24
 * https://github.com/murach-vi/LED-Cube-8x8x8
 * CC BY-NC-SA 4.0 2017 Murach Vitali (murach.vi@gmail.com)
 * License: https://creativecommons.org/licenses/by-nc-sa/4.0

 * Configuration:
    MCU: PIС18F2550
    Oscillator: 20MHz
    Shift registers: 74HC595 x8
*/

#define SH_CP         PORTA.F0
#define DS            PORTA.F1
#define ST_CP         PORTA.F2
#define CUBE_SIZE     8
#define CUBE_BYTES    512
#define EFFECTS_TOTAL 22

/*
// Game of Life for the 4x4x4 and 8x8x8 led cube
#define GOL_CREATE_MIN 3
#define GOL_CREATE_MAX 3
// Added 1 to each of these.  Current cell is always alive
// when checking them!  Originally 3 and 5
#define GOL_TERMINATE_LONELY 3
#define GOL_TERMINATE_CROWDED 5
#define GOL_X 8
#define GOL_Y 8
#define GOL_Z 8
#define GOL_WRAP 0x01
*/

volatile char cube[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE];
volatile char buffer[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE];
char effect=1, fl_button=0, fl_start=0, fl_reset=0;
const double PI=3.14159;

// обработка прерываний

void interrupt(void) {
    INTCON.GIE=0x00;
    // обработка прерываний по нажатию кнопки
    if (INTCON.INT0IF) {
        if (effect<EFFECTS_TOTAL) {
            effect++;
        } else {
            effect=1;
        }
        delay_ms(1500); // задержка для исключения дребезга
        fl_button=1;
        fl_reset=1;
        INTCON.INT0IF=0x00;
        // asm reset
    }
    // обработка прерываний по времени
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
/*                              ФУНКЦИИ РИСОВАНИЯ                           */
/* ------------------------------------------------------------------------ */

// Проверка координат xyz на соответствие размерам куба
char inrange(char z, char y, char x) {
    if (x>=0 && x<CUBE_SIZE && y>=0 && y<CUBE_SIZE && z>=0 && z<CUBE_SIZE) {
        return 1;
    } else {
        // Одна из координат за пределами куба
        return 0;
    }
}

// Время задержки перед защелкиванием данных: speed=1ms * delay
void delay_msec(int speed) {
    int delay;
    for (delay=1; delay<=speed; delay++) {
        delay_ms(1);
    }
}

// Подсветка этажей куба
void cube_draw(int speed, int iterations) {
    int x, y, z, i;

    for (i=0; i<iterations; i++) {
        // Записывание 64 бита в свиговые регистры
        for (z=0; z<CUBE_SIZE; z++) {
            for (y=(CUBE_SIZE - 1); y>=0; y--) {
                for (x=(CUBE_SIZE - 1); x>=0; x--) {
                    DS=cube[z][y][x];
                    // Записываем бит в регистр
                    SH_CP=1;
                    SH_CP=0;
                }
            }
            // Поочерёдная подсветка этажей куба с управляемой задержкой speed
            // Дергаем ногой и защёлкиваем данные
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
            // задержка между подсветкой этажей
            delay_msec(speed);
            // Защелкивание данных
            ST_CP=0;
            PORTB=0b00000000; // layer0-layer6
            PORTC=0b00000000; // layer7
            // Окончание записывания данных в сдвиговые регистры

        }
    }
}

// Включение светодиода
void setvoxel(char z, char y, char x) {
    if (inrange(z, y, x)) {
        cube[z][y][x]=0xff;
    }
}

// Выключение светодиода
void clrvoxel(char z, char y, char x) {
    if (inrange(z, y, x))
        cube[z][y][x]=0x00;
}

// Определение состояния (включен/выключен) светодиода
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

// Изменение состояния светодиода (включение/выключение - state)
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

// Инвертирование состояния светодиода.
// Если был включен (1), то выключится (0) и наоборот.
void flpvoxel(char z, char y, char x) {
    if (inrange(z, y, x))
        cube[z][y][x] ^= cube[z][y][x];
}

// Включение одного светодиода во временном буфере
void tmpsetvoxel(char z, char y, char x) {
    if (inrange(z, y, x))
        buffer[z][y][x]=0xff;
}

// Выключение одного светодиода во временном буфере
void tmpclrvoxel(int z, int y, int x) {
    if (inrange(z, y, x))
        buffer[z][y][x]=0x00;
}

// Установка светящейся плоскости по оси X
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

// Погашение светящейся плоскости по оси X
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

// Установка светящейся плоскости по оси Y
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

// Погашение светящейся плоскости по оси Y
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

// Подсветка всех 64 бит этажа
// Используется для погашения всех fill(layer, 0x00)
// или для зажигания всех светодиодов fill(layer, 0xff)
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

// Подсветка всего куба
// Используется для погашения всех fill_cube(0x00)
// или для зажигания всех светодиодов fill_cube(0xff)
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

// Установка светящейся плоскости по заданной оси
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

// Погашение светящейся плоскости по заданной оси
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

// Убеждается, что x1 всегда меньше x2, а если наоборот, то испраляет.
// Используется для предотвращения зацикливания некоторых функций
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

// Рисует полностью светящийся параллелепипед
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

// Рисует полый параллелепипед со светящимися стенами
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

// Рисует полый параллелепипед со светящимися гранями
void box_wireframe(char z1, char y1, char x1, char z2, char y2, char x2) {
    char iz, iy, ix;

    argorder(x1, x2, &x1, &x2);
    argorder(y1, y2, &y1, &y2);
    argorder(z1, z2, &z1, &z2);

    for (iz=z1; iz<=z2; iz++) {
        for (iy=y1; iy<=y2; iy++) {
            for (ix=x1; ix<=x2; ix++) {
                if ((iz>=z1 && iz<=z2 && iy==y1 && ix==x1) || // прямая (z1,y1,x1)-(z2,y1,x1)
                        (iz>=z1 && iz<=z2 && iy==y2 && ix==x1) || // прямая (z1,y2,x1)-(z2,y2,x1)
                        (iz>=z1 && iz<=z2 && iy==y1 && ix==x2) || // прямая (z1,y1,x2)-(z2,y1,x2)
                        (iz>=z1 && iz<=z2 && iy==y2 && ix==x2) || // прямая (z1,y2,x2)-(z2,y2,x2)

                        (iz==z1 && iy==y1 && ix>=x1 && ix<=x2) || // прямая (z1,y1,x1)-(z1,y1,x2)
                        (iz==z1 && iy==y2 && ix>=x1 && ix<=x2) || // прямая (z1,y2,x1)-(z1,y2,x2)
                        (iz==z2 && iy==y1 && ix>=x1 && ix<=x2) || // прямая (z2,y1,x1)-(z1,y1,x2)
                        (iz==z2 && iy==y2 && ix>=x1 && ix<=x2) || // прямая (z2,y2,x1)-(z1,y2,x2)

                        (iz==z1 && iy>=y1 && iy<=y2 && ix==x1) || // прямая (z1,y1,x1)-(z1,y2,x1)
                        (iz==z1 && iy>=y1 && iy<=y2 && ix==x2) || // прямая (z1,y1,x2)-(z1,y2,x2)
                        (iz==z2 && iy>=y1 && iy<=y2 && ix==x1) || // прямая (z2,y1,x1)-(z2,y2,x1)
                        (iz==z2 && iy>=y1 && iy<=y2 && ix==x2)) // прямая (z2,y1,x2)-(z2,y2,x2)
                {
                    cube[iz][iy][ix]=0xff;
                } else {
                    cube[iz][iy][ix]=0x00;
                }
            }
        }
    }
}

// Рисует линию в по трём координатам в 3d пространстве
// Используются целые величины, из-за чего линии не всегда плавные
void line(char z1, char y1, char x1, char z2, char y2, char x2) {
    char x, y, z, last_y, last_z;
    float xy, xz; // количество узловых точек, при прохождении от "y" к "x"
    // тип float повышает точность рисования (на прямых, проходящих
    // не через узлы), но использует ROM на 7% большем, чем тип char

    if (inrange(z1, y1, x1) & inrange(z2, y2, x2)) {
        // Мы всегда рисуем линии от x=0 к x=7
        // Если x1 больше x2, то необходимо поменять координаты местами
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

        // Приращение "y" для каждого шага "x"
        for (x=x1; x<=x2; x++) {
            y=(xy * (x - x1)) + y1;
            z=(xz * (x - x1)) + z1;
            setvoxel(z, y, x);
        }
    }
}

// Копирует содержимое куба в буфер
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

// Копирует содержимое буфера в куб
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

// Переворот куба на 180 градусов вдоль оси "z"
void mirror_z(void) {
    char z, y, x;
    cube2buffer(); // копирует содержимое куба в буфер
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

// Переворот куба на 180 градусов вдоль оси "y"
void mirror_y(void) {
    char z, y, x;
    cube2buffer(); // копирует содержимое куба в буфер
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

// Переворот куба на 180 градусов вдоль оси "x"
void mirror_x(void) {
    char z, y, x;
    cube2buffer(); // копирует содержимое куба в буфер
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

// Перемещение всего содержимого куба вдоль одной оси.
// Используется для эффектов с перетеканиями от одной стороны куба к другой.
// Например, для эффекта дождь используется shift(1,0).
void shift(char axis, char direction) {
    char z, y, x, ii, iii, state;

    if (axis==1) // AXIS_Z - 1
    {
        for (z=0; z<CUBE_SIZE; z++) {
            for (y=0; y<CUBE_SIZE; y++) {
                for (x=0; x<CUBE_SIZE; x++) {
                    if (direction==0) // direction=0 (-1) - по AXIS_Z спускается вниз
                    { // direction=1 - по AXIS_Z подымается вверх
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
                    if (direction==0) // direction=0 (-1) - по AXIS_Y движется влево
                    { // direction=1 - по AXIS_Y движется вправо
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
                    if (direction==0) // direction=0 (-1) - по AXIS_X движется влево
                    { // direction=1 - по AXIS_X движется вправо
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

// Английский алфавит, цифры и знаки для куба 8х8х8
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
// Английский алфавит, цифры и знаки для куба 4х4х4
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

// Русский алфавит, цифры и знаки для куба 4х4х4
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

// Извлечение букв из массива font4(8) и сопоставление им шестнадцатеричного кода
void font_getchar(char chr, unsigned char dst[]) {
    int i;
    chr -= 32; // our bitmap font starts at ascii char 32                   33
    /* Массив скрыт для экономии память
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
/*                           КОНЕЦ ФУНКЦИЙ РИСОВАНИЯ                        */
/* ------------------------------------------------------------------------ */





/* ------------------------------------------------------------------------ */
/*                             ФУНКЦИИ ЭФФЕКТОВ                             */
/* ------------------------------------------------------------------------ */

// Дождь
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
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
    }
}

// Движение плоскостей вперёд и назад
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
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
    }
}

// Мигание
void effect_blinky2() {/*
	int i,r;
  fill_cube(0x00);

	for (r=0; r<2; r++)
	{
		i=200;
		while (i>0)
		{
			fill_cube(0x00);
			cube_draw(1,5);

			fill_cube(0xff);
			cube_draw(1,i);

			i=i-(10+(100/(i/10)));
		}

		delay_ms(1000);

		i=200;
		while (i>0)
		{
			fill_cube(0x00);
			cube_draw(1,5);

			fill_cube(0xff);
			cube_draw(1,201-i);

			i=i-(10+(100/(i/10)));
		}
	}*/
}

// Уменьшение и возрастание полого куба из разных углов
void effect_box_shrink_grow(void) {
    int x, i, ii, xyz;
    int iterations=1;
    int flip=ii & (CUBE_SIZE / 2);
    int delay=2;

    for (ii=0; ii<CUBE_SIZE; ii++) {
        for (x=0; x<iterations; x++) {
            for (i=0; i<(CUBE_SIZE * 2); i++) {
                xyz=(CUBE_SIZE - 1) - i; // Возобновление счётчика i между 0 and (CUBE_SIZE-1)
                if (i>(CUBE_SIZE - 1))
                    xyz=i - CUBE_SIZE; // если i>7, i 8-15 становятся xyz 0-7

                fill_cube(0x00);
                cube_draw(1, 1);
                if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки

                box_wireframe(0, 0, 0, xyz, xyz, xyz);

                if (flip>0) // вверх дном
                    mirror_z();
                if (ii==(CUBE_SIZE - 3) || ii==(CUBE_SIZE - 1))
                    mirror_y();
                if (ii==(CUBE_SIZE - 2) || ii==(CUBE_SIZE - 1))
                    mirror_x();

                cube_draw(1, delay);
                fill_cube(0x00);
                if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
            }
        }
    }
}

// Создает полый куб, который уменьшает или растет из центра куба
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
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
    }
}

// Посылает светодиод от одной стороны куба до другой,
// если был внизу, то посылается вверх...
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
        }
    }
}

// Посылает все светодиоды от одной стороны куба до другой
// Начала в z и посылаются в произвольном порядке на противоположную сторону
void effect_sendplane_rand_z(unsigned char z, int delay, int wait) {
    unsigned char x, y, axis=1, loop=CUBE_SIZE * 2; // loop=16
    fill_cube(0x00);
    fill_layer(z, 0xff);

    // Посылает объемные элементы произвольно, так что бы все loop=16 пересекли куб
    while (loop) {
        x=rand() % CUBE_SIZE; // rand()%(CUBE_SIZE/2)
        y=rand() % CUBE_SIZE; // rand()%(CUBE_SIZE/2)
        if (getvoxel(z, y, x)) {
            // Посылает объёмный элемент в полёт
            sendvoxel_axis(z, y, x, axis, delay);
            cube_draw(1, wait); // Задержка в течении которой непрерывно отрисовывается куб
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
            loop--; // один раз вниз (цикл--), когда светодиоды дошли до 0, выход из цикла
        }
    }
}

// Для каждой координаты вдоль осей X и Y, светящийся светодиод
// установливается или на уровне 0 или на уровне CUBE_SIZE для n итерации.
// После чего произвольный светодиод посылается на противоположную сторону куба.
void effect_sendvoxels_rand_axis(int iterations, char axis, char delay, char wait) {
    unsigned char x, y, z, i, last_x=0, last_y=0, last_z=0;
    fill_cube(0x00);

    if (axis==1) // сверху вниз
    {
        // Соединение через все x,y координаты
        for (x=0; x<CUBE_SIZE; x++) {
            for (y=0; y<CUBE_SIZE; y++) {
                // Затем устанавливается объёмный элемент или на низ или на верх куба.
                // rand()%2 возвращает 0 или 1, умножение даёт 0 или 7.
                setvoxel(((rand() % 2)*(CUBE_SIZE - 1)), y, x);
            }
        }
        for (i=0; i<iterations; i++) {
            // Выбор произвольной x,y позиции
            x=rand() % CUBE_SIZE;
            y=rand() % CUBE_SIZE;
            // но не один и тот же дважды подряд
            if (y != last_y && x != last_x) {
                // Если объёмный элемент x,y внизу,
                if (getvoxel(0, y, x)) {
                    // то он посылается наверх
                    sendvoxel_axis(0, y, x, axis, delay);
                } else {
                    // Если объёмный элемент вверху, то он посылается наверх
                    sendvoxel_axis((CUBE_SIZE - 1), y, x, axis, delay);
                }
                cube_draw(1, wait); // Задержка в течении которой непрерывно отрисовывается куб
                if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
                // Запоминание последнего перемещения
                last_y=y;
                last_x=x;
            }
        }
    }

    if (axis==2) {
        // Соединение через все x,y координаты
        for (x=0; x<CUBE_SIZE; x++) {
            for (y=0; y<CUBE_SIZE; y++) {
                // Затем устанавливается объёмный элемент или на низ или на верх куба.
                // rand()%2 возвращает 0 или 1, умножение даёт 0 или 7.
                setvoxel(y, ((rand() % 2)*(CUBE_SIZE - 1)), x);
            }
        }
        for (i=0; i<iterations; i++) {
            // Выбор произвольной x,y позиции
            x=rand() % CUBE_SIZE;
            y=rand() % CUBE_SIZE;
            // но не один и тот же дважды подряд
            if (y != last_y && x != last_x) {
                // Если объёмный элемент x,y внизу,
                if (getvoxel(y, 0, x)) {
                    // то он посылается наверх
                    sendvoxel_axis(y, 0, x, axis, delay);
                } else {
                    // Если объёмный элемент вверху, то он посылается наверх
                    sendvoxel_axis(y, (CUBE_SIZE - 1), x, axis, delay);
                }
                cube_draw(1, wait); // Задержка в течении которой непрерывно отрисовывается куб
                if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
                // Запоминание последнего перемещения
                last_y=y;
                last_x=x;
            }
        }
    }

    if (axis==3) {
        // Соединение через все x,y координаты
        for (x=0; x<CUBE_SIZE; x++) {
            for (y=0; y<CUBE_SIZE; y++) {
                // Затем устанавливается объёмный элемент или на низ или на верх куба.
                // rand()%2 возвращает 0 или 1, умножение даёт 0 или 7.
                setvoxel(y, x, ((rand() % 2)*(CUBE_SIZE - 1)));
            }
        }
        for (i=0; i<iterations; i++) {
            // Выбор произвольной x,y позиции
            x=rand() % CUBE_SIZE;
            y=rand() % CUBE_SIZE;
            // но не один и тот же дважды подряд
            if (y != last_y && x != last_x) {
                // Если объёмный элемент x,y внизу,
                if (getvoxel(y, x, 0)) {
                    // то он посылается наверх
                    sendvoxel_axis(y, x, 0, axis, delay);
                } else {
                    // Если объёмный элемент вверху, то он посылается наверх
                    sendvoxel_axis(y, x, (CUBE_SIZE - 1), axis, delay);
                }
                cube_draw(1, wait); // Задержка в течении которой непрерывно отрисовывается куб
                if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
                // Запоминание последнего перемещения
                last_y=y;
                last_x=x;
            }
        }
    }
}

// Движущиеся вперёд буквы
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
            shift(3, 1);
        }
        delay_ms(100); // задержка между буквами
    }
}

// Движущаяся точка (со следом или без) или змейка перемещающаяся по кубу
void effect_boingboing(int iterations, char delay, unsigned char mode, unsigned char drawmode) {
    int x, y, z; // Current coordinates for the point                  // mode=0x02;                 // не работает
    int dx, dy, dz; // Direction of movement                     // delay=4; mode=0x01; drawmode=0x01;  // точка, перемещающаяся по кубу
    int lol, i; // lol?                                      // delay=2; mode=0x01; drawmode=0x02;  // точка, перемещающаяся по кубу остявляющая след и заполняющая весь куб
    unsigned char crash_x, crash_y, crash_z; // delay=3; mode=0x01; drawmode=0x03;  // змейка, перемещающаяся по кубу
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

        // show one voxel at time (один светодиод)
        if (drawmode==0x01)
        {
            setvoxel(z, y, x);
            cube_draw(1, delay);
            clrvoxel(z, y, x);
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
        }
        // flip the voxel in question (переделано в движущийся светодиод, оставляющий след)
        if (drawmode==0x02)
        {
            // flpvoxel(z,y,x); // исходный вариант
            setvoxel(z, y, x);
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
        }
        // draw a snake (змейка)
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
            for (i=0; i<CUBE_SIZE; i++) {
                clrvoxel(snake[i][2], snake[i][1], snake[i][0]);
            }
        }
        iterations--;
    }
}

// Включение или выключение всех светодиодов в случайном порядке
void effect_random_filler(int delay, char state) {/*
	char z,y,x;
	int loop;

  if (state==1)
    fill_cube(0x00);
  	else
      fill_cube(0xff);

	for (loop=0; loop<(CUBE_BYTES-3); loop++)  // CUBE_BYTES-3
	{
		z=rand()%CUBE_SIZE;
		y=rand()%CUBE_SIZE;
    x=rand()%CUBE_SIZE;
		if ((state==0 && getvoxel(z,y,x)==0x01) || (state==1 && getvoxel(z,y,x)==0x00))
			altervoxel(z,y,x,state);
  cube_draw(1,delay);
	}*/
}

// Вспомогоательный эффект для эффекта effect_z_updown_move и effect_axis_updown_randsuspend
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

// Вспомогоательный эффект для эффекта effect_z_updown
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

// Подсветка плоскости, случайное распределение её по кубу
// и перетеккание на противоположную сторону
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
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
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
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
    }

    // Suspend the positions in top-bottom for a while
    cube_draw(1, sleep * 2);
}

// Возникновение скопления светодиодов в центре куба
// и перетекание их снизу вверх и сверху в низ
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
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
    }
    for (i=0; i<iterations; i++) {
        for (move=0; move<CUBE_SIZE; move++) {
            effect_z_updown_move(positions, destinations, 1); // (positions, destinations, AXIS_Z)
            cube_draw(1, 4); //  cube_draw(1,delay);
        }
        cube_draw(1, 100); //  cube_draw(1,delay*CUBE_SIZE/2);
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
        for (y=0; y<(CUBE_SIZE * CUBE_SIZE / 2); y++) {
            destinations[rand() % (CUBE_SIZE * CUBE_SIZE)]=rand() % CUBE_SIZE;
        }
    }
}

// Подстветка всего этажа или стенки и перетекание на противоположную сторону
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
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
        draw_positions_axis(axis, cubepos, 0);
        // LED_PORT ^= LED_RED;
    }
    cube_draw(1, 80); // пауза в крайних положениях
}

// Поочерёдная подсветка всех светодиодов,
// затем последовательное их погашение
void effect_loadbar(int delay) {
    int z, y, x;
    fill_cube(0x00);

    for (z=(CUBE_SIZE - 1); z>=0; z--) {
        for (y=(CUBE_SIZE - 1); y>=0; y--) {
            for (x=(CUBE_SIZE - 1); x>=0; x--) {
                setvoxel(z, y, x);
                cube_draw(1, delay);
                if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
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
                if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
            }
            // cube_draw(1,delay);  // delay_ms(delay);
        }
    }
}

// Включение n (int voxels) светодиодов в случайном порядке
// (вспомогоательный эффект для эффекта effect_random_sparkle)
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

// Произвольное сверкание светодиодов
// Включение 1 случайного светодиода, 2 случайных светодиодов...
// и так далее до (CUBE_SIZE*x) и обратно до одного светодиода
void effect_random_sparkle(void) {
    int i;

    if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
    
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

// Заполнение всего куба по диагонали
void effect_telcstairs(int invert, int delay, int val) { //  0           5       0xff
    /*	int x,y,z;               invert=1, delay=5, val=0xff;
      fill_cube(0x00);

            for(z=0; z<CUBE_SIZE; z++)
            {
         for(y=0; y<=z; y++)
                {
              for(x=0; x<=z; x++)
                  {
               cube[z][y][x]=val;
            }
          }
          cube_draw(1,delay);
        }

        if(invert)
        {
            for(x=CUBE_SIZE*2; x>=0; x--)
            {
            for(y=0, z=x; y<=z; y++, x--)
            {
                if(x<CUBE_SIZE)
                {
                    cube[z][y][x]=val;
                }
            }
            cube_draw(1,delay);
            x=z;
            }
    }
      else
        {
            for(x=0; x<CUBE_SIZE*2; x++)
            {
        for(y=0, z=x; y<=z; y++, x--)
        {
            if(x<CUBE_SIZE)    //    x<CUBE_SIZE && y<CUBE_SIZE
            {
                cube[z][y][x]=val;  // cube[x][y]=val;
            }
        }
        cube_draw(1,delay);
          x=z;
            }
        }
     */
}


// Сжимающийся червь
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
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
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

// Вспомогательный эффект для effect_pathspiral и effect_path_bitmap
void effect_pathmove(unsigned char *path, int length) {
    int i, z;
    unsigned char state;

    // Движение справо налево
    for (i=(length - 1); i>=1; i--) {
        for (z=0; z<CUBE_SIZE; z++) {
            state=getvoxel(z, CUBE_SIZE - 1 - (path[(i - 1)] >> 4 & 0x0f), CUBE_SIZE - 1 - ((path[(i - 1)])&0x0f));
            altervoxel(z, CUBE_SIZE - 1 - (path[i] >> 4 & 0x0f), CUBE_SIZE - 1 - ((path[i])&0x0f), state);
        }
    }
    for (z=0; z<CUBE_SIZE; z++)
        clrvoxel(z, CUBE_SIZE - 1 - (path[0] >> 4 & 0x0f), CUBE_SIZE - 1 - ((path[0])&0x0f));
    /*
      // Движение слева направо (для картинок нормально, а буквы перевёрнутые)
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

// Вращающаяся спираль
void effect_pathspiral(int iterations, int delay) {
    int i;
    unsigned char path[28];
    fill_cube(0x00);
    font_getpath(1, path, 28);

    for (i=0; i<iterations; i++) {
        setvoxel(i % 9, (CUBE_SIZE - 1), CUBE_SIZE / 2); // setvoxel(4,0,i%8);
        cube_draw(1, delay);
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
        effect_pathmove(path, 28);
    }
}

// Движущаяся волнистая линия по периметру куба
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
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
    }
}

// Вращаяющийся по периметру куба текст
void effect_path_text(int delay, char *str) {
    int z, i, ii;
    unsigned char path[28]; // 28 - периметр куба 8x8x8
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
        }
        effect_pathmove(path, 28);
        cube_draw(1, delay);
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
    }
    for (i=0; i<28; i++) {
        effect_pathmove(path, 28);
        cube_draw(1, delay);
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
    }
}

// Изображения для эффекта effect_path_bitmap
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

// Вспомогательный эффект для effect_path_bitmap
unsigned char font_getbitmappixel(char bitmap, char x, char y) {
    char tmp=bitmaps[bitmap][x];
    return (tmp >> y) & 0x01;
}

// Вращаяющиеся по периметру куба изображения
void effect_path_bitmap(int delay, char bitmap, int iterations) {
    int z, i, ii;
    unsigned char path[28]; // 28 - периметр куба 8x8x8
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
            effect_pathmove(path, 28);
        }
        for (ii=0; ii<20; ii++) {
            cube_draw(1, delay);
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
            effect_pathmove(path, 28);
        }
    }
    for (ii=0; ii<10; ii++) {
        cube_draw(1, delay);
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
        effect_pathmove(path, 28);
    }
}

// Вращающиеся изображения bitmaps[]
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
            fill_cube(0x00);
        }

    }
}

// Вращающаяся квадратная спираль
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
            iter++;
        }
        loc=0;
    }
}

// Вращающаяся двойная квадратная спираль
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
            if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
            iter++;
        }
        loc=0;
    }
}

// Кручение линий
void effect_twister(int iterations, int delay) {/*
	int iter,twist;
  fill_cube(0x00);

  for (iter=0; iter<iterations; iter++)
	{
		for (twist=0; twist<CUBE_SIZE; twist++)
		{
      line (twist,0,0,7-twist,0,7);
			line (0,twist,0,0,7-twist,7);
      // line (0,0,twist,(CUBE_SIZE-1),0,(CUBE_SIZE-1)-twist);
			// line (0,twist,0,(CUBE_SIZE-1),(CUBE_SIZE-1)-twist,0);
			cube_draw(1,delay);
			shift(2,1);  // AXIS_Y
			shift(1,0);  // AXIS_Z
			shift(3,0);  // AXIS_X
		}
	}*/
}

// Фейерверк
void effect_fireworks(char iterations) {
    char i, f, z;
    char n=80; // количество осколков
    char origin_x, origin_y, origin_z;
    //float slowrate, gravity;  // замедление и гравитация

    // Particles and their position, x,y,z and their movement, dx, dy, dz
    char particles[80][6]; // float particles[n][6]

    fill_cube(0x00);

    for (i=0; i<iterations; i++) {
        if (fl_button) { fl_button=0; return; }  // прерывание по нажатию кнопки
        // Определение координат стартовой точки выстрела
        origin_x=rand() % 4 + 2; // (0..3)+2=2..5
        origin_y=rand() % 4 + 2; // (0..3)+2=2..5
        // Высота подъёма выстрела
        origin_z=1;

        // Выстреливание частицы со случайного местоположения на нижней плоскости
        for (z=(CUBE_SIZE - 1); z>=origin_z; z--) {
            setvoxel(z, origin_y, origin_x);
            cube_draw(1, (5 - TAN((z + 0, 1) / 20)*10)); // отрисовка подъёма снаряда с замедлением
            fill_cube(0x00);
        }

        // Заполнение массива взрыва
        for (f=0; f<n; f++) {
            // Позиция
            particles[f][0]=origin_x;
            particles[f][1]=origin_y;
            particles[f][2]=origin_z;

            // Перемещение
            particles[f][3]=1 - rand() % 3; // dx  //=1-(float)rand_x/100;
            particles[f][4]=1 - rand() % 3; // dy
            particles[f][5]=1 + rand() % 2; // dz
        }

        // Взрыв
        for (z=0; z<5; z++) // количество взрывов
        {
            // slowrate=1+tan((z+0.1)/20)*10;
            // gravity =tan((z+0.1)/20)/2;
            for (f=0; f<n; f++) // количество осколков
            {
                particles[f][0] += particles[f][3];
                particles[f][1] += particles[f][4];
                particles[f][2] += particles[f][5] - rand() % 2;
                if (particles[f][2]==0) {
                    particles[f][2]++;
                }
                setvoxel(particles[f][2], particles[f][1], particles[f][0]);
            }
            cube_draw(1, 4); // отрисовка взрыва
            fill_cube(0x00);
        }
        delay_ms(600); // пауза между взрывами
    }
}

// Математические функии (в разработке)
/*void init_LUT(unsigned char LUT[65])
{
    unsigned char i;
    float sin_of,sine;
    for (i=0; i<65; i++)
    {
        sin_of=i*PI/64; // Just need half a sin wave
        sine=sin(sin_of);
        // Use 181.0 as this squared is <32767, so we can multiply two sin or cos without overflowing an int.
        LUT[i]=sine*181.0;
    }
}

int totty_sin(unsigned char LUT[65],int sin_of)
{
    unsigned char inv=0;
    if (sin_of<0)
    {
        sin_of=-sin_of;
        inv=1;
    }
    sin_of&=0x7f;  // 127
    if (sin_of>64)
    {
        sin_of-=64;
        inv=1-inv;
    }
    if (inv)
        return -LUT[sin_of];
    else
        return LUT[sin_of];
}

int totty_cos(unsigned char LUT[65],int cos_of)
{
    unsigned char inv=0;
    cos_of+=32;    // Simply rotate by 90 degrees for COS
    cos_of&=0x7f;  // 127
    if (cos_of>64)
    {
        cos_of-=64;
        inv=1;
    }
    if (inv)
        return -LUT[cos_of];
    else
        return LUT[cos_of];
}*/

// Волны в кубе
void effect_int_sidewaves(int iterations, int delay) {/*
	unsigned char LUT[65];
	int i;
	int origin_x, origin_y, distance, height;
	int x_dist,x_dist2,y_dist;
	int x,y,x_vox;

	init_LUT(LUT);

	for (i=0; i<iterations; i++)
	{	// To provide some finer control over the integer calcs the x and y
		// parameters are scaled up by a factor of 15.  This is primarily to
		// keep the sum of their squares within the scale of an integer.
		// 120^2 + 120^2=28800
		// If we scaled by 16 we would overflow at the very extremes,
		// e.g 128^2+128^2=32768.  The largest int is 32767.
		//
		// Because origin_x/y is a sin/cos pair centred at 60, the actual
		// highest distance in this effect would be at:
		// x=8,y=8, origin_x=102, origin_y=102
		//=approximate sum of 17400.
		//
		// It is probably safer to work at a scale of 15 to allow simple changes
		// to the maths to be made without risking an overflow.
		origin_x=(totty_sin(LUT,i/2)+180)/3;  // Approximately 0 to 120
		origin_y=(totty_cos(LUT,i/2)+180)/3;
		fill_cube(0x00);
		for (x=8; x<120; x+=15)// 8 steps from 8 to 113
		{
			// Everything in here happens 8 times per cycle
			x_dist=abs(x-origin_x);
			x_dist2=x_dist*x_dist;  // square of difference
			x_vox=x/15;  // Unscale x
			for (y=8; y<120; y+=15)
			{
				// Everything in here happens 64 times per cycle
				y_dist=abs(y-origin_y);
				if (x_dist||y_dist)  // Either x OR y non-zero
				{
					// Calculate sum of squares of linear distances
					// We use a 1st order Newton approximation:
					// sqrt=(N/guess+guess)/2
					distance=(x_dist2+y_dist*y_dist);
					height=(x_dist+y_dist)/2;  // Approximate quotient
					// We divide by 30.  1st approx would be /2
					// but we have a factor of 15 included in our scale calcs
					distance=(distance/height+height)/3; // 1st approx at sqrt
				}
				else
					distance=0;  // x and y=origin_x and origin_y
				height=(totty_sin(LUT,distance+i)+180)/52;
				setvoxel(height,y/15,x_vox);
			}
		}
		cube_draw(1,delay); // delay_ms(delay);
	}*/
}

// Вспомогательный эффект для эффекта effect_filip_filop
void effect_plane_flip(unsigned char LUT[], unsigned char start, unsigned char end, int delay) {/*
	unsigned char p1,p2;  // point across and down on flip
	unsigned char x,y,z;
	unsigned char i;      // rotational position
	unsigned char i1;     // linear position
	unsigned char dir=0;  // 0-2  0=sidetrap 1=door 2=trap
	unsigned char rev=0;  // 0 for forward 1 for reverse
	unsigned char inv=0;  // bit 0=Y bit 1=Z
	// Sort out dir, rev and inv for each start/end combo.
	// There are 24, but with some neat combinations of
	// tests we can simplify so that only a max of 3 tests
	// are required for each type!
	if (start<2)// 0 or 1.  buh
	{
		if (end<4)
		{
			dir=0;
			if (end==3) inv=0x01;
		}
		else
		{
			dir=2;
			if (end==5) inv=0x01;
		}
		if (start==1) inv|=0x02;
	} else if (start<4)  // 2 or 3. Buh
	{
		if (end<2)  // going to 0 or 1
		{
			rev=1;
			dir=0;
			if (start==3) inv=0x01;
			if (end==1) inv|=0x02;
		}
		else  // going to 4 or 5
		{
			dir=1;  // door moves
			if(start+end==7)  // 3 to 4 or 2 to 5
			{
				if (start==3)
					inv=0x02;
				else
					inv=0x01;
			}
			else//2 to 4 or 3 to 5
			{
				if (start==3)
					inv=0x03;
			}
		}
	}
	else //start is 4 or 5
	{
		if (end<2)// 0 or 1
		{
			dir=2;
			rev=1;// reverse trapdoor.  Yeah!
			if (start+end==5)//4 to 1 or 5 to 0
			{
				if (start==4)//4 to 1
					inv=0x02;
				else
					inv=0x01;
			}
			else  // 4 to 0 or 5 to 1
				if (start==5)
					inv=0x03;
		}
		else  // end is 2 or 3
		{
			rev=1;  // all reverse
			dir=1;  // all door
			if (start+end==7)  // 4 to 3 or 5 to 2
			{
				if (start==4) // 4 to 3
					inv=0x02;
				else
					inv=0x01;
			}
			else
			{
				if (start==5)  // 5 to 3
					inv=0x03;
			}
		}
	}
	// Do the actual plane drawing
	for(i=0; i<7; i++)
	{
		if (rev)  // Reverse movement goes cos-sin
		{
			p2=totty_sin(LUT,i*4);  // angle 0-45 degrees
			p1=totty_cos(LUT,i*4);
		}
		else
		{
			p1=totty_sin(LUT,i*4);  // angle 0-45 degrees
			p2=totty_cos(LUT,i*4);
		}
		fill_cube(0x00);
		for (i1=0; i1<8; i1++)
		{
			z=p1*i1/168;
			if (inv&0x02) z=7-z;  // invert in Z axis
			y=p2*i1/168;
			if (inv&0x01) y=7-y;  // invert in Y axis
			for(x=0; x<8; x++)
			{
				if(!dir)//dir=0
				{
					setvoxel(x,y,z);
				}
				else if(dir==1)
				{
					setvoxel(y,z,x);
				}
				else//dir=2
				{
					setvoxel(y,x,z);
				}
			}
		}
		cube_draw(1,delay); // delay_ms(delay);
	}*/
}

//
void effect_filip_filop(int iterations, int delay) {/*
	// LUT_START // Macro
	unsigned char now_plane=0;  // start at top
	unsigned char next_plane;
  unsigned char LUT[];
  // delay=FF_DELAY;
	int i;  // iteration counter
	for (i=iterations; i; i--)
	{
		next_plane=rand()%6; // 0-5
		// Check that not the same, and that:
		// 0/1 2/3 4/5 pairs do not exist.
		if ((next_plane&0x06)==(now_plane&0x06))
			next_plane=(next_plane+3)%6;
		effect_plane_flip(LUT,now_plane,next_plane,delay);
		now_plane=next_plane;
	}*/
}

//
void effect_cubix(int iterations, unsigned char cubies) {/* // cube array:
	// Stupid name to prevent confusion with cube[][]!
	// 0=pos
	// 1=dir (0 stopped 1,2 or 4 move in X, Y or Z)
	// 2=inc or dec in direction (2=inc, 0=dec)
	// 3=countdown to new movement
	// 4=x, 5=y, 6=z
	unsigned char qubes[][7];  // qubes[cubies][7];
	int ii; // iteration counter
	unsigned char i,j,diridx,newdir;
	unsigned char runstate=2;
	// Initialise qubes array
	for (i=0; i<cubies; i++)
	{
		qubes[i][0]=i;  // position=i
		qubes[i][1]=0;  // static
		qubes[i][3]=rand()&0x0f;  // 0-15
		// Hack in the X,Y and Z positions
		qubes[i][4]=4*(i&0x01);
		qubes[i][5]=2*(i&0x02);
		qubes[i][6]=(i&0x04);
	}
	// Main loop
	ii=iterations;
	while(runstate)
	{
		fill_cube(0x00);
		for (i=0; i<cubies; i++)
		{
			// Use a pointer to simplify array indexing
			// qube[0..7]=qubes[i][0..7]
			unsigned char *qube=&qubes[i][0];
			if (qube[1]) // moving
			{
				diridx=3+qube[1];  // 4,5 or 7
				if (diridx==7) diridx=6;
				qube[diridx]+=qube[2]-1;
				if ((qube[diridx]==0)||(qube[diridx]==4))
				{
					// XOR old pos and dir to get new pos.
					qube[0]=qube[0]^qube[1];
					qube[1]=0;  // Stop moving!
					qube[3]=rand()&0x0f;  // countdown to next move 0-15
					if(runstate==1)
						if (qube[0]<5)
							qube[3]*=1;  // Make lower qubes move very slowly to finish was value 4
				}
			}
			else  // not moving
			{
				if (qube[3])  // counting down
					qube[3]--;
				else // ready to move
				{
					newdir=(1<<(rand()%3));  // 1,2 or 4
					diridx=qube[0]^newdir;
					for (j=0; j<cubies; j++)  // check newdir is safe to move to
					{
						if ((diridx==qubes[j][0])||(diridx==(qubes[j][0]^qubes[j][1])))
						{
							newdir=0;
							qube[3]=5;
						}
					}

					if (newdir)
					{
						diridx=3+newdir;
						if (diridx==7) diridx=6;
						if (qube[diridx])// should be 4 or 0
							qube[2]=0; // dec if at 4
						else
						{
							qube[2]=2; // inc if at 0
							if(runstate==1)// Try to make qubes go home
								if ((diridx>4)&&(qube[0]<4))
									newdir=0;//Don't allow qubes on bottom row to move up or back
						}
					}
					qube[1]=newdir;
				}
			}
			// if (i&0x01)  // odd number
			box_filled(qube[6],qube[5],qube[4],qube[6]+3,qube[5]+3,qube[4]+3);
			// else
			// box_wireframe(qube[4],qube[5],qube[6],qube[4]+3,qube[5]+3,qube[6]+3);
		} // i loop
  cube_draw(1,80);  // delay_ms(800);
		if(runstate==2)   // If normal running
		{
			if(!(--ii))     // decrement iteration and check for zero
				runstate=1;   // If zero go to homing
		}
		else              // runstate at 1
		{
			diridx=0;
			for(j=0; j<cubies; j++)
				if (qubes[j][0]+1>cubies) diridx=1;  // Any cube not at home
			if (!diridx)
				runstate=0;
		}
 }*/ // Main loop
}

// Пульсации
void effect_int_ripples(int iterations, int delay) {/*
	// 16 values for square root of a^2+b^2.  index a*4+b=10*sqrt
	// This gives the distance to 3.5,3.5 from the point
	unsigned char sqrt_LUT[]={49,43,38,35,43,35,29,26,38,29,21,16,35,25,16,7};
	// LUT_START // Macro from new tottymath.  Commented and replaced with full code
	unsigned char LUT[65];
	// init_LUT(LUT);
	int i;
	unsigned char x,y,height,distance;
	for (i=0; i<iterations*CUBE_SIZE; i+=CUBE_SIZE)
	{
		fill_cube(0x00);
		for (x=0; x<CUBE_SIZE; x++)
			for(y=0; y<CUBE_SIZE; y++)
			{
				// x+y*4 gives no. from 0-15 for sqrt_LUT
				distance=sqrt_LUT[x+y*CUBE_SIZE];  // distance is 0-50 roughly
				// height is sin of distance + iteration*4
				// height=4+totty_sin(LUT,distance+i)/52;
				height=(196+totty_sin(LUT,distance+i))/49;
				// Use 4-way mirroring to save on calculations
				setvoxel(x,y,height);
				setvoxel((CUBE_SIZE-1)-x,y,height);
				setvoxel(x,(CUBE_SIZE-1)-y,height);
				setvoxel((CUBE_SIZE-1)-x,(CUBE_SIZE-1)-y,height);
			}
  cube_draw(1,delay);
	}*/
}

// Пульсации по бокам кубика
void effect_side_ripples(int iterations, int delay) {/*
	// 16 values for square root of a^2+b^2.  index a*4+b=10*sqrt
	// This gives the distance to 3.5,3.5 from the point
	unsigned char sqrt_LUT[]={49,43,38,35,43,35,29,26,38,29,21,16,35,25,16,7};
	// LUT_START // Macro from new tottymath.  Commented and replaced with full code
	unsigned char LUT[65];
	// init_LUT(LUT);
	int i;
	unsigned char x,y,height,distance;
	for (i=0; i<iterations*CUBE_SIZE; i+=CUBE_SIZE)
	{
		fill_cube(0x00);
		for (x=0; x<CUBE_SIZE; x++)
			for(y=0; y<CUBE_SIZE; y++)
			{
				// x+y*4 gives no. from 0-15 for sqrt_LUT
				distance=sqrt_LUT[x+y*CUBE_SIZE];  // distance is 0-50 roughly
				// height is sin of distance + iteration*4
				// height=4+totty_sin(LUT,distance+i)/52;
				height=(196+totty_sin(LUT,distance+i))/49;
				// Use 4-way mirroring to save on calculations
				setvoxel(x,height,y);
				setvoxel((CUBE_SIZE-1)-x,height,y);
				setvoxel(x,height,(CUBE_SIZE-1)-y);
				setvoxel((CUBE_SIZE-1)-x,height,(CUBE_SIZE-1)-y);
				setvoxel(x,(CUBE_SIZE-1)-height,y);
				setvoxel((CUBE_SIZE-1)-x,(CUBE_SIZE-1)-height,y);
				setvoxel(x,(CUBE_SIZE-1)-height,(CUBE_SIZE-1)-y);
				setvoxel((CUBE_SIZE-1)-x,(CUBE_SIZE-1)-height,(CUBE_SIZE-1)-y);
			}
    cube_draw(1,delay);
	}*/
}

// Пульсации по стенкам кубика
void effect_mirror_ripples(int iterations, int delay) {/*
	// 16 values for square root of a^2+b^2.  index a*4+b=10*sqrt
	// This gives the distance to 3.5,3.5 from the point
	unsigned char sqrt_LUT[]={49,43,38,35,43,35,29,26,38,29,21,16,35,25,16,7};
	//LUT_START // Macro from new tottymath.  Commented and replaced with full code
	unsigned char LUT[65];
	//init_LUT(LUT);
	int i;
	unsigned char x,y,height,distance;
	for (i=0; i<iterations*CUBE_SIZE; i+=CUBE_SIZE)
	{
		fill_cube(0x00);
		for (x=0; x<CUBE_SIZE; x++)
			for(y=0; y<CUBE_SIZE; y++)
			{
				// x+y*4 gives no. from 0-15 for sqrt_LUT
				distance=sqrt_LUT[x+y*CUBE_SIZE];  // distance is 0-50 roughly
				// height is sin of distance + iteration*4
				// height=4+totty_sin(LUT,distance+i)/52;
				height=(196+totty_sin(LUT,distance+i))/49;
				// Use 4-way mirroring to save on calculations
				setvoxel(height,y,x);
				setvoxel(height,y,(CUBE_SIZE-1)-x);
				setvoxel(height,(CUBE_SIZE-1)-y,x);
				setvoxel(height,(CUBE_SIZE-1)-y,(CUBE_SIZE-1)-x);
				setvoxel((CUBE_SIZE-1)-height,y,x);
				setvoxel((CUBE_SIZE-1)-height,y,(CUBE_SIZE-1)-x);
				setvoxel((CUBE_SIZE-1)-height,(CUBE_SIZE-1)-y,x);
				setvoxel((CUBE_SIZE-1)-height,(CUBE_SIZE-1)-y,(CUBE_SIZE-1)-x);
			}
		cube_draw(1,delay);
	}*/
}

// Пульсации по всем бокам кубика
void effect_quad_ripples(int iterations, int delay) {/*
	// 16 values for square root of a^2+b^2.  index a*4+b=10*sqrt
	// This gives the distance to 3.5,3.5 from the point
	unsigned char sqrt_LUT[]={49,43,38,35,43,35,29,26,38,29,21,16,35,25,16,7};
	// LUT_START // Macro from new tottymath.  Commented and replaced with full code
	unsigned char LUT[65];
	// init_LUT(LUT);
	int i;
	unsigned char x,y,height,distance;
	for (i=0; i<iterations*CUBE_SIZE; i+=CUBE_SIZE)
	{
		fill_cube(0x00);
		for (x=0; x<CUBE_SIZE; x++)
			for(y=0; y<CUBE_SIZE; y++)
			{
				// x+y*4 gives no. from 0-15 for sqrt_LUT
				distance=sqrt_LUT[x+y*CUBE_SIZE];// distance is 0-50 roughly
				// height is sin of distance + iteration*4
				//height=4+totty_sin(LUT,distance+i)/52;
				height=(196+totty_sin(LUT,distance+i))/49;
				// Use 4-way mirroring to save on calculations
				setvoxel(x,y,height);
				setvoxel((CUBE_SIZE-1)-x,y,height);
				setvoxel(x,(CUBE_SIZE-1)-y,height);
				setvoxel((CUBE_SIZE-1)-x,(CUBE_SIZE-1)-y,height);
				setvoxel(x,y,(CUBE_SIZE-1)-height);
				setvoxel((CUBE_SIZE-1)-x,y,(CUBE_SIZE-1)-height);
				setvoxel(x,(CUBE_SIZE-1)-y,(CUBE_SIZE-1)-height);
				setvoxel((CUBE_SIZE-1)-x,(CUBE_SIZE-1)-y,(CUBE_SIZE-1)-height);
				setvoxel(x,height,y);
				setvoxel((CUBE_SIZE-1)-x,height,y);
				setvoxel(x,height,(CUBE_SIZE-1)-y);
				setvoxel((CUBE_SIZE-1)-x,height,(CUBE_SIZE-1)-y);
				setvoxel(x,(CUBE_SIZE-1)-height,y);
				setvoxel((CUBE_SIZE-1)-x,(CUBE_SIZE-1)-height,y);
				setvoxel(x,(CUBE_SIZE-1)-height,(CUBE_SIZE-1)-y);
				setvoxel((CUBE_SIZE-1)-x,(CUBE_SIZE-1)-height,(CUBE_SIZE-1)-y);
			}
  cube_draw(1,delay);
	}*/
}

// Дождь с ускорением и сдвигом капель
void effect_rain2(int iterations, int delay, int hold, int speed) {/*
	int i;
	int p;		// Position of the raindrop on Z
	int z;		// cube layer
	int y;		// byte
  int x;
	fill_cube(0x00);

	for (i=0; i<iterations; i++)
	{
		// Start by setting a random pixel on layer 3.
		setvoxel((CUBE_SIZE-1),rand()%CUBE_SIZE,rand()%CUBE_SIZE);  // setvoxel(rand()%4,rand()%4,3);
		cube_draw(1,hold);  // delay_ms(hold);

		// The raindrop has to step down one layer 4 times
		// in order to travel from the top, and exit out the bottom.
		for (p=1; p<CUBE_SIZE; p++)
		{
			// Shift all the layers one position down,
			for (z=(CUBE_SIZE-1); z<=0; z--)
			{
				for (y=0; y<CUBE_SIZE; y++)
				{
        for (x=0; x<CUBE_SIZE; x++)
				{
        	cube[z][y][x]=cube[z+1][y][x];
				}
				}
			}

			// and a blank image on the top layer.
      //fill_layer(7,0x00);
      cube[(CUBE_SIZE-1)][0][0]=0x00;
			cube[(CUBE_SIZE-1)][1][0]=0x00;
			cube[(CUBE_SIZE-1)][2][0]=0x00;
			cube[(CUBE_SIZE-1)][3][0]=0x00;

			// Accelerate the raindrop as it falls.
			// (speed/p) will decrease as p increases.
			cube_draw(1,speed+(speed/p));  // delay_ms(speed+(speed/p));
		}

		fill_cube(0x00);
		cube_draw(1,delay);  // delay_ms(delay);
	}*/
}

// Игра Жизнь (Game of Life)
/*unsigned char gol_count_neighbors (int z, int y, int x)
{
    int ix, iy, iz; // offset 1 in each direction in each dimension
    int nx, ny, nz; // neighbours address.
    unsigned char neigh=0; // number of alive neighbours.

    for (iz=-1; iz<2; iz++)
    {
        for (iy=-1; iy<2; iy++)
        {
            for (ix=-1; ix<2; ix++)
            {
                // Your not your own neighbour, exclude 0,0,0, offset.
                if ( !(ix==0 && iy==0 && iz==0) )
                {
                    if (GOL_WRAP==0x01)
                    {
                        nx=(x+ix)%GOL_X;
                        ny=(y+iy)%GOL_Y;
                        nz=(z+iz)%GOL_Z;
                    } else
                    {
                        nx=x+ix;
                        ny=y+iy;
                        nz=z+iz;
                    }
                    if ( getvoxel(nz, ny, nx) )
                        neigh++;
                }
            }
        }
    }
    return neigh;
}

void gol_nextgen (void)
{
    int i, x,y,z;
    unsigned char neigh;
  for (i=0; i<CUBE_SIZE; i++)
  {
    tmpfill_layer(i,0x00); // tmpfill_(0x00);
  }

    for (z=0; z<GOL_Z; z++)
    {
        for (y=0; y<GOL_Y; y++)
        {
            for (x=0; x<GOL_X; x++)
            {
                neigh=gol_count_neighbors(z, y, x);

                // Current voxel is alive.
                if (getvoxel(z,y,x)==0x01)
                {
                    if (neigh<=GOL_TERMINATE_LONELY)
                    {
                        tmpclrvoxel(z,y,x);
                    } else if(neigh>=GOL_TERMINATE_CROWDED)
                    {
                        tmpclrvoxel(z,y,x);
                    } else
                    {
                        tmpsetvoxel(z,y,x);
                    }
                // Current voxel is dead.
                } else
                {
                    if (neigh>=GOL_CREATE_MIN && neigh<=GOL_CREATE_MAX)
                        tmpsetvoxel(z,y,x);
                }
            }
        }
    }
}

int gol_count_changes (void)
{
    int x,y,z;
    int i=0;

    for (z=0; z<GOL_Z; z++)
    {
    for (y=0; y<GOL_Y; y++)
    {
        for (x=0; x<GOL_X; x++)
        {
            if (buffer[z][x][y] != cube[z][x][y])
                i++;
        }
    }
  }
    return i;
}

void gol_play (int iterations, int delay)
{
    int i;
    for (i=0; i<iterations; i++)
    {
        gol_nextgen();
        if (gol_count_changes()==0)
            return;

        buffer2cube();
        cube_draw(1,delay);
    }
}*/

// Shows an animation of a spinning plane
void effect_spinning_plane(int direction, int iterations, int delay) {/*
	int i;
	int x,y,z;
  const unsigned char spinning_line[6][2]=
  {	{0x84,0x21},
  	{0x0c,0x30},
  	{0x03,0xc0},
  	{0x12,0x48},
  	{0x22,0x44},
  	{0x44,0x22}};

  fill_cube(0x00);

	for (i=0; i<iterations; i++)
	{
		// Loop cube levels
	for (z=0; z<4; z++)
	{
		cube[0][0][z]=spinning_line[(i)%6][0] >> 4;
		cube[1][1][z]=spinning_line[(i)%6][0];
		cube[2][2][z]=spinning_line[(i)%6][1] >> 4;
		cube[3][3][z]=spinning_line[(i)%6][1];
	}
		cube_draw(1,delay);
	}*/
}

/*
animation::animation(){
    reset();

    //fill the sine array in memory
    for(uint16_t i=0; i<91; i++){
        sinA[i]=sin(i*PI/180);
    }
}

//SINGLE PLANE BOUNCING
uint8_t animation::bouncePlane(cube &c, uint8_t axis){
    slow++;
    if(slow<200){
        return 0;
    }
    slow=0;

    //the X animation looks the same but actually clears everything in its path
    //rather than clear everything at the start, it makes a simple but cool
    //transition between some animations
    if(axis != 'X'){
        c.clearVoxels();
    }

    switch(axis){
        case 'X':
            for(uint8_t z=0; z<Zd; z++){
                for(uint8_t y=0; y<Yd; y++){
                    for(uint8_t x=0; x<Xd; x++){
                        if(x==pos - dir){
                            c.clearVoxel(x, y, z);
                        }
                        c.setVoxel(pos, y, z);
                    }
                }
            }
            break;
        case 'Y':
            for(uint8_t x=0; x<Xd; x++){
                for(uint8_t z=0; z<Zd; z++){
                    c.setVoxel(x, pos, z);
                }
            }
            break;
        case 'Z':
            for(uint8_t x=0; x<Xd; x++){
                for(uint8_t y=0; y<Yd; y++){
                    c.setVoxel(x, y, pos);
                }
            }
            break;
    }

    //bounce the pos variable between 0 and 7
    bouncePos();

    return 1;
}

//RANDOM LIGHTS
uint8_t animation::randomExpand(cube &c){
    uint16_t count=0;
    uint8_t rX, rY, rZ;

    slow++;
    if(slow<20){
        return 0;
    }
    slow=0;

    randomSeed(analogRead(0));
    rX=rand()%8+0;
    rY=rand()%8+0;
    rZ=rand()%8+0;

    //find an empty voxel
    while(c.getVoxel(rX, rY, rZ)==1 && dir==1){
        rX=random(0, 8);
        rY=random(0, 8);
        rZ=random(0, 8);
        count++;

        if(count>200){
            dir *= -1;
        }
    }
    count=0;

    //find a full voxel
    while(c.getVoxel(rX, rY, rZ)==0 && dir==-1){
        rX=random(0, 8);
        rY=random(0, 8);
        rZ=random(0, 8);
        count++;

        if(count>200){
            dir *= -1;
        }
    }

    //fill or clear the voxel found
    if(dir==1){
        c.setVoxel(rX, rY, rZ);
    }else{
        c.clearVoxel(rX, rY, rZ);
    }

    return 1;
}

//HELIX ANIMATION
uint8_t animation::helix(cube &c){
    float X=0;
    float Y=0;
    float Z=0;

    slow++;
    if(slow<100){
        return 0;
    }
    slow=0;

    c.clearVoxels();

    //use my fancy pants sine function
    for(uint8_t i=0; i<3; i++){
        for(uint8_t z=0; z<4; z++){
            Z=z*52;
            X=get_sinA(Z + phase + 18*i);
            Y=get_sinA(Z + phase + 90 + 18*i);
            X=(X+1)*4;
            Y=(Y+1)*4;
            c.setVoxel((uint8_t)X, (uint8_t)Y, z);
            c.setVoxel((uint8_t)(8-X), (uint8_t)(8-Y), z+4);
        }
    }

    //increment the phase
    phase+=18;

    if(phase>360){
        phase -= 360;
    }

    return 1;
}

//RAIN FUNCTION
uint8_t animation::rain(cube &c){
    uint16_t count=0;
    uint8_t rX=0;
    uint8_t rY=0;
    uint8_t rZ=0;

    slow++;
    if(slow<200){
        return 0;
    }
    slow=0;

    randomSeed(analogRead(0));

    //shift the rain down
    for(int x=0; x<Xd; x++){
        for(int y=0; y<Yd; y++){
            for(int z=0; z<Zd-1; z++){
                if(z==0 && c.getVoxel(x, y, z)){
                    c.clearVoxel(x, y, z);
                }

                if(c.getVoxel(x, y, z+1)){
                    c.clearVoxel(x, y, z+1);
                    c.setVoxel(x, y, z);
                }
            }
        }
    }

    //create some raindrops
    rX=random(0,8);
    rY=random(0,8);
    while(c.getVoxel(rX, rY, 7) != 0 && count<100){
        rX=random(0,8);
        rY=random(0,8);
        count++;
    }
    c.setVoxel(rX, rY, 7);

    return 1;
}

//CUBE PULSE
uint8_t animation::cubePulse(cube &c){
    uint8_t cubeSize=pos+1;
    uint8_t test=0;
    uint8_t xCen=0;
    uint8_t yCen=0;
    uint8_t zCen=0;

    slow++;
    if(slow<150){
        return 0;
    }
    slow=0;

    //build the cube with consitional statements,
    //I feel like I could do this more elegantly but it works
    for(uint8_t x=0; x<Xd; x++){
        for(uint8_t y=0; y <Yd; y++){
            for(uint8_t z=0; z<Zd; z++){
                test=0;
                xCen=absA(2*(x-centre));
                yCen=absA(2*(y-centre));
                zCen=absA(2*(z-centre));

                if(xCen==cubeSize && yCen==cubeSize && zCen==cubeSize){
                    c.setVoxel(x, y, z);
                    test=1;
                }
                if(xCen==cubeSize && yCen==cubeSize && zCen<cubeSize){
                    c.setVoxel(x, y, z);
                    test=1;
                }
                if(xCen==cubeSize && zCen==cubeSize && yCen<cubeSize){
                    c.setVoxel(x, y, z);
                    test=1;
                }
                if(zCen==cubeSize && yCen==cubeSize && xCen<cubeSize){
                    c.setVoxel(x, y, z);
                    test=1;
                }

                if(!test && c.getVoxel(x, y, z)){
                    c.clearVoxel(x, y, z);
                }
            }
        }
    }

    pos += dir*2;

    if(pos>6 || pos<0){
        dir *= -1;
        pos += dir*2;
    }

    return 1;
}

//do the bounce
void animation::bouncePos(void){
    pos += dir;

    if(pos>7 || pos<0){
        dir*=-1;
        pos += dir;
    }
}

//reset all variables
void animation::reset(void){
    dir=1;
    pos=0;
    slow=0;
    phase=0;
}

//my awesome function to get data out of my sine array
//I use the fact that sine functions are periodic to reduce the size of the array
float animation::get_sinA(int16_t deg){
    while(deg>360){
        deg -= 360;
    }

    if(deg<=90){
        return sinA[deg];
    }else if(deg>90 && deg<=180){
        return sinA[90-(deg-90)];
    }else if(deg>180 && deg<=270){
        return -sinA[deg-180];
    }else if(deg>270 && deg<=360){
        return -sinA[90-(deg-270)];
    }
    return sinA[deg];
}

//absolute value function
int16_t animation::absA(int16_t in){
   int16_t out=in;
   if(out<0){
        out=-out;
   }
   return out;
}

*/












/* ------------------------------------------------------------------------ */
/*                          КОНЕЦ ФУНКЦИЙ ЭФФЕКТОВ                          */
/* ------------------------------------------------------------------------ */




/* ------------------------------------------------------------------------ */
/*                              ЗАПУСК ЭФФЕКТОВ                             */
/* ------------------------------------------------------------------------ */

void launch_effect(char effect) {
    char i, bitmap;

    // Выбор эффекта для запуска
    switch (effect) {
        case 0: // настроен
            // effect_loadbar(1);  // срывается, когда доходит до верхнего слоя
            break;
        case 1: // настроен
            for (i=1; i<4; i++)
                effect_planboing(i,3);
            delay_msec(100);
            break;
        case 2:
            // настроен
            effect_rain(500);
            break;
        case 3:
            // настроен
            effect_box_shrink_grow();
            effect_box_woopwoop(4, 0);
            effect_box_woopwoop(4, 1);
            effect_box_woopwoop(4, 0);
            effect_box_woopwoop(4, 1);
            effect_box_woopwoop(4, 0);
            effect_box_woopwoop(4, 1);
            break;
        case 4:
            // настроен
            // effect_sendvoxels_rand_axis(50,1,1,5);  // сверху-вниз: срывается // (int iterations, char axis, int delay, int wait)
            effect_sendvoxels_rand_axis(50, 2, 1, 5);  // слева-направо
            effect_sendvoxels_rand_axis(50, 3, 1, 5);  // вперёд-назад
            // effect_sendplane_rand_z(0,1,5);         // сверху-вниз: срывается
            // effect_sendplane_rand_z(7,1,5);         // снизу-вверх: срывается
            break;
        case 5:
            // настроен
            effect_stringfly("HELLO WORLD");
            break;
        case 6:
            // настроен
            effect_boingboing(200, 4, 0x01, 0x01); // точка, перемещающаяся по кубу
            effect_boingboing(500, 2, 0x01, 0x02); // точка, перемещающаяся по кубу остявляющая след и заполняющая весь куб
            effect_boingboing(300, 3, 0x01, 0x03); // змейка, перемещающаяся по кубу
            break;
        case 7:
            // настроен
            // effect_boxside_randsend_parallel(1,0,1,1);  // закомментированные срываются, возможно не хватает тока
            // effect_boxside_randsend_parallel(1,1,1,1);  // (char axis, int origin, int delay, int mode)
            // effect_boxside_randsend_parallel(1,0,1,2);
            // effect_boxside_randsend_parallel(1,1,1,2);
            // effect_boxside_randsend_parallel(2,0,1,1);
            // effect_boxside_randsend_parallel(2,1,1,1);
            effect_boxside_randsend_parallel(2, 0, 1, 2);
            effect_boxside_randsend_parallel(2, 1, 1, 2);
            // effect_boxside_randsend_parallel(3,0,1,1);
            // effect_boxside_randsend_parallel(3,1,1,1);
            effect_boxside_randsend_parallel(3, 0, 1, 2);
            effect_boxside_randsend_parallel(3, 1, 1, 2);
            break;
        case 8:
            // настроен
            effect_random_sparkle();
            break;
        case 9:
            // настроен
            effect_wormsqueeze(2, 0, 200, 4);
            effect_wormsqueeze(3, 1, 200, 4);
            // effect_wormsqueeze(1,0,100,3);  // на остальных не движется
            // effect_wormsqueeze(1,1,100,3);
            // effect_wormsqueeze(2,1,100,3);
            // effect_wormsqueeze(3,0,100,3);
            break;
        case 10:
            effect_rand_patharound(1000, 3);
            break;
        case 11:
            // настроен
            effect_smileyspin(2, 3, 0); // стартовая картинка
            for (i=7; i<9; i++) {
                effect_smileyspin(5, 3, i);
            } // (int count, int delay, char bitmap)
            break;
        case 12:
            // настроен
            effect_path_text(6, "HELLO WORLD");
            break;
        case 13:
            // настроен
            effect_axis_updown_randsuspend(3, 2, 50, 0); // (char axis, int delay, int sleep, int invert)
            effect_axis_updown_randsuspend(3, 2, 50, 1);
            effect_axis_updown_randsuspend(2, 2, 50, 0);
            effect_axis_updown_randsuspend(2, 2, 50, 1);
            // effect_axis_updown_randsuspend(1,2,50,0); effect_axis_updown_randsuspend(1,2,50,1);  // срывается
            break;
        case 14:
            // настроен
            effect_z_updown(10);
            break;
        case 15:
            // настроен
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
            // Эффекты, которые плохо работают
            /*//------------------------------
            effect_blinky2();
            //------------------------------
            effect_twister(8,2);
            //------------------------------
            effect_filip_filop(50,7);
            //------------------------------
            effect_int_ripples(200,5);
                effect_mirror_ripples(200,5);
                  effect_side_ripples(200,5);
                  effect_quad_ripples(200,5);
            //-----------------------------
            effect_int_sidewaves(500,1);
            //------------------------------
            effect_cubix(100,1);
                  effect_cubix(100,2);
                  effect_cubix(100,3);
            //------------------------------
            effect_spinning_plane(1,50,5);
            //------------------------------
                  effect_telcstairs(0,5,0xff);
                  effect_telcstairs(0,5,0x00);
                  effect_telcstairs(1,5,0xff);
                  effect_telcstairs(1,5,0xff);
            // ------------------------------
            // не все зажигаются и потухают, тормозит
            effect_random_filler(1,1);
                  effect_random_filler(1,0);
                  effect_random_filler(1,1);
                  effect_random_filler(1,0);
            // ------------------------------
            // Создание случайных стартовых точек для Game of Life effect
            fill_cube(0x00);
            for (i=0; i<20; i++)
                  { setvoxel(rand()%4,rand()%4,rand()%4); }
                  gol_play(20,2);
            // ------------------------------
            effect_rain2(50,2,10,10);
            // ------------------------------ */
            break;
        case 17:
            // настроен
            effect_fireworks(3);
            break;
        case 18:
            // настроен
            effect_pathspiral(64, 4);
            break;
        case 19:
            // настроен
            for (bitmap=0; bitmap<14; bitmap++) {
                effect_path_bitmap(5, bitmap, 1); // (int delay, char bitmap, int iterations)
            }
            break;
        case 20: // настроен
            effect_squarespiral1(200, 4);
            effect_squarespiral2(200, 4);
            break;
        case 21:
            // тестовые эффекты
            // fill_cube(0x00);
            setvoxel(7, 7, 7);
            cube_draw(1, 100);
            // fill_cube(0xff);
            // setvoxel(0,0,0);
            // cube_draw(1,100);
            // fill_cube(0x00);
            // line (0,0,0,7,0,7);
            // line (0,0,0,7,7,0);
            // cube_draw(1,100);
            // delay_ms(1000);
            // cube_draw(1,50);
            break;
        default: // В случае, если выбран номер не существующего эффекта
            effect_planboing(1, 3);
            //effect_stringfly("ERROR");
            break;
    }

    /*
            for (uint8_t i=0; i<EFFECTS; i++) {
                switch (prog[i])
                {
                    default:
                        break;
                }
            }
     */


}



/* ------------------------------------------------------------------------ */
/*                          КОНЕЦ ЗАПУСКА ЭФФЕКТОВ                          */
/* ------------------------------------------------------------------------ */





/* ------------------------------------------------------------------------ */
/*                          ОСНОВНОЙ ЦИКЛ ПРОГРАММЫ                         */
/* ------------------------------------------------------------------------ */

// PIC выполянет эту функцию после запуска

void main(void) {
    INTCON  = 0b11110000; // разрешение прерываний
    INTCON2 = 0b10000101; // разрешение прерываний
    RCON    = 0b00000000; // отключение приоритеной системы прерываний
    T0CON   = 0b11000101; // управление таймером TMR0
    CMCON   = 0b00000111;
    PR2     = 0b00000000;
    PIE1    = 0b00000000;
    ADCON0  = 0b00000000; // биты управления модулем АЦП (отключен)
    ADCON1  = 0b00001110; // биты управления модулем АЦП (AN7-AN1 - digital, AN0 - analog)
    LVDCON  = 0b00000000; // биты управления модулем LVD (отключен)
    PORTA   = 0b00000000; // 8-разрядный порт ввода вывода PORTA
    TRISA   = 0b01000000; // биты управления направлением PORTA (RA6 - вход от кварцевого генератора)
    PORTB   = 0b00000000; // 8-разрядный порт ввода вывода PORTB
    TRISB   = 0b00000001; // биты управления направлением PORTB (RB0 - вход от кнопки)
    PORTC   = 0b00000000; // 8-разрядный порт ввода вывода PORTC
    TRISC   = 0b00000000; // биты управления направлением PORTC
    // 76543210   // нумерация портов   // отключить компараторы!

    while (1) {
        // Запуск случайного эффекта при первом запуске куба
        if (fl_start==0) {                                                        // ((fl_start==0) & EEprom_Read(0)==0xff )
            fl_start=1;
            // EEprom_Write(0,0);  // хранение флага первого запуска куба
            effect=rand()%EFFECTS_TOTAL;
            // EEprom_Write(1,effect);  // хранение номера выполняемого эффекта
        }
/*
        // Запись номера эффекта после нажатия кнопки
        if (fl_reset==1) {
            fl_reset=0;
            EEprom_Write(1,effect);  // хранение номера выполняемого эффекта
            asm reset
        }
*/
        launch_effect(effect);                                                    // EEprom_Read(1)
    }
}

/* ------------------------------------------------------------------------ */
/*                     КОНЕЦ ОСНОВНОГО ЦИКЛА ПРОГРАММЫ                      */
/* ------------------------------------------------------------------------ */



    /*
    // Очистка пользовательской RAM
    // Банки [00 .. 07] ( 8 x 256=2048 байтов )
    asm {
      LFSR     FSR0, 0x000
      MOVLW    0x08
      CLRF     POSTINC0, 0
      CPFSEQ   FSR0H, 0
      BRA      $ - 2 }
    */

    /*
      // Если эффект запущен по кнопке, то необходимо
      // обнуление куба для исключения наложения эффектов
      if (fl_button==1)
      {
        fill_cube(0x00);
        cube_draw(1,1);
        fl_button=0;
      }
    */
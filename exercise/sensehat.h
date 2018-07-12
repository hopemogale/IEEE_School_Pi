// sensehat.h - header file for sensehat.c library

#define KEY_UP    103
#define KEY_LEFT  105
#define KEY_RIGHT 106
#define KEY_DOWN  108
#define KEY_ENTER  28

void led_clear();
int read_joystick();
void set_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);

// sensehat.c - simple library to access sense hat joystick and LED

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <poll.h>
#include <dirent.h>
#include <string.h>

#include <linux/input.h>
#include <linux/fb.h>

static int is_event_device(const struct dirent *dir)
{
	return strncmp("event", dir->d_name,
		       strlen("event")-1) == 0;
}
static int is_framebuffer_device(const struct dirent *dir)
{
	return strncmp("fb", dir->d_name,
		       strlen("fb")-1) == 0;
}

static int open_evdev(const char *dev_name)
{
	struct dirent **namelist;
	int i, ndev;
	int fd = -1;

	ndev = scandir("/dev/input", &namelist, is_event_device, versionsort);
	if (ndev <= 0)
		return ndev;

	for (i = 0; i < ndev; i++)
	{
		char fname[64];
		char name[256];

		snprintf(fname, sizeof(fname), "/dev/input/%s", namelist[i]->d_name);
		fd = open(fname, O_RDONLY);
		if (fd < 0)
			continue;
		ioctl(fd, EVIOCGNAME(sizeof(name)), name);
		if (strcmp(dev_name, name) == 0)
			break;
		close(fd);
	}

	for (i = 0; i < ndev; i++)
		free(namelist[i]);

	return fd;
}

static int open_fbdev(const char *dev_name)
{
   struct dirent **namelist;
   int i, ndev;
   int fd = -1;
   struct fb_fix_screeninfo fix_info;
   
   ndev = scandir("/dev", &namelist, is_framebuffer_device, versionsort);
   if (ndev <= 0)
      return ndev;
   
   for (i = 0; i < ndev; i++) {
      char fname[64];
      char name[256];
      
      snprintf(fname, sizeof(fname), "%s/%s", "/dev", namelist[i]->d_name);
      fd = open(fname, O_RDWR);
      if (fd < 0)
         continue;
      ioctl(fd, FBIOGET_FSCREENINFO, &fix_info);
      if (strcmp(dev_name, fix_info.id) == 0)
         break;
      close(fd);
      fd = -1;
      }
   for (i = 0; i < ndev; i++)
      free(namelist[i]);
   
   return fd;
}


int read_joystick()
{
   static int evfd = 0;
   struct input_event ev[64];
   int i, rd;
   struct pollfd evpoll = {
      .events = POLLIN,
   };

   if (!evfd) {
      evfd = open_evdev("Raspberry Pi Sense HAT Joystick");
      if (evfd < 0) {
         fprintf(stderr, "Event device not found.\n");
         exit(1);
      }
   }

   evpoll.fd = evfd;
   if (poll(&evpoll, 1, 0) == 0)
      return 0;

   rd = read(evfd, ev, sizeof(struct input_event) * 64);
   if (rd < (int) sizeof(struct input_event)) {
      fprintf(stderr, "expected %d bytes, got %d\n",
             (int) sizeof(struct input_event), rd);
      return 0;
   }
   for (i = 0; i < rd / sizeof(struct input_event); i++) {
      if (ev->type != EV_KEY)
         continue;
      if (ev->value != 1)
         continue;
      return ev->code;
   }
   
   return 0;
}

struct fb_t {
   uint16_t pixel[8][8];
};


static struct fb_t *_fb = NULL;

void led_init()
{
   int fbfd;
   
   if (!_fb) {
      fbfd = open_fbdev("RPi-Sense FB");
      if (fbfd <= 0) {
         printf("Error: cannot open framebuffer device.\n");
         exit(1);
      }
      _fb = mmap(0, 128, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
      if (!_fb) {
         printf("Failed to mmap.\n");
         exit(1);
      }
      memset(_fb, 0, 128);
   }
}

void led_clear()
{
   if (!_fb)
      led_init();
   memset(_fb, 0, 128);
}

void set_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
   unsigned short br, bg, bb, value;
   
   if (!_fb)
      led_init();
   
   br = r >> 3;
   bg = g >> 3;
   bb = b >> 3;
   value = (br << 11) | (bg << 6) | (bb << 1);
   _fb->pixel[x][y] = value;
}

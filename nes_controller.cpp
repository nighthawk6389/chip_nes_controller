#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>

int HIGH = 1;
int LOW = 0;

int MICRO=1e3;

struct Buttons {
   int a;
   int b;
   int sel;
   int start;
   int up;
   int down;
   int left;
   int right;
   
   int code(){
   	return  (a << 1) | (b << 2) | (sel << 3) |
   		(start << 4) | (up << 5) | (down << 6) |
   		(left << 7) | (right << 8);
   }
};

#define WCHK(cond_expr) do { \
  if (!(cond_expr)) { \
    fprintf(stderr, "%s:%d, Warning, expected '%s' to be true (%s)\n", \
      __FILE__, __LINE__, #cond_expr, strerror(errno)); \
  }  \
} while (0)

#define ECHK(cond_expr) do { \
  if (!(cond_expr)) { \
    fprintf(stderr, "%s:%d, Error, expected '%s' to be true (%s)\n", \
      __FILE__, __LINE__, #cond_expr, strerror(errno)); \
    abort(); \
  }  \
} while (0)

int openGPIO(int pin, bool in){

    int PIN_STR_LEN = 4;
    char pinStr[PIN_STR_LEN];
    snprintf(pinStr, PIN_STR_LEN, "%d", pin);
    
    int DIR_STR_LEN = 50;
    char dirStr[DIR_STR_LEN] = {};
    snprintf(dirStr, DIR_STR_LEN, "/sys/class/gpio/gpio%d/direction", pin);
    
    int VAL_STR_LEN = 50;
    char valStr[VAL_STR_LEN] = {};
    snprintf(valStr, VAL_STR_LEN, "/sys/class/gpio/gpio%d/value", pin);
    
    int INOUT_STR_LEN = 4;

    int export_fd = open("/sys/class/gpio/export", O_WRONLY);  
    ECHK(export_fd != -1);
    int wlen = write(export_fd, pinStr, PIN_STR_LEN);  
    WCHK(wlen == PIN_STR_LEN);
    close(export_fd);

    int dir_fd = open(dirStr, O_RDWR);  
    ECHK(dir_fd != -1);
    wlen = write(dir_fd, (in ? "in": "out"), INOUT_STR_LEN);  
    ECHK(wlen == INOUT_STR_LEN);
    close(dir_fd);
	
    int in_val_fd = open(valStr, O_RDWR);  
    ECHK(in_val_fd != -1);
    
    std::cout << "Opened FD=" << in_val_fd << std::endl;
    
    return in_val_fd;
}

int writeGPIO(int fd, int val){

    int wlen;
    if(val >= 1){
    	wlen = write(fd, "1", 2); 
    } else {
        wlen = write(fd, "0", 2);
    }
    std::cout << "wrote " << wlen << " size" << std::endl;
    ECHK(wlen == 2);
    return 0;
}

int readGPIO(int fd){

    char readbuf[3] = {};
    int st = lseek(fd, 0, SEEK_SET);  
    ECHK(st == 0);
    int rlen = read(fd, readbuf, sizeof(readbuf));  
    ECHK(rlen > 0);
    
    return atoi(readbuf);
}

inline void wait(int pulseFD){
    writeGPIO(pulseFD,HIGH);
    usleep(6);
    writeGPIO(pulseFD,LOW);
    usleep(6);
}

int checkController(int latchFD, int pulseFD, int dataFD, Buttons *bt){

  writeGPIO(latchFD, HIGH);
  usleep(12);
  
  writeGPIO(latchFD, LOW);
  usleep(6);
  
  bt->a 	= readGPIO(dataFD);
  wait(pulseFD);
  bt->b 	= readGPIO(dataFD);
  wait(pulseFD);
  bt->sel 	= readGPIO(dataFD);
  wait(pulseFD); 
  bt->start 	= readGPIO(dataFD);
  wait(pulseFD);
  bt->up 	= readGPIO(dataFD);
  wait(pulseFD);
  bt->down 	= readGPIO(dataFD);
  wait(pulseFD);
  bt->left 	= readGPIO(dataFD);
  wait(pulseFD);
  bt->right 	= readGPIO(dataFD);
  
  /*
  std::cout << "a=" << a << std::endl;
  std::cout << "b=" << b << std::endl;
  std::cout << "sel=" << sel << std::endl;
  std::cout << "start=" << start << std::endl;
  std::cout << "up=" << up << std::endl;
  std::cout << "down=" << down << std::endl;
  std::cout << "left=" << left << std::endl;
  std::cout << "right=" << right << std::endl;
  */
  
  return 0;
  
}

int main(int argc, char **argv){
    int latchPin = 132;
    int pulsePin = 134;
    int dataPin = 136;
    
    int latchFD = openGPIO(latchPin, false);
    int pulseFD = openGPIO(pulsePin, false);
    int dataFD = openGPIO(dataPin, true);
    
    while(true){
        Buttons bt = {};
        checkController(latchFD, pulseFD, dataFD, &bt);
        std::cout << bt.code() << std::endl;
        sleep(1);
    }
}

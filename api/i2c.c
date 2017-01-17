#include <sys/ioctl.h> 
#include <fcntl.h> 
#include <linux/i2c-dev.h> 
#include <linux/i2c.h> 

#define CHIP "/dev/i2c-1" 
#define CHIP_ADDR 0x28 
int main() 
{      
   printf("hello, this is i2c tester\n");         
   int fd = open(CHIP, O_RDWR);      
   if (fd < 0) 
   {         
      printf("open "CHIP"failed\n");          
      goto exit;      
   }      
   if (ioctl(fd, I2C_SLAVE_FORCE, CHIP_ADDR) < 0) 
   {         /* 设置芯片地址 */         
      printf("oictl:set slave address failed\n");        
      goto close;      
   }

   unsigned char rddata;      
   unsigned char rdaddr[2] = {0, 0};     /* 将要读取的数据在芯片中的偏移量 */      
   unsigned char wrbuf[3] = {0, 0, 0x3c};     /* 要写的数据，头两字节为偏移量 */         
 /*
   printf("input a char you want to write to E2PROM\n");     
   wrbuf[2] = getchar();     
   printf("write return:%d, write data:%x\n", write(fd, wrbuf, 3), wrbuf[2]);     
   sleep(1);     */
   printf("write address return: %d\n",write(fd, rdaddr, 2));     /* 读取之前首先设置读取的偏移量 */     
   printf("read data return:%d\n", read(fd, &rddata, 1));     
   printf("rddata: %c\n", rddata); 
close:      
  close(fd); 
exit:      
  return 0; 
}
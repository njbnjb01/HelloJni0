#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/timeb.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <jni.h>
//#include "cutils/properties.h"

#include <android/log.h>
#define LOG_TAG "RFID"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static int openfd = -1;
static int irdev = -1;
static int boardrate = 0;
static int Start_Scan = 0;
static int End_Scan = 1;
static int Scan_Time = 100000;
#define NO_Baudrate -2
#define OPEN_DEV_ERROR -3
#define CONFIG_DEV_ERROR -4
#define SETTIMEOUT_ERROR -5
#define NOT_FOR_READ -6
#define PREPARE_READ_ERROR -7
#define READ_DATA_ERROR -8
#define WRITE_ERROR -9
#define NO_DEVICE_OPEN -10
#define NOT_FOR_WRITE -11
#define PREPARE_WRITE_ERROR -12
#define TOO_LONG_DATA -13
#define B28800 28800

#define FALSE -1
#define TRUE  0
#define GPIO_READ  0
#define GPIO_WRITE 1


#define  IR_IOC_MAGIC   'm'
#define   IO_CTL_TRIG_H   _IO(IR_IOC_MAGIC,150)
#define   IO_CTL_TRIG_L  _IO(IR_IOC_MAGIC, 151)

static speed_t getBaudrate(jint baudrate)
 {
    	switch(baudrate) {
    	case 0: return B0;
    	case 50: return B50;
    	case 75: return B75;
    	case 110: return B110;
    	case 134: return B134;
    	case 150: return B150;
    	case 200: return B200;
    	case 300: return B300;
    	case 600: return B600;
    	case 1200: return B1200;
    	case 1800: return B1800;
    	case 2400: return B2400;
    	case 4800: return B4800;
   	case 9600: return B9600;
    	case 19200: return B19200;
    	case 38400: return B38400;
	case 28800: return B28800;
    	case 57600: return B57600;
    	case 115200: return B115200;
    	case 230400: return B230400;
    	case 460800: return B460800;
    	case 500000: return B500000;
    	case 576000: return B576000;
    	case 921600: return B921600;
    	case 1000000: return B1000000;
    	case 1152000: return B1152000;
    	case 1500000: return B1500000;
    	case 2000000: return B2000000;
    	case 2500000: return B2500000;
    	case 3000000: return B3000000;
    	case 3500000: return B3500000;
    	case 4000000: return B4000000;
   	 default: return -1;
  }
 }


int set_Parity(int fd, int databits, int stopbits, int parity)
{
	struct termios options;
	
	if ( tcgetattr( fd , &options) != 0){
		LOGE("Setup Serial 1");
		return(CONFIG_DEV_ERROR);
	}

	LOGE("#############set_Parity, fd=%d", fd);

	options.c_cflag &= ~CSIZE;
	options.c_cflag &= ~CRTSCTS;
	
	switch(databits)
	{
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			LOGE("Unsupported data size\n"); 
			return (CONFIG_DEV_ERROR);
	}

	switch (parity)
	{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;	/* clear partity enable */
			options.c_iflag &= ~INPCK;	/* Enable partity checking */
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);	/* Enable odd checking */
			options.c_iflag |= INPCK;		/* Disabled partity checking */
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;	/* Enable parity */
			options.c_cflag &= ~PARODD;	/* Chage to odd checking */
			options.c_iflag |= INPCK;	/* Disnable parity checking */
			break;
		case 's':
		case 'S':
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			LOGE("Unsupported parity\n");
			return (CONFIG_DEV_ERROR);

	}
	/* Set stop bit */
	switch(stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			LOGE("Unsupported stop bits\n");
			return (CONFIG_DEV_ERROR);
	}
	/* Set input parity option */
	if( parity != 'n')
		options.c_iflag |= INPCK;
	tcflush(fd,TCIFLUSH);
	
	options.c_lflag &= ~(ICANON | IEXTEN | ISIG | ECHO);
	options.c_oflag &= ~OPOST;
	options.c_iflag &= ~(ICRNL | INPCK | ISTRIP | IXON | BRKINT );

	options.c_cc[VMIN] = 1;
	options.c_cc[VTIME] = 0;
	options.c_cflag |= (CLOCAL | CREAD);
//	options.c_cc[VTIME] = 150;
//	options.c_cc[VMIN] = 0;
//	options.c_lflag &=  ~(ICANON | ECHO | ISIG);
//	options.c_oflag &=  ~OPOST;
	if(tcsetattr(fd,TCSANOW, &options) != 0){
		LOGE("set Serial 3");
		return (CONFIG_DEV_ERROR);
	}
	return (0);
}


int set_speed(int fd, unsigned int speed)
{	
	int i;
	int status;
	unsigned int baud;
	struct termios Opt;

	LOGE("set_speed, fd = %d", fd);
  	tcgetattr(fd, &Opt);

	switch(speed)
	{
		case 300:
			baud = B300;
			break;
		case 600:
			baud = B600;
			break;
		case 1200:
			baud = B1200;
			break;
		case 2400:
			baud = B2400;
			break;
		case 4800:
			baud = B4800;
			break;
		case 9600:
			baud = B9600;
			break;
		case 19200:
			baud = B19200;
			break;
		case 38400:
			baud = B38400;
			break;
		case 57600:
			baud = B57600;
			break;
		case 115200:
			baud = B115200;
			break;
		default:
			LOGE("##can't find this baud.");
			return -1;
	}
	tcflush(fd, TCIOFLUSH);
	cfsetispeed(&Opt, baud);
	cfsetospeed(&Opt, baud);
	status = tcsetattr(fd, TCSANOW, &Opt);

	if (status != 0){
		LOGE("tcsetattr fd");
		return -1;
	}
	tcflush(fd, TCIOFLUSH);
	return 0;
}


static int read_seting_echo(int fd, char *buf, int timeout, int *len){
	int nfds, rc = -1,i, num = 0; 
	fd_set readfds; 
	struct timeval time_out; 
	unsigned int sec, usec, set = 0;
	char *p = buf;

	while(1){
		FD_ZERO(&readfds);
     		FD_SET(fd,&readfds);

		//sec = timeout /1000;
		//usec = (timeout %1000)*1000;
		time_out.tv_sec = 0;
		time_out.tv_usec = 500000;
		rc = select(fd+1, &readfds, NULL, NULL, &time_out);
			
		if(rc < 0){
			LOGE("#####select error\n");
			break;
		}
		else if(rc > 0){
			//LOGE("#####read ok\n");
			if(FD_ISSET(fd, &readfds)) {
				num = read(fd, p, 100);
				//for(i = 0; i < num; i++)
					//LOGE("%x",p[i]);
				*len += num;
				p = p + num;
				num = 0;	
			}
			
		}
		
		else if(rc == 0){
			LOGE("########no data read\n");
			break;
		}
	}
	
	//printf("receive data is %s", buf);
	return rc;
}

jobject 
Java_com_example_hellojni_HelloJni_open
 (JNIEnv* env,jobject thiz,jstring path, jint baudrate) {
    	 	jobject mFileDescriptor;
		  jboolean iscopy;
		  int fd1, num, length, ret, i;
		  char cmd[512] = {0};
		  char buff[1024] = {0};
	
      		/* Opening device */
        	const char *path_utf = (char *)(*env)->GetStringUTFChars(env, path, &iscopy);
         	LOGI("Opening serial port %s", path_utf);
         	openfd = open(path_utf, O_RDWR | O_NOCTTY);
         	LOGI("open() fd = %d", openfd);
        	 (*env)->ReleaseStringUTFChars(env, path, path_utf);
        	if (openfd == -1)
        	{
            		 /* Throw an exception */
             		LOGE("Cannot open port");
            		/* TODO: throw an exception */
            		return OPEN_DEV_ERROR;
         	}
      		
	  	set_speed(openfd,19200);
	  	if (set_Parity(openfd,8,1,'N')== CONFIG_DEV_ERROR)
  		{
    			LOGE("#######Set Parity Error\n");
    			return CONFIG_DEV_ERROR;
  		}

		/////////////////////////////////////1.region china
		memset(buff,0,1024);
		cmd[0] = 0xbb;
		cmd[1] = 0x00;
		cmd[2] = 0x07;
		cmd[3] = 0x00;
		cmd[4] = 0x01;
		cmd[5] = 0x32;
		cmd[6] = 0x7e;
		num = 0;
		num = write(openfd, cmd, 7);
		if(num <= 0)
			LOGE("####write error\n");
		length = 0;
		ret = read_seting_echo(openfd, buff, 0, &length);
		for(i = 0; i < length; i++)
			LOGE("%x", buff[i]);
		
		//if(buff[0] != 0xbb)
			//return -1;

		/////////////////////////////////////2.channel 10
		memset(buff,0,1024);
		cmd[0] = 0xbb;
		cmd[1] = 0x00;
		cmd[2] = 0xab;
		cmd[3] = 0x00;
		cmd[4] = 0x02;
		cmd[5] = 0x0a;
		cmd[6] = 0x0a;
		cmd[7] = 0x7e;
		num = 0;
		num = write(openfd, cmd, 8);
		if(num <= 0)
			LOGE("####write error\n");
		length = 0;
		ret = read_seting_echo(openfd, buff, 0, &length);
		for(i = 0; i < length; i++)
			LOGE("%x", buff[i]);
		//if(buff[0] != 0xbb)
			//return -1;
		
		/////////////////////////////////////3.set tx power
		memset(buff,0,1024);
		cmd[0] = 0xbb;
		cmd[1] = 0x00;
		cmd[2] = 0xb6;
		cmd[3] = 0x00;
		cmd[4] = 0x02;
		cmd[5] = 0xfd;
		cmd[6] = 0xa8;
		cmd[7] = 0x7e;
		num = 0;
		num = write(openfd, cmd, 8);
		if(num <= 0)
			LOGE("####write error\n");
		length = 0;
		ret = read_seting_echo(openfd, buff, 0, &length);
		for(i = 0; i < length; i++)
			LOGE("%x", buff[i]);
		//if(buff[0] != 0xbb)
			//return -1;
  
       	 /* Create a corresponding file descriptor */
             jclass cFileDescriptor = (*env)->FindClass(env, "java/io/FileDescriptor");
             jmethodID iFileDescriptor = (*env)->GetMethodID(env, cFileDescriptor, "<init>", "()V");
             jfieldID descriptorID = (*env)->GetFieldID(env, cFileDescriptor, "descriptor", "I");
             mFileDescriptor = (*env)->NewObject(env, cFileDescriptor, iFileDescriptor);
             (*env)->SetIntField(env, mFileDescriptor, descriptorID, (jint)openfd);
         
        	return mFileDescriptor;//返回打开的文件描述符
 }

void
Java_com_example_hellojni_HelloJni_close
   (JNIEnv* env, jobject thiz) {
       	jclass SerialPortClass = (*env)->GetObjectClass(env, thiz);
      		jclass FileDescriptorClass = (*env)->FindClass(env, "java/io/FileDescriptor");
      		jfieldID mFdID = (*env)->GetFieldID(env, SerialPortClass, "mFd", "Ljava/io/FileDescriptor;");
       	jfieldID descriptorID = (*env)->GetFieldID(env, FileDescriptorClass, "descriptor", "I");
     		jobject mFd = (*env)->GetObjectField(env, thiz, mFdID);
       	jint descriptor = (*env)->GetIntField(env, mFd, descriptorID);

		//Start_Scan = 0;
		//while(End_Scan == 1);
      		LOGI("close(fd = %d)", descriptor);
       	close(descriptor);
 }


void 
Java_com_example_hellojni_HelloJni_tcflush
   (JNIEnv* env, jobject thiz, jint type) {
	 	jclass SerialPortClass = (*env)->GetObjectClass(env, thiz);
       	jclass FileDescriptorClass = (*env)->FindClass(env, "java/io/FileDescriptor");

      		jfieldID mFdID = (*env)->GetFieldID(env, SerialPortClass, "mFd", "Ljava/io/FileDescriptor;");
       	jfieldID descriptorID = (*env)->GetFieldID(env, FileDescriptorClass, "descriptor", "I");

      		jobject mFd = (*env)->GetObjectField(env, thiz, mFdID);
       	jint descriptor = (*env)->GetIntField(env, mFd, descriptorID);

	   	LOGI("################tcflush(fd = %d)", descriptor);
		tcflush(descriptor, type);
}





jobject
Java_com_example_hellojni_HelloJni_readRfidTag(JNIEnv* env, jobject thiz,  jint timeout, jint scan_interval){
	char buf[1024] = {0};
	char cmd[512] = {0};
	int num, flage;
	int nfds, rc = -1,i, strlen, ret; 

	LOGE("##############readRfidTag");
	if(openfd  < 0){
		LOGE("#####fd < 0");
		return NO_DEVICE_OPEN;
	}

	 //set_speed(openfd,19200);
	 //set_Parity(openfd, 8, 1, 'N');

	jclass clsstring = (*env)->FindClass(env, "com/example/hellojni/HelloJni");
	jmethodID mid = (*env)->GetMethodID(env, clsstring, "report_rfidtag", "([B)V");
	
	cmd[0] = 0xbb;
	cmd[1] = 0x00;
	cmd[2] = 0x22;
	cmd[3] = 0x00;
	cmd[4] = 0x00;
	cmd[5] = 0x7e;

	tcflush(openfd, TCIOFLUSH);
	num = 0;
	num = write(openfd, cmd, 6);
	if(num <= 0)
		LOGE("####write error\n");
	LOGE("###write end num = %d\n", num);
	strlen = 0;
	memset(buf,0,1024);
	read_seting_echo(openfd, buf, timeout, &strlen);
		for(i = 0; i < strlen; i++)
			LOGE("%x", buf[i]);
	jbyte *jbuf = (jbyte *)buf;
	jbyteArray carr = (*env)->NewByteArray(env,strlen);
	(*env)->SetByteArrayRegion(env, carr, 0, strlen, jbuf);
	(*env)->CallVoidMethod(env, thiz,  mid, carr);
}


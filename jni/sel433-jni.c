#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <jni.h>

#include <android/log.h>
#define LOG_TAG "SerialPort"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static int openfd = -1;
static int boardrate = 0;
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


struct serial_struct {
	int	type;
	int	line;
	unsigned int	port;
	int	irq;
	int	flags;
	int	xmit_fifo_size;
	int	custom_divisor;
	int	baud_base;
	unsigned short	close_delay;
	char	io_type;
	char	reserved_char[1];
	int	hub6;
	unsigned short	closing_wait; /* time to wait before closing */
	unsigned short	closing_wait2; /* no longer used... */
	unsigned char	*iomem_base;
	unsigned short	iomem_reg_shift;
	unsigned int	port_high;
	unsigned long	iomap_base;	/* cookie passed into ioremap */
};
#define ASYNCB_SPD_HI		 4 /* Use 56000 instead of 38400 bps */
#define ASYNCB_SPD_VHI		 5 /* Use 115200 instead of 38400 bps */
#define ASYNC_SPD_VHI		(1U << ASYNCB_SPD_VHI)
#define ASYNC_SPD_HI		(1U << ASYNCB_SPD_HI)
#define ASYNC_SPD_CUST		(ASYNC_SPD_HI|ASYNC_SPD_VHI)

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
		perror("Setup Serial 1");
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

int serial_set_speci_baud(int myfd,int baud)
{
    struct serial_struct ss;
	struct serial_struct ss_set;
    struct termios    options;

    cfsetispeed(&options,B38400);
    cfsetospeed(&options,B38400);

    tcflush(myfd,TCIFLUSH);/*handle unrecevie char*/
    tcsetattr(myfd,TCSANOW,&options);

    if((ioctl(myfd,TIOCGSERIAL,&ss))<0){
        LOGE("BAUD: error to get the serial_struct info:%s",strerror(errno));
        return -1;
    }

    ss.flags = ASYNC_SPD_CUST;
    ss.custom_divisor = ss.baud_base / baud;

	LOGE("##########################serial_set_speci_baud");

    if((ioctl(myfd,TIOCSSERIAL,&ss))<0){
        LOGE("BAUD: error to set serial_struct:%s",strerror(errno));
        return -2;
    }

    ioctl(myfd,TIOCGSERIAL,&ss_set);
    LOGE("BAUD: success set baud to %d,custom_divisor=%d,baud_base=%d",
            baud,ss_set.custom_divisor,ss_set.baud_base);

    return 0;
}


int set_speed(int fd, unsigned int speed)
{	
	int i;
	int status;
	unsigned int baud;
	struct termios Opt;

LOGE("#############set_speed, fd = %d", fd);
	
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
/*
  * Class:     com_kz_serialport_ctl_SerialControl
  * Method:    open
  * Signature: (Ljava/lang/String;I)Ljava/io/FileDescriptor;
  */
jobject 
Java_com_example_hellojni_HelloJni_open
   (JNIEnv* env,jobject thiz,jstring path, jint baudrate) {
     speed_t speed;
     jobject mFileDescriptor;
     int fd, ret = 2;
	 char value;
	//extern int errno;
  
     /* Check arguments */
     {
          speed = getBaudrate(baudrate);
		LOGE("!!!!!!!!!!baudrate=%d,speed=%d", baudrate, speed);
          if (speed == -1) {
          /* TODO: throw an exception */
               LOGE("Invalid baudrate");
               return NO_Baudrate;
          }
     }

	boardrate = baudrate;
      /* Opening device */
     {
         jboolean iscopy;
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
      }

	/*fd = open("/data/buard", O_RDWR);
		if(fd < 0){
			LOGE("#####open buard error");
			return -1;
		}*/
		
	 /* Configure device */
 	/*if(speed == B28800){
		//serial_set_speci_baud(openfd, 28800);
		LOGE("####@@@#serial_set_speci_baud 28800");
		value = '1';
		ret = write(fd, &value, 1);
		if(ret < 0){
			LOGE("#####write error");
			return -1;
		}
	}*/

	//else{
	//value = '0';
	//ret = write(fd, &value, 1);
	//if(ret < 0){
	//	LOGE("#####write error");
	//	return -1;
	//}
	  	set_speed(openfd,baudrate);
	//}

	  if (set_Parity(openfd,8,1,'N')== CONFIG_DEV_ERROR)
  	{
    		LOGE("#######Set Parity Error\n");
    		return CONFIG_DEV_ERROR;
  	}
  
     
	
        /* Create a corresponding file descriptor */
       {
             jclass cFileDescriptor = (*env)->FindClass(env, "java/io/FileDescriptor");
             jmethodID iFileDescriptor = (*env)->GetMethodID(env, cFileDescriptor, "<init>", "()V");
             jfieldID descriptorID = (*env)->GetFieldID(env, cFileDescriptor, "descriptor", "I");
             mFileDescriptor = (*env)->NewObject(env, cFileDescriptor, iFileDescriptor);
             (*env)->SetIntField(env, mFileDescriptor, descriptorID, (jint)openfd);
         }

        return mFileDescriptor; //返回打开的设备描述符
 }

/*
  * Class:     com_kz_serialport_ctl_SerialControl
  * Method:    close
  * Signature: ()V
  */
void 
Java_com_example_hellojni_HelloJni_close
   (JNIEnv* env, jobject thiz) {
       jclass SerialPortClass = (*env)->GetObjectClass(env, thiz);
       jclass FileDescriptorClass = (*env)->FindClass(env, "java/io/FileDescriptor");

      //×¢Òâ£ºmFdÊÓjavaÖÐ±äÁ¿Ãû¶ø¶š¡£

      jfieldID mFdID = (*env)->GetFieldID(env, SerialPortClass, "mFd", "Ljava/io/FileDescriptor;");
       jfieldID descriptorID = (*env)->GetFieldID(env, FileDescriptorClass, "descriptor", "I");

      jobject mFd = (*env)->GetObjectField(env, thiz, mFdID);
       jint descriptor = (*env)->GetIntField(env, mFd, descriptorID);

      LOGI("close(fd = %d)", descriptor);
       close(descriptor);
 }


void 
Java_com_example_hellojni_HelloJni_tcflush
   (JNIEnv* env, jobject thiz, jint type) {
	 jclass SerialPortClass = (*env)->GetObjectClass(env, thiz);
       jclass FileDescriptorClass = (*env)->FindClass(env, "java/io/FileDescriptor");

      //×¢Òâ£ºmFdÊÓjavaÖÐ±äÁ¿Ãû¶ø¶š¡£

      jfieldID mFdID = (*env)->GetFieldID(env, SerialPortClass, "mFd", "Ljava/io/FileDescriptor;");
       jfieldID descriptorID = (*env)->GetFieldID(env, FileDescriptorClass, "descriptor", "I");

      jobject mFd = (*env)->GetObjectField(env, thiz, mFdID);
       jint descriptor = (*env)->GetIntField(env, mFd, descriptorID);

	LOGI("################tcflush(fd = %d)", descriptor);
	tcflush(descriptor, type);
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

		sec = timeout /1000;
		usec = (timeout %1000)*1000;
		time_out.tv_sec = sec;
		time_out.tv_usec = usec;
		rc = select(fd+1, &readfds, NULL, NULL, &time_out);
			
		if(rc < 0){
			LOGE("#####select error\n");
			break;
		}
		else if(rc > 0){
			if(FD_ISSET(fd, &readfds)) {
				num = read(fd, p, 100);
				*len += num;
				p = p + num;
				num = 0;	
			}
			
		}
		
		else if(rc == 0){
			printf("########no data read\n");
			break;
		}
	}
	
	return rc;
}


jobject
Java_com_example_hellojni_HelloJni_read(JNIEnv* env, jobject thiz,jint length,  jint timeout){
	char buf[1024] = {0};
	int num, flage;
	int nfds, rc = -1,i, len, strlen = 0; 
	fd_set readfds; 
	struct timeval time_out; 
	unsigned int sec, usec;
	char *p = buf;
	//extern int errno; 

	if(openfd  < 0 || boardrate == 0){
		LOGE("#####fd < 0");
		return NO_DEVICE_OPEN;
	}
		
	 //set_speed(openfd,boardrate);
	 //set_Parity(openfd,8,1,'N');
  
	//LOGE("###########HelloJni_read, openfd=%d", openfd );
	

	if(length > 1024){
		LOGE("######too long data to read");
		return TOO_LONG_DATA;
	}

	jclass clsstring = (*env)->FindClass(env, "com/example/hellojni/HelloJni");
	jmethodID mid = (*env)->GetMethodID(env, clsstring, "report_data", "([B)V"); 
	
	tcflush(openfd, TCIOFLUSH);
	read_seting_echo(openfd, buf, timeout, &strlen);
	//for(i = 0; i < strlen; i++)
		//LOGE("buf[%d]=%x", i, buf[i]);
	
	jbyteArray carr = (*env)->NewByteArray(env,strlen);
	(*env)->SetByteArrayRegion(env, carr, 0, strlen, buf);
	(*env)->CallVoidMethod(env, thiz,  mid, carr);

	return strlen;	//返回读取的数据长度
}

jint
Java_com_example_hellojni_HelloJni_write(JNIEnv* env, jobject thiz,  jbyteArray arr, jint lenth, jint timeout){ //jstring string
	int num, i;
	int nfds, rc; 
	fd_set writefds; 
	struct timeval time_out; 
	unsigned int sec, usec;

	if(openfd < 0){
		LOGE("#########fd < 0");
		return NO_DEVICE_OPEN;
	}
	
	FD_ZERO(&writefds);
     	FD_SET(openfd,&writefds);
	if(timeout == -1){
		//wait for ever
		rc = select(openfd+1, NULL, &writefds, NULL, NULL); 
	}

	else if(timeout > 0){
		//wait for timeout
		sec = timeout /1000;
		usec = (timeout %1000)*1000;
		time_out.tv_sec = sec;
		time_out.tv_usec = usec;
		rc = select(openfd+1, NULL, &writefds, NULL, &time_out);
	}

	else if(timeout == 0){
		//no wait
		time_out.tv_sec = 0;
		time_out.tv_usec = 0;
		rc = select(openfd+1, NULL, &writefds, NULL, &time_out); 
	}

	if(rc < 0){
		LOGE("#####select error");
		return PREPARE_WRITE_ERROR;
	}

	if(rc > 0){
		char *tmpdata = (char *)(*env)->GetByteArrayElements(env, arr, NULL);
		//const char *str = (*env)->GetStringUTFChars(env, string, 0);
		//LOGE("##############str=%s", str);
		num = write(openfd, tmpdata, lenth);
		if(num > 0){
			LOGE("######write ok");
			//(*env)->ReleaseStringUTFChars(env, string, str);
			(*env)->ReleaseByteArrayElements(env, arr, tmpdata, 0);
			return num;//返回写入数据的长度
		}

		else if(num < 0){
			LOGE("######write error");
			//(*env)->ReleaseStringUTFChars(env, string, str);
			(*env)->ReleaseByteArrayElements(env, arr, tmpdata, 0);
			return WRITE_ERROR;
		}
	}

	if(rc == 0){
		return NOT_FOR_WRITE;
	}
	
}

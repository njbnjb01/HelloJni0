#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include<sys/time.h>
#include <sys/select.h>
#include <sys/timeb.h>
#include <pthread.h>
#include <stdio.h>
#include<unistd.h>
#include <jni.h>

#include <android/log.h>
#define LOG_TAG "SerialPort"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static int openfd = -1;
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
#define TOO_LONG_DATA -13c

#define  IR_IOC_MAGIC   'm'
#define   IO_CTL_RST 			_IO(IR_IOC_MAGIC,50)
#define   IO_CTL_W_BYTE  		_IO(IR_IOC_MAGIC,51)
#define   IO_CTL_R_BYTE  		_IO(IR_IOC_MAGIC,52)
#define u8  unsigned char 

jobject 
Java_com_example_hellojni_HelloJni_opendev
 (JNIEnv* env,jobject thiz,jstring path) {
    	 	jobject mFileDescriptor;
		  jboolean iscopy;
		  int fd1;
	
      		/* Opening device */
        	const char *path_utf = (char *)(*env)->GetStringUTFChars(env, path, &iscopy);
         	LOGI("Opening serial port %s", path_utf);
         	openfd = open(path_utf, O_RDWR);
         	LOGI("open() fd = %d", openfd);
        	 (*env)->ReleaseStringUTFChars(env, path, path_utf);
        	if (openfd == -1)
        	{
            		 /* Throw an exception */
             		LOGE("Cannot open dev");
            		/* TODO: throw an exception */
            		return OPEN_DEV_ERROR;
         	}
      	
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

      		LOGI("close(fd = %d)", descriptor);
       	close(descriptor);
 }


static int rest()
{
	unsigned char value = 0,ret;

	ret = ioctl(openfd, IO_CTL_RST,&value);
	if(ret < 0)
	{
		LOGE("IO_CTL_TRIG_L error\n");
		return -1;
	}
	return value;
}

u8 readbyte()
{
	u8 value = 0,ret;
	
	ret=ioctl(openfd, IO_CTL_R_BYTE,&value);
	if(ret < 0)
	{
		LOGE("IO_CTL_R_BYTE error");
		return -1;
	}
	return value;
}

static u8 writebyte(u8 value )
{
	u8 ret;
	
	ret = ioctl( openfd,  IO_CTL_W_BYTE, &value);
	if(ret < 0)
	{
		LOGE("IO_CTL_W_BYTE error");
		return -1;
	}
	
	return value;
}

static int readrom(char rom_cmd, char *buf)
{
	unsigned char  i;
	
	if (rest()) {
		LOGE("ds28e01 reset error");
		return CONFIG_DEV_ERROR;
	}

	writebyte(rom_cmd);

	for (i = 0; i <= 7; i ++) {
		buf[i] = readbyte();
	}

	return 8;
}



jobject
Java_com_example_hellojni_HelloJni_Ds28eRomRead(JNIEnv* env, jobject thiz,  jint ReadRom_Cmd){
	char buf[512] = {0};
	int i, num;

	LOGE("#########Ds28eRomRead");
	
	if(openfd < 0){
		LOGE("can not open ds28e01 dev");
		return NO_DEVICE_OPEN;
	}

	jclass clsstring = (*env)->FindClass(env, "com/example/hellojni/HelloJni");
	jmethodID mid = (*env)->GetMethodID(env, clsstring, "report_ds28e01_data", "([B)V"); 

	num = readrom(ReadRom_Cmd, buf);
	for(i = 0; i <num; i++)
		LOGE("ROMID[%d]=%x", i, buf[i]);

	jbyte *jbuf = (jbyte *)buf;
	jbyteArray carr = (*env)->NewByteArray(env,num);
	(*env)->SetByteArrayRegion(env, carr, 0, num, jbuf);
	(*env)->CallVoidMethod(env, thiz,  mid, carr);

	return num;//返回得到的字节数
	
}



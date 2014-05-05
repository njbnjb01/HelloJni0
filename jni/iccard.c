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

#define CFG_RD_ADDR 0x01

jobject 
Java_com_example_hellojni_HelloJni_opendev
 (JNIEnv* env,jobject thiz,jstring path) {
    	 	jobject mFileDescriptor;
		  jboolean iscopy;
		  int fd1;
	
      		/* Opening device */
        	const char *path_utf = (char *)(*env)->GetStringUTFChars(env, path, &iscopy);
         	LOGI("Opening dev path %s", path_utf);
         	openfd = open(path_utf, O_RDWR);
         	LOGI("open() iccard dev fd = %d", openfd);
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


jobject
Java_com_example_hellojni_HelloJni_ReadIccard(JNIEnv* env, jobject thiz,  jint addr, jint count){
	char buf[1024] = {0};
	int i, num, ret;

	LOGE("#########ReadIccard");
	
	if(openfd < 0){
		LOGE("can not open iccard dev");
		return NO_DEVICE_OPEN;
	}

	LOGE("#########ReadIccard openfd=%d", openfd);
	jclass clsstring = (*env)->FindClass(env, "com/example/hellojni/HelloJni");
	jmethodID mid = (*env)->GetMethodID(env, clsstring, "report_iccard_data", "([B)V"); 

	ret = ioctl(openfd, CFG_RD_ADDR, addr);
	if(ret < 0){
		LOGE("#####ioctl error");
		return -1;
	}
	num = read(openfd, buf, 100);
	if(num < 0){
		LOGE("#######read error\n");
	}
	
	for(i = 0; i <100; i++)
		LOGE("[%d]%x ", i,buf[i]);

	jbyte *jbuf = (jbyte *)buf;
	jbyteArray carr = (*env)->NewByteArray(env,count);
	(*env)->SetByteArrayRegion(env, carr, 0, count, jbuf);
	(*env)->CallVoidMethod(env, thiz,  mid, carr);
	
	return count;//返回读取的字节数
	
}



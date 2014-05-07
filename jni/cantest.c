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
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/net.h>
#include "can.h"
#include <jni.h>

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

enum sock_type {
	SOCK_STREAM	= 1,
	SOCK_DGRAM	= 2,
	SOCK_RAW	= 3,
	SOCK_RDM	= 4,
	SOCK_SEQPACKET	= 5,
	SOCK_DCCP	= 6,
	SOCK_PACKET	= 10,
};

#ifndef PF_CAN
#define PF_CAN 29
#endif

#ifndef AF_CAN
#define AF_CAN PF_CAN
#endif
static int close_can = 0;
struct sockaddr_can addr;
static int openfd = -1;

static int close_can1 = 0;
struct sockaddr_can addr1;
static int openfd1 = -1;

#include <android/log.h>
#define LOG_TAG "CanTest"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


jobject 
Java_com_example_hellojni_HelloJni_opendev
 (JNIEnv* env,jobject thiz,jstring path) {
    	 	jobject mFileDescriptor;
		  jboolean iscopy;
		struct ifreq ifr;

      		/* Opening device */
        	const char *path_utf = (char *)(*env)->GetStringUTFChars(env, path, &iscopy);
         	LOGI("Opening dev path %s", path_utf);
        if(strcmp(path_utf,"can0")==0){
			openfd = socket(PF_CAN,SOCK_RAW,CAN_RAW);
			if (openfd < 0 )
				{
						 /* Throw an exception */
						LOGE("Cannot open dev");
						/* TODO: throw an exception */
						return OPEN_DEV_ERROR;
				}

			strcpy((char *)(ifr.ifr_name),path_utf);
			ioctl(openfd, SIOCGIFINDEX,&ifr);
			LOGE("#####can_ifindex = %x\n",ifr.ifr_ifindex);

			addr.can_family = AF_CAN;
			addr.can_ifindex = ifr.ifr_ifindex;
			bind(openfd, (struct sockaddr*)&addr, sizeof(addr));
				 (*env)->ReleaseStringUTFChars(env, path, path_utf);
			 system("/system/bin/ip link set can0 type can bitrate 125000");
			 system("/system/bin/ifconfig can0 up");
        }else{
			openfd1 = socket(PF_CAN,SOCK_RAW,CAN_RAW);
			if (openfd1 < 0 )
				{
						 /* Throw an exception */
						LOGE("Cannot open dev");
						/* TODO: throw an exception */
						return OPEN_DEV_ERROR;
				}

			strcpy((char *)(ifr.ifr_name),path_utf);
			ioctl(openfd1, SIOCGIFINDEX,&ifr);
			LOGE("#####can_ifindex = %x\n",ifr.ifr_ifindex);

			addr1.can_family = AF_CAN;
			addr1.can_ifindex = ifr.ifr_ifindex;
			bind(openfd1, (struct sockaddr*)&addr1, sizeof(addr1));
				 (*env)->ReleaseStringUTFChars(env, path, path_utf);
			system("/system/bin/ip link set can1 type can bitrate 125000");
			system("/system/bin/ifconfig can1 up");
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
   (JNIEnv* env, jobject thiz, jint m) {
   		unsigned i;
   		jfieldID mFdID;
       	jclass SerialPortClass = (*env)->GetObjectClass(env, thiz);
      		jclass FileDescriptorClass = (*env)->FindClass(env, "java/io/FileDescriptor");
      		if(m==0){
      		 mFdID = (*env)->GetFieldID(env, SerialPortClass, "mFd", "Ljava/io/FileDescriptor;");
      		}else{
      		 mFdID = (*env)->GetFieldID(env, SerialPortClass, "mFd1", "Ljava/io/FileDescriptor;");
      		}

       	jfieldID descriptorID = (*env)->GetFieldID(env, FileDescriptorClass, "descriptor", "I");
     		jobject mFd = (*env)->GetObjectField(env, thiz, mFdID);
       	jint descriptor = (*env)->GetIntField(env, mFd, descriptorID);

      		LOGI("close(fd = %d)", descriptor);
       	close(descriptor);
 }

jobject
Java_com_example_hellojni_HelloJni_ReadCan(JNIEnv* env, jobject thiz, jint m){
	char buf[1024] = {0};
	int i,  ret, s, len;
	unsigned long nbytes;
	struct ifreq ifr;
	struct can_frame frame;
	jboolean iscopy;

	LOGE("#########ReadCan");
	if(openfd < 0){
		LOGE("can not open can dev");
		return NO_DEVICE_OPEN;
	}
	if(m==0){
	nbytes = recvfrom(openfd,&frame,sizeof(struct can_frame),0,(struct sockaddr *)&addr,&len);
	////for debug
	ifr.ifr_ifindex = addr.can_ifindex;
	ioctl(s,SIOCGIFNAME,&ifr);
	LOGE("Received a CAN frame from interface %s\n",ifr.ifr_name);
	LOGE("frame message\n"
		"--can_id = %x\n"
		"--can_dlc = %x\n"
		"--data = %s\n",frame.can_id,frame.can_dlc,frame.data);
	////////////////////
	}else{
		nbytes = recvfrom(openfd1,&frame,sizeof(struct can_frame),0,(struct sockaddr *)&addr1,&len);
		////for debug
		ifr.ifr_ifindex = addr1.can_ifindex;
		ioctl(s,SIOCGIFNAME,&ifr);
		LOGE("Received a CAN frame from interface %s\n",ifr.ifr_name);
		LOGE("frame message\n"
			"--can_id = %x\n"
			"--can_dlc = %x\n"
			"--data = %s\n",frame.can_id,frame.can_dlc,frame.data);
	}

	return (*env)->NewStringUTF(env, frame.data);	//返回得到的数据
}


jobject
Java_com_example_hellojni_HelloJni_SendCan(JNIEnv* env, jobject thiz, jint  canid, jstring value, jint m){
	struct ifreq ifr;
	struct can_frame frame;
	jboolean iscopy;
	int nbytes;

	if(openfd < 0){
		LOGE("can not open can dev");
		return NO_DEVICE_OPEN;
	}

	frame.can_id = canid;
	const char *can_date = (char *)(*env)->GetStringUTFChars(env, value, &iscopy);
	strcpy((char *)frame.data,can_date);
	frame.can_dlc = strlen(frame.data);
	if(m==0)
	nbytes = sendto(openfd,&frame,sizeof(struct can_frame),0,(struct sockaddr*)&addr,sizeof(addr));
	else
		nbytes = sendto(openfd1,&frame,sizeof(struct can_frame),0,(struct sockaddr*)&addr1,sizeof(addr1));

	(*env)->ReleaseStringUTFChars(env, value, can_date);
	return nbytes;//返回写入的字节数
}




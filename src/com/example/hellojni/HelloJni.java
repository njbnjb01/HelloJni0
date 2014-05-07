/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.hellojni;


import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

import android.app.Activity;
import android.app.ProgressDialog;
import android.app.TabActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TabHost;
import android.widget.TextView;
import android.widget.Toast;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import java.io.BufferedReader;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.text.DecimalFormat;

/*#define NO_Baudrate -2 //波特率设置不对
#define OPEN_DEV_ERROR -3 //打开设备失败
#define CONFIG_DEV_ERROR -4 //配置设备失败
#define SETTIMEOUT_ERROR -5 //设置超时时间失败
#define NOT_FOR_READ -6 //没有数据可读
#define PREPARE_READ_ERROR -7 //读数据准备失败
#define READ_DATA_ERROR -8 //读数据失败
#define WRITE_ERROR -9 //写数据失败
#define NO_DEVICE_OPEN -10 //没有可用的设备
#define NOT_FOR_WRITE -11 //不能写数据
#define PREPARE_WRITE_ERROR -12 //准备写数据失败
#define TOO_LONG_DATA -13 //一次要求读取的数据太多
*/

/*#define		ReadRom		0x33
#define		MatcRom		0x55
#define		SeacRom		0xF0
#define		SkipRom		0xCC
#define		Resume		0xA5
#define		OvskRom		0x3C
#define		OvmaRom		0x69

//The next part is the Memory function commands (applied for DS2431 and DS28E01-100)
#define		Wrscratch	0x0F
#define		Rescratch	0xAA
#define		Coscratch	0x55
#define		Redmemory	0xF0

#define		Loadfirse	0x5A
#define		Comnextse	0x33
#define		Reautpage	0xA5
#define		Anautpage	0xCC
#define		Refreshsc	0xA3
*/

public class HelloJni extends Activity{
  /** Called when the activity is first created. */
	private Button send;
	private Button read;
	private Button close;
	private Button open;
	private Button flush;
	private Button irscan;
	private Button readrom;
	private Button readiccard;
	private Button readrfidtag;
	private Button readcan;
	private Button sendcan;
	private int TCIFLUSH = 1;//清除读缓冲区的数据
	private int TCOFLUSH = 2;//清除写缓冲区的数据
	private int TCIOFLUSH = 3;//清除读写缓冲区的数据
	private int ReadRom = 0x33;//读DS28E01 ROM
	
	private Button close1;
	private Button open1;
	private Button readcan1;
	private Button sendcan1;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sample);
        
        send = (Button)findViewById(R.id.send);
        send.setOnClickListener(sendlisten);
        read = (Button)findViewById(R.id.read);
        read.setOnClickListener(readlisten);
        close = (Button)findViewById(R.id.close);
        close.setOnClickListener(closelisten);
        open = (Button)findViewById(R.id.open);
        open.setOnClickListener(openlisten);
        irscan = (Button)findViewById(R.id.irscan);
        irscan.setOnClickListener(irscanlisten);
        readrom  = (Button)findViewById(R.id.readrom);
        readrom.setOnClickListener(readromlisten);
        readiccard = (Button)findViewById(R.id.readiccard);
        readiccard.setOnClickListener(readiccardlisten);
        readrfidtag = (Button)findViewById(R.id.readrfidtag);
        readrfidtag.setOnClickListener(readrfidtaglisten);
        readcan = (Button)findViewById(R.id.readcan);
        readcan.setOnClickListener(readcanlisten);
        sendcan = (Button)findViewById(R.id.sendcan);
        sendcan.setOnClickListener(sendcanlisten);
        
        close1 = (Button)findViewById(R.id.close1);
        close1.setOnClickListener(closelisten1);
        open1 = (Button)findViewById(R.id.open1);
        open1.setOnClickListener(openlisten1);
        readcan1 = (Button)findViewById(R.id.readcan1);
        readcan1.setOnClickListener(readcanlisten1);
        sendcan1 = (Button)findViewById(R.id.sendcan1);
        sendcan1.setOnClickListener(sendcanlisten1);
        
        //exec_cmd("/system/bin/su");
      //  flush = (Button)findViewById(R.id.);
       // flush.setOnClickListener(flushlisten);
    }
    //打开设备
	OnClickListener openlisten = new OnClickListener(){
    	public void onClick(View arg0) {
      	   	//mFd = open("/dev/ttymxc2", 19200);//此方法用来打开串口相关的设备，比如红外扫描，433，RFID，以及DB9
      	   	//其中433是/dev/ttymxc2，RFID是/dev/ttymxc3，红外扫描是/dev/ttymxc4，DB9是/dev/ttymxc0
    		//mFd = opendev("/dev/ds28e01");//打开DS28E01加密芯片
    		//mFd = opendev("/dev/miccard");//打开iccard 
      	   	mFd = opendev("can0");//打开can设备 
      	    if (mFd == null) {
      	    		Log.e(TAG, "native open returns null");
      	    		return ;
      	   	}
      	   //	exec_cmd("ip link set can0 type can bitrate 125000");
      	  // 	exec_cmd("ifconfig can0 up ");
      	  //tcflush(TCIOFLUSH);
    	}	    	
  	};
	//打开设备
	OnClickListener openlisten1 = new OnClickListener(){
	    	public void onClick(View arg0) {
	      	   	//mFd = open("/dev/ttymxc2", 19200);//此方法用来打开串口相关的设备，比如红外扫描，433，RFID，以及DB9
	      	   	//其中433是/dev/ttymxc2，RFID是/dev/ttymxc3，红外扫描是/dev/ttymxc4，DB9是/dev/ttymxc0
	    		//mFd = opendev("/dev/ds28e01");//打开DS28E01加密芯片
	    		//mFd = opendev("/dev/miccard");//打开iccard 
	      	   	mFd1 = opendev("can1");//打开can设备 
	      	    if (mFd1 == null) {
	      	    		Log.e(TAG, "native open returns null");
	      	    		return ;
	      	   	}
	      	   // exec_cmd("ip link set can1 type can bitrate 125000");
	      	   	//exec_cmd("ifconfig can1 up ");
	      	  //tcflush(TCIOFLUSH);
	    	}
		    	
		};
	//关闭设备
	OnClickListener closelisten = new OnClickListener(){
	    	public void onClick(View arg0) {
	    	//	tcflush(TCIOFLUSH);
	    		close(0);
	    		exec_cmd("ifconfig can0 down");
	    	}
		    	
		};	
	//关闭设备
	OnClickListener closelisten1 = new OnClickListener(){
	    	public void onClick(View arg0) {
	    		//tcflush(TCIOFLUSH);
	    		close(1);
	    		exec_cmd("ifconfig can1 down");
	    	}
		    	
		};
		
	 OnClickListener sendcanlisten = new OnClickListener(){
	    	public void onClick(View arg0) {
	    		String str = "hello";
	    		int canid = 0x123;
	    		//int len = str.length();
	 			
				int ret = SendCan(canid, str, 0);
				Log.e("#####@@@@@@#####write ret="+ret, TAG);
	    	}
		};
	 OnClickListener sendcanlisten1 = new OnClickListener(){
	    	public void onClick(View arg0) {
	    		String str = "hello";
	    		int canid = 123;
	    		//int len = str.length();
	 			
				
				int ret = SendCan(canid, str, 1);
				Log.e("#####@@@@@@#####write ret="+ret, TAG);
	    	}
		    	
		};
		
	OnClickListener readcanlisten = new OnClickListener(){
    	public void onClick(View arg0) {
    		new Thread(new Runnable() {
				
				@Override
				public void run() {
					// TODO Auto-generated method stub
					String candate = ReadCan(0);
						Log.e(TAG, "candate:"+candate);
				}
			}).start();
    	}
	    	
	};	
	OnClickListener readcanlisten1 = new OnClickListener(){
    	public void onClick(View arg0) {
    		new Thread(new Runnable() {
				
				@Override
				public void run() {
					// TODO Auto-generated method stub
					String candate = ReadCan(1);
						Log.e(TAG, "candate:"+candate);
				}
			}).start();
    	}
	    	
	};	

	public void exec_cmd(String cmd){		
		String con="";
		String result="";
		  try {
			/* Missing read/write permission, trying to chmod the file */
			Process p;
			p = Runtime.getRuntime().exec(cmd);
			BufferedReader br=new BufferedReader(new InputStreamReader(p.getInputStream()));
			while((result=br.readLine())!=null){
				con+=result;
			}
		} catch (Exception e) {
			e.printStackTrace();
			throw new SecurityException();
		}
		  Log.e("con: "+con, TAG);
	}
		
    
    
    
    //写数据，参数为要写的byte字节数据，要写的大小，以及超时设置。
    //对于超时参数，当为-1时，表示一直等到设备有数据到来，否则一直阻塞或者底层准备写失败
    //当为0时，表示写的时候假如设备不忙，可以写的时候就会立即写，否则返回
    //当为大于0的时候，表示在设置的这段时间内设备可以被写的时候就会立即写，如果在设置时间超时了设备还是不可写的话就返回
    OnClickListener sendlisten = new OnClickListener(){
    	public void onClick(View arg0) {
    		//String str = "hello_jni";
    		byte[] reset = new byte[3];
			reset[0] = 0x72;
			reset[1] = 0x73;
 			reset[2] = 0x74;
    		//int len = str.length();
 			
			int ret = write(reset, 3, 5000);
			Log.e("#####@@@@@@#####write ret="+ret, TAG);
    	}
	    	
	};
	

	
	//读书据，参数为要求读数据的长度，以及读取数据的超时设置，超时意义与写相似
	 OnClickListener readlisten = new OnClickListener(){
	    	public void onClick(View arg0) {
	    		new Thread(new Runnable() {
					
					@Override
					public void run() {
						// TODO Auto-generated method stub
						DecimalFormat nf = new DecimalFormat("02");
			    		//byte[] buffer = new byte[128];
			    		int ret = read(10, 100);
			    		Log.e("##@@@@@########read ret="+ret, TAG);
					}
				}).start();
	    	}
		    	
		};
		
		//红外扫描数据，调用一次函数读取一次
		OnClickListener irscanlisten = new OnClickListener(){
	    	public void onClick(View arg0) {
	    		new Thread(new Runnable() {
					
					@Override
					public void run() {
						// TODO Auto-generated method stub
							String scandate = readirscan(1000, 500000);
							Log.e(TAG, "scandate:"+scandate);
					}
				}).start();
	    	}
		    	
		};
		
		OnClickListener readromlisten = new OnClickListener(){
	    	public void onClick(View arg0) {
	    		new Thread(new Runnable() {
					
					@Override
					public void run() {
						// TODO Auto-generated method stub
							Log.e(TAG, "Ds28eRomRead:");
							int ret = Ds28eRomRead(ReadRom);
							Log.e(TAG, "ret:"+ret);
					}
				}).start();
	    	}
		    	
		};
		
		OnClickListener readiccardlisten = new OnClickListener(){
	    	public void onClick(View arg0) {
	    		new Thread(new Runnable() {
					
					@Override
					public void run() {
						// TODO Auto-generated method stub
							Log.e(TAG, "iccardread:");
							int ret = ReadIccard(0, 50);
							Log.e(TAG, "ret:"+ret);
					}
				}).start();
	    	}
		    	
		};
		
		OnClickListener readrfidtaglisten = new OnClickListener(){
	    	public void onClick(View arg0) {
	    		new Thread(new Runnable() {
					
					@Override
					public void run() {
						// TODO Auto-generated method stub
							Log.e(TAG, "rfidtagread:");
							readRfidTag(100000);
					}
				}).start();
	    	}
		    	
		};
		
		
					
    private String TAG = "mytest";
    private FileDescriptor mFd;
    
    private FileDescriptor mFd1;
    //private FileInputStream mFileInputStream;
    //private FileOutputStream mFileOutputStream;
    
	private native static FileDescriptor open(String path, int baudrate);
	private native static FileDescriptor opendev(String path);
	public native void close(int m);
	public native int read(int num, int timeout);//timeout is ms
	public native int write(byte[] str, int num, int timeout);//timeout is ms
	public native  void tcflush(int type);//clean tty
	public native String readirscan(int timeout, int scan_interval);
	public native int Ds28eRomRead(int RomCMD);
	public native int ReadIccard(int addr, int count);
	public native void readRfidTag(int timeout);
	public native String ReadCan(int m);
	public native int SendCan(int canid, String str, int m);
	
	 static {
		 	//sel433-jni是使用433功能和DB9串口调用的库函数
		 	//irscan-jni是使用红外扫描时调用的函数
		 	//ds28e01-jni是使用加密芯片调用的库函数
		 	//iccard-jni是读写IC卡调用的库函数
		 //rfid-jni是操作RFID设备的库函数
	        //System.loadLibrary("sel433-jni");
		 	//System.loadLibrary("irscan-jni");
		 	//System.loadLibrary("ds28e01-jni");
		 	//System.loadLibrary("iccard-jni");
		 	//System.loadLibrary("rfid-jni");
		 	System.loadLibrary("cantest-jni");
	    }
	 
	 //当成功读到433数据时回调此方法
	 public void report_data(byte[] arr){
		 Log.e(TAG,"@@@@report 433 data");
		 DecimalFormat nf = new DecimalFormat("02"); 
		 	int ret = arr.length;
			for(int i=0;i <ret;i++)
				//Log.e(TAG,Byte.toString(arr[i]));
				Log.e(TAG,nf.format(arr[i]));
	 }
	 
	//当成功读到DS28E01数据时回调此方法
	 public void report_ds28e01_data(byte[] arr){
			 Log.e(TAG,"@@@@report_ds28e01_data");
			 DecimalFormat nf = new DecimalFormat("02"); 
			 	int ret = arr.length;
				for(int i=0;i <ret;i++)
					//Log.e(TAG, Byte.toString(arr[i]));
					Log.e(TAG,nf.format(arr[i]));
					//Log.e(TAG, "arr[i]"+arr[i]);
		 }
	 
	 public void report_iccard_data(byte[] arr){
		 Log.e(TAG,"@@@@report_iccard_data");
		 DecimalFormat nf = new DecimalFormat("02"); 
		 	int ret = arr.length;
			for(int i=0;i <ret;i++)
				//Log.e(TAG, Byte.toString(arr[i]));
				Log.e(TAG,nf.format(arr[i]));
				//Log.e(TAG, "arr[i]"+arr[i]);
	 }
	 
	 public void report_rfidtag(byte[] arr) throws UnsupportedEncodingException{
		 Log.e(TAG,"@@@@report_rfidtag");
		 DecimalFormat nf = new DecimalFormat("02"); 
		 int ret = arr.length;
		 for(int i=0;i <ret;i++)
				//Log.e(TAG, Byte.toString(arr[i]));
				Log.e(TAG,nf.format(arr[i]));
				//Log.e(TAG, "arr[i]"+arr[i]);
	 }
	 
	 public void report_can_data(byte[] arr){
		 Log.e(TAG,"@@@@report can data");
		 DecimalFormat nf = new DecimalFormat("02"); 
		 	int ret = arr.length;
			for(int i=0;i <ret;i++)
				//Log.e(TAG,Byte.toString(arr[i]));
				Log.e(TAG,nf.format(arr[i]));
	 }
	 
}

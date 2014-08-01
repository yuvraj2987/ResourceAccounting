package com.cse.buffalo.resourceusage;

import java.io.DataOutputStream;
import java.io.IOException;
import java.util.concurrent.ConcurrentHashMap;

import android.os.Bundle;
//import android.os.ICurrentTime;
import android.os.CurrentTimeService;
import android.app.Activity;
import android.content.Context;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.widget.TextView;

public class MainActivity extends Activity {
	public String TAG_RESOURCE_USAGE = "Resource Usage";
	public static int counter = 0;
	public static boolean flag = true;
	public static TextView textView = null;
	static {
		System.loadLibrary("resource_usagejni"); // "myjni.dll" in Windows, "libmyjni.so" in Unixes
	}
	//public static ICurrentTime resourceUsage = null;
	public static CurrentTimeService resourceUsage = null;
	public static ConcurrentHashMap<Integer, OutputTableRow> sharedMap = new ConcurrentHashMap<Integer, OutputTableRow>();
	BackgroundThread bkThread = null;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		resourceUsage = (CurrentTimeService) getSystemService(Context.CURRENT_TIME_SERVICE);
        Log.e(TAG_RESOURCE_USAGE, resourceUsage.toString());
		textView = new TextView(this);
		setContentView(textView);
        textView.setMovementMethod(new ScrollingMovementMethod());
		bkThread = (BackgroundThread) new BackgroundThread(this).executeOnExecutor(BackgroundThread.THREAD_POOL_EXECUTOR, (Void)null);
	}

	@Override
	protected void onResume() {
		super.onResume();
		changePermissions();
	}
	/**
	 * Change permissions of /dev/event_log_dev 
	 * We need to root the device to make this work
	 */
	private void changePermissions()
	{
		Process chperm;
		try {
			chperm=Runtime.getRuntime().exec("su");
			DataOutputStream os = 
					new DataOutputStream(chperm.getOutputStream());
			os.writeBytes("chmod 777 /dev/event_log_dev\n");
			os.flush();

			os.writeBytes("exit\n");
			os.flush();

			chperm.waitFor();

		} catch (IOException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

}
package com.cse.buffalo.resourceusage;

import java.io.FileOutputStream;
import java.util.HashMap;
import java.util.Set;
import java.util.Map;

import android.content.Context;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.RemoteException;
import android.util.Log;

public class BackgroundThread extends AsyncTask<Void, OutputTableRow, Void> {
	public String TAG_RESOURCE_USAGE = "Resource Usage";

	public native int[] getMinors(); 
	public native OutputTableRow getReading(int minor);
	public native String getProcessName(int pid);

	/*may need to adjust \t in string according to cell phone */
	public final String SENSOR_NAME 		= "1.Sensor Name:		\t";
	public final String OPENED_TIME 		= "2.Opened Time:		\t";//time1;
	public final String EVENT_GENERATED 	= "3.Events Generated:	";// long log_event_generated_count;
	public final String EVENT_DROPPED 		= "4.Events Dropped:	\t";//long log_event_dropped_count;
	public final String PROCESS_NAME 		= "5.Process(es):		\t\t";// processName;
	public final String EVENT_CONSUMED 	= "6.Events Consumed:	";//long log_event_consumed_count;
	public final String LATENCY 			= "7.Latency:			\t\t\t";//String time2;
	
	private HashMap<String, FileOutputStream> foshm = new HashMap<String, FileOutputStream>();
	
	private Context mContext;
    public BackgroundThread(Context context) {
        mContext = context;
    } 

	@Override
	protected Void doInBackground(Void... params) {
		/*
		 * 1. Get all minor numbers
		 * 2. Already have the handle to SystemService 
		 * 3. In while(true) loop, 
		 * 		call getPidSensorMap() method to get all <pid, sensors> pairs.
		 * 		call native getReading() method to get reading of one row..this will have sensor name
		 * 		in the map<pid,sensors>, if sensor name in reading == one of sensors in map
		 * 			get the name of that process
		 * 			construct the whole bundle for one sensor
		 * 			put the bundle in some synchronized data structure( MainActivity will read everything from this)
		 * 		sleep for 10 seconds
		 */
		int [] minors = getMinors();
		if(minors!=null){

			OutputTableRow[] output = new OutputTableRow[minors.length];
			//ConcurrentHashMap<String, String> pidSensorMap = null;
			Map<String, String> pidSensorMap = null;
			while(true){
				try {
					//pidSensorMap = (ConcurrentHashMap<String, String>) MainActivity.resourceUsage.getPidSensorMap();
					pidSensorMap = (Map<String, String>) MainActivity.resourceUsage.getPidSensorMap();
					Log.d(TAG_RESOURCE_USAGE, pidSensorMap.toString());
				} catch (RemoteException e) {
					Log.d(TAG_RESOURCE_USAGE, "Exception "+e.toString());
					e.printStackTrace();
					
					return null;
				}
				if(minors!=null){
					for(int minor : minors){ //for each sensor
						OutputTableRow oneRow = getReading(minor);//get reading for that sensor
						oneRow.minor = minor;
						Log.d(TAG_RESOURCE_USAGE,"got the values");
						Set<String> pids = pidSensorMap.keySet();
						Log.v(TAG_RESOURCE_USAGE, pids.toString());
						for(String pid : pids){ //check whether we have received pid for this sensor from framework
							Log.v(TAG_RESOURCE_USAGE, "pid: " + pid);
							String sensor = pidSensorMap.get(pid); // sensor(s) that pid is using
							Log.v(TAG_RESOURCE_USAGE, "sensor: " + sensor);
							if(sensor.contains(oneRow.log_name)){
								String nameOfProcessUsingThisSensor = getProcessName(Integer.parseInt(pid));
								Log.v(TAG_RESOURCE_USAGE, "Process found: " + nameOfProcessUsingThisSensor);
								if(nameOfProcessUsingThisSensor != null ){
									oneRow.processName = oneRow.processName.concat(","+nameOfProcessUsingThisSensor);
								}
							}
						}
						output[minor] = oneRow; 
						Log.v(TAG_RESOURCE_USAGE, "Processes: " + oneRow.processName);
					}
					publishProgress(output);
				}
				try {
					Thread.sleep(5000);
				} catch (InterruptedException e) {
					Log.d(TAG_RESOURCE_USAGE, "Exception "+e.toString());
					e.printStackTrace();
				}
			}
		}
		return null;
	}
	@Override
	protected void onProgressUpdate(OutputTableRow... values) {
		MainActivity.textView.setText("");

		for(int i =0;i<values.length;i++){
			
			OutputTableRow oneRow = values[i];
			
			//write header to log files
			if (!foshm.containsKey(oneRow.log_name)) {
				try {
					foshm.put(oneRow.log_name, mContext.openFileOutput(oneRow.log_name, Context.MODE_PRIVATE));
					String header = "TIME_STAMP\tOPENED_TIME\tEVENT_GENERATED\tEVENT_DROPPED\tPROCESS_NAME\tEVENT_CONSUMED\tLATENCY\n";
					foshm.get(oneRow.log_name).write(header.getBytes());
				} catch(Exception e) {
					e.printStackTrace();
				}
			}
			
			StringBuilder sb = new StringBuilder();
			sb.append("\n"+SENSOR_NAME + oneRow.log_name);
			sb.append("\n"+OPENED_TIME + oneRow.time1);
			sb.append("\n"+EVENT_GENERATED + oneRow.log_event_generated_count);
			sb.append("\n"+EVENT_DROPPED + oneRow.log_event_dropped_count);
			sb.append("\n"+PROCESS_NAME + oneRow.processName);
			sb.append("\n"+EVENT_CONSUMED + oneRow.log_event_consumed_count);
			sb.append("\n"+LATENCY + oneRow.time2);

			if(i%2==0)
				MainActivity.textView.setTextColor(Color.HSVToColor(new float[]{28,100,50.2f}));
			else
				MainActivity.textView.setTextColor(Color.BLACK);
			MainActivity.textView.append(sb.toString()+"\n");
			
			//log to internal file storage
			try {
				String ts = String.valueOf(System.currentTimeMillis()/1000);
				String d = ts+"\t"+oneRow.time1+"\t"+oneRow.log_event_generated_count+"\t"+oneRow.log_event_dropped_count
						   +"\t"+oneRow.processName+"\t"+oneRow.log_event_consumed_count+"\t"+oneRow.time2+"\n";
				foshm.get(oneRow.log_name).write(d.getBytes());
				foshm.get(oneRow.log_name).flush();
			} catch(Exception e) {
				e.printStackTrace();
			}
			
		}
	}
}



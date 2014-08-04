package com.cse.buffalo.resourceusage;
/*
 * printf("%-20s%-20s%-20ld%-20ld%-20s%-20ld%-20s\n",log.name,time1,log.event_generated.count
					,log.event_dropped.count,pname,log.event_consumed[i].counts.count,time2);

 */
public class OutputTableRow {

	int minor;
	String log_name;
	String time1;
	long log_event_generated_count;
	long log_event_dropped_count;
	String processName;
	long log_event_consumed_count;
	String time2;
	public OutputTableRow(
			long log_event_generated_count
			, long log_event_dropped_count
			, long log_event_consumed_count
			,String log_name
			, String time1
			,String time2,
			String processName ) {
		super();
		this.log_name = log_name;
		this.time1 = time1;
		this.log_event_generated_count = log_event_generated_count;
		this.log_event_dropped_count = log_event_dropped_count;
		this.processName = processName;
		this.log_event_consumed_count = log_event_consumed_count;
		this.time2 = time2;
	}
	
	public OutputTableRow(long log_event_dropped_count,
			long log_event_consumed_count, String time2, String processName) {
		super();
		this.log_event_dropped_count = log_event_dropped_count;
		this.processName = processName;
		this.log_event_consumed_count = log_event_consumed_count;
		this.time2 = time2;
	}

	public OutputTableRow() {
		super();
	}
	
}

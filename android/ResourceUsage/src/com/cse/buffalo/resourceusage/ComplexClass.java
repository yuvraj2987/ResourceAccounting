package com.cse.buffalo.resourceusage;

public class ComplexClass {
	public int a = -900;
	public String b;
	public long c = -100;
	public ComplexClass() {
		super();
		this.a = 100;
		this.b = "newname";
		this.c = 10000;
	}
	public ComplexClass(int a, String b, long c) {
		super();
		this.a = a;
		this.b = b;
		this.c = c;
	}
	public ComplexClass(String aa){
		this.b=aa;
	}
}

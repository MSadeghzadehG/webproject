

package com.example;

import java.io.File;
import java.lang.management.ManagementFactory;


public class BootRunApplication {

	protected BootRunApplication() {

	}

	public static void main(String[] args) {
		int i = 1;
		for (String entry : ManagementFactory.getRuntimeMXBean().getClassPath()
				.split(File.pathSeparator)) {
			System.out.println(i++ + ". " + entry);
		}
	}

}

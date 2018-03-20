

package com.google.gwt.corp.compatibility;

import java.io.OutputStream;
import java.io.PrintStream;


public class ConsolePrintStream extends PrintStream {

	StringBuilder buf = new StringBuilder();

	public ConsolePrintStream () {
		super((OutputStream)null);
	}

	public void print (String s) {

		while (true) {
			int cut = s.indexOf('\n');
			if (cut == -1) {
				break;
			}
			println(s.substring(0, cut));
			s = s.substring(cut + 1);
		}

		buf.append(s);
	}

	public native void consoleLog (String msg) ;

	public void print (char c) {
		if (c == '\n') {
			println("");
		} else {
			buf.append(c);
		}
	}

	public void println () {
		println("");
	}

	@Override
	public void println (String s) {
		buf.append(s);
		consoleLog(buf.toString());
		buf.setLength(0);
	}

}

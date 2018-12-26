

package org.springframework.boot.loader.data;

import java.io.IOException;
import java.io.InputStream;


public interface RandomAccessData {

	
	InputStream getInputStream() throws IOException;

	
	RandomAccessData getSubsection(long offset, long length);

	
	byte[] read() throws IOException;

	
	byte[] read(long offset, long length) throws IOException;

	
	long getSize();

}

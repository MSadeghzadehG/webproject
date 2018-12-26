

package org.springframework.boot.actuate.trace.http;

import java.net.URI;
import java.util.List;
import java.util.Map;


public interface TraceableRequest {

	
	String getMethod();

	
	URI getUri();

	
	Map<String, List<String>> getHeaders();

	
	String getRemoteAddress();

}

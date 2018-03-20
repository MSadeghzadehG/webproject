

package org.springframework.boot.actuate.trace.http;

import java.util.List;
import java.util.Map;


public interface TraceableResponse {

	
	int getStatus();

	
	Map<String, List<String>> getHeaders();

}

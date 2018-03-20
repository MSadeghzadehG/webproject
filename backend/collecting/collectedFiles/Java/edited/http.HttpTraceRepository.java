

package org.springframework.boot.actuate.trace.http;

import java.util.List;


public interface HttpTraceRepository {

	
	List<HttpTrace> findAll();

	
	void add(HttpTrace trace);

}

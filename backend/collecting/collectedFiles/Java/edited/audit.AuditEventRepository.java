

package org.springframework.boot.actuate.audit;

import java.time.Instant;
import java.util.List;


public interface AuditEventRepository {

	
	void add(AuditEvent event);

	
	List<AuditEvent> find(String principal, Instant after, String type);

}

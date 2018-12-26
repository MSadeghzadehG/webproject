

package org.springframework.boot.actuate.health;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;


public abstract class AbstractHealthAggregator implements HealthAggregator {

	@Override
	public final Health aggregate(Map<String, Health> healths) {
		List<Status> statusCandidates = new ArrayList<>();
		for (Map.Entry<String, Health> entry : healths.entrySet()) {
			statusCandidates.add(entry.getValue().getStatus());
		}
		Status status = aggregateStatus(statusCandidates);
		Map<String, Object> details = aggregateDetails(healths);
		return new Health.Builder(status, details).build();
	}

	
	protected abstract Status aggregateStatus(List<Status> candidates);

	
	protected Map<String, Object> aggregateDetails(Map<String, Health> healths) {
		return new LinkedHashMap<>(healths);
	}

}

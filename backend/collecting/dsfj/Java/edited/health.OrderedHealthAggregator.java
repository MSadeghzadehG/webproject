

package org.springframework.boot.actuate.health;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;

import org.springframework.util.Assert;


public class OrderedHealthAggregator extends AbstractHealthAggregator {

	private List<String> statusOrder;

	
	public OrderedHealthAggregator() {
		setStatusOrder(Status.DOWN, Status.OUT_OF_SERVICE, Status.UP, Status.UNKNOWN);
	}

	
	public void setStatusOrder(Status... statusOrder) {
		String[] order = new String[statusOrder.length];
		for (int i = 0; i < statusOrder.length; i++) {
			order[i] = statusOrder[i].getCode();
		}
		setStatusOrder(Arrays.asList(order));
	}

	
	public void setStatusOrder(List<String> statusOrder) {
		Assert.notNull(statusOrder, "StatusOrder must not be null");
		this.statusOrder = statusOrder;
	}

	@Override
	protected Status aggregateStatus(List<Status> candidates) {
				List<Status> filteredCandidates = new ArrayList<>();
		for (Status candidate : candidates) {
			if (this.statusOrder.contains(candidate.getCode())) {
				filteredCandidates.add(candidate);
			}
		}
				if (filteredCandidates.isEmpty()) {
			return Status.UNKNOWN;
		}
				filteredCandidates.sort(new StatusComparator(this.statusOrder));
		return filteredCandidates.get(0);
	}

	
	private class StatusComparator implements Comparator<Status> {

		private final List<String> statusOrder;

		StatusComparator(List<String> statusOrder) {
			this.statusOrder = statusOrder;
		}

		@Override
		public int compare(Status s1, Status s2) {
			int i1 = this.statusOrder.indexOf(s1.getCode());
			int i2 = this.statusOrder.indexOf(s2.getCode());
			return (i1 < i2 ? -1 : (i1 == i2 ? s1.getCode().compareTo(s2.getCode()) : 1));
		}

	}

}

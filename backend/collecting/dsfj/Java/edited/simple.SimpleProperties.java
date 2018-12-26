

package org.springframework.boot.configurationsample.simple;

import java.beans.FeatureDescriptor;
import java.util.Comparator;

import org.springframework.boot.configurationsample.ConfigurationProperties;


@ConfigurationProperties(prefix = "simple")
public class SimpleProperties {

	
	private String theName = "boot";

		
	private boolean flag;

		private Comparator<?> comparator;

			private FeatureDescriptor featureDescriptor;

		@SuppressWarnings("unused")
	private Long counter;

		private Integer size;

	public String getTheName() {
		return this.theName;
	}

	@Deprecated
	public void setTheName(String name) {
		this.theName = name;
	}

	@Deprecated
	public boolean isFlag() {
		return this.flag;
	}

	public void setFlag(boolean flag) {
		this.flag = flag;
	}

	public Comparator<?> getComparator() {
		return this.comparator;
	}

	public void setComparator(Comparator<?> comparator) {
		this.comparator = comparator;
	}

	public FeatureDescriptor getFeatureDescriptor() {
		return this.featureDescriptor;
	}

	public void setCounter(Long counter) {
		this.counter = counter;
	}

	public Integer getSize() {
		return this.size;
	}

}

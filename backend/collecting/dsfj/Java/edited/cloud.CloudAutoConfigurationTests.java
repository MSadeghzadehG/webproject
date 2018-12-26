

package org.springframework.boot.autoconfigure.cloud;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.junit.Test;

import org.springframework.boot.autoconfigure.TestAutoConfigurationSorter;
import org.springframework.boot.autoconfigure.data.jpa.JpaRepositoriesAutoConfiguration;
import org.springframework.boot.autoconfigure.data.mongo.MongoRepositoriesAutoConfiguration;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.autoconfigure.mongo.MongoAutoConfiguration;
import org.springframework.core.type.classreading.CachingMetadataReaderFactory;

import static org.assertj.core.api.Assertions.assertThat;


public class CloudAutoConfigurationTests {

	@Test
	public void testOrder() {
		TestAutoConfigurationSorter sorter = new TestAutoConfigurationSorter(
				new CachingMetadataReaderFactory());
		Collection<String> classNames = new ArrayList<>();
		classNames.add(MongoAutoConfiguration.class.getName());
		classNames.add(DataSourceAutoConfiguration.class.getName());
		classNames.add(MongoRepositoriesAutoConfiguration.class.getName());
		classNames.add(JpaRepositoriesAutoConfiguration.class.getName());
		classNames.add(CloudAutoConfiguration.class.getName());
		List<String> ordered = sorter.getInPriorityOrder(classNames);
		assertThat(ordered.get(0)).isEqualTo(CloudAutoConfiguration.class.getName());
	}

}

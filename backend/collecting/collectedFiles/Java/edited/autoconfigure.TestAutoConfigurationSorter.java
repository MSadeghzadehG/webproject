

package org.springframework.boot.autoconfigure;

import java.util.Properties;

import org.springframework.core.type.classreading.MetadataReaderFactory;


public class TestAutoConfigurationSorter extends AutoConfigurationSorter {

	public TestAutoConfigurationSorter(MetadataReaderFactory metadataReaderFactory) {
		super(metadataReaderFactory,
				AutoConfigurationMetadataLoader.loadMetadata(new Properties()));
	}

}

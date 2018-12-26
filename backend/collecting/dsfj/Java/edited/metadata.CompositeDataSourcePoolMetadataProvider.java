

package org.springframework.boot.jdbc.metadata;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import javax.sql.DataSource;


public class CompositeDataSourcePoolMetadataProvider
		implements DataSourcePoolMetadataProvider {

	private final List<DataSourcePoolMetadataProvider> providers;

	
	public CompositeDataSourcePoolMetadataProvider(
			Collection<? extends DataSourcePoolMetadataProvider> providers) {
		this.providers = (providers == null ? Collections.emptyList()
				: Collections.unmodifiableList(new ArrayList<>(providers)));
	}

	@Override
	public DataSourcePoolMetadata getDataSourcePoolMetadata(DataSource dataSource) {
		for (DataSourcePoolMetadataProvider provider : this.providers) {
			DataSourcePoolMetadata metadata = provider
					.getDataSourcePoolMetadata(dataSource);
			if (metadata != null) {
				return metadata;
			}
		}
		return null;
	}

}

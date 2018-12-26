

package org.springframework.boot.autoconfigure.cache;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.springframework.boot.util.LambdaSafe;
import org.springframework.cache.CacheManager;


public class CacheManagerCustomizers {

	private final List<CacheManagerCustomizer<?>> customizers;

	public CacheManagerCustomizers(
			List<? extends CacheManagerCustomizer<?>> customizers) {
		this.customizers = (customizers != null ? new ArrayList<>(customizers)
				: Collections.emptyList());
	}

	
	@SuppressWarnings("unchecked")
	public <T extends CacheManager> T customize(T cacheManager) {
		LambdaSafe.callbacks(CacheManagerCustomizer.class, this.customizers, cacheManager)
				.withLogger(CacheManagerCustomizers.class)
				.invoke((customizer) -> customizer.customize(cacheManager));
		return cacheManager;
	}

}

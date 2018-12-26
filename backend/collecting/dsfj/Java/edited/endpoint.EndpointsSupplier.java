

package org.springframework.boot.actuate.endpoint;

import java.util.Collection;


@FunctionalInterface
public interface EndpointsSupplier<E extends ExposableEndpoint<?>> {

	
	Collection<E> getEndpoints();

}

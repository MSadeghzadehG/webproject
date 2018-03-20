

package org.springframework.boot.actuate.endpoint.web;

import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import org.springframework.boot.actuate.endpoint.EndpointsSupplier;
import org.springframework.util.Assert;


public class PathMappedEndpoints implements Iterable<PathMappedEndpoint> {

	private final String basePath;

	private final Map<String, PathMappedEndpoint> endpoints;

	
	public PathMappedEndpoints(String basePath, EndpointsSupplier<?> supplier) {
		Assert.notNull(supplier, "Supplier must not be null");
		this.basePath = (basePath == null ? "" : basePath);
		this.endpoints = getEndpoints(Collections.singleton(supplier));
	}

	
	public PathMappedEndpoints(String basePath,
			Collection<EndpointsSupplier<?>> suppliers) {
		Assert.notNull(suppliers, "Suppliers must not be null");
		this.basePath = (basePath == null ? "" : basePath);
		this.endpoints = getEndpoints(suppliers);
	}

	private Map<String, PathMappedEndpoint> getEndpoints(
			Collection<EndpointsSupplier<?>> suppliers) {
		Map<String, PathMappedEndpoint> endpoints = new LinkedHashMap<>();
		suppliers.forEach((supplier) -> {
			supplier.getEndpoints().forEach((endpoint) -> {
				if (endpoint instanceof PathMappedEndpoint) {
					endpoints.put(endpoint.getId(), (PathMappedEndpoint) endpoint);
				}
			});
		});
		return Collections.unmodifiableMap(endpoints);
	}

	
	public String getBasePath() {
		return this.basePath;
	}

	
	public String getRootPath(String endpointId) {
		PathMappedEndpoint endpoint = getEndpoint(endpointId);
		return (endpoint == null ? null : endpoint.getRootPath());
	}

	
	public String getPath(String endpointId) {
		return getPath(getEndpoint(endpointId));
	}

	
	public Collection<String> getAllRootPaths() {
		return asList(stream().map(PathMappedEndpoint::getRootPath));
	}

	
	public Collection<String> getAllPaths() {
		return asList(stream().map(this::getPath));
	}

	
	public PathMappedEndpoint getEndpoint(String endpointId) {
		return this.endpoints.get(endpointId);
	}

	
	public Stream<PathMappedEndpoint> stream() {
		return this.endpoints.values().stream();
	}

	@Override
	public Iterator<PathMappedEndpoint> iterator() {
		return this.endpoints.values().iterator();
	}

	private String getPath(PathMappedEndpoint endpoint) {
		return (endpoint == null ? null : this.basePath + "/" + endpoint.getRootPath());
	}

	private <T> List<T> asList(Stream<T> stream) {
		return stream.collect(Collectors.collectingAndThen(Collectors.toList(),
				Collections::unmodifiableList));
	}

}

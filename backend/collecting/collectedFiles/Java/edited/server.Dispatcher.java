

package org.springframework.boot.devtools.remote.server;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.springframework.core.annotation.AnnotationAwareOrderComparator;
import org.springframework.http.HttpStatus;
import org.springframework.http.server.ServerHttpRequest;
import org.springframework.http.server.ServerHttpResponse;
import org.springframework.util.Assert;


public class Dispatcher {

	private final AccessManager accessManager;

	private final List<HandlerMapper> mappers;

	public Dispatcher(AccessManager accessManager, Collection<HandlerMapper> mappers) {
		Assert.notNull(accessManager, "AccessManager must not be null");
		Assert.notNull(mappers, "Mappers must not be null");
		this.accessManager = accessManager;
		this.mappers = new ArrayList<>(mappers);
		AnnotationAwareOrderComparator.sort(this.mappers);
	}

	
	public boolean handle(ServerHttpRequest request, ServerHttpResponse response)
			throws IOException {
		for (HandlerMapper mapper : this.mappers) {
			Handler handler = mapper.getHandler(request);
			if (handler != null) {
				handle(handler, request, response);
				return true;
			}
		}
		return false;
	}

	private void handle(Handler handler, ServerHttpRequest request,
			ServerHttpResponse response) throws IOException {
		if (!this.accessManager.isAllowed(request)) {
			response.setStatusCode(HttpStatus.FORBIDDEN);
			return;
		}
		handler.handle(request, response);
	}

}

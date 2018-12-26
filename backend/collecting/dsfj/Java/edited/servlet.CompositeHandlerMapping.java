

package org.springframework.boot.actuate.autoconfigure.web.servlet;

import java.util.ArrayList;
import java.util.List;

import javax.servlet.http.HttpServletRequest;

import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.core.annotation.AnnotationAwareOrderComparator;
import org.springframework.web.servlet.HandlerExceptionResolver;
import org.springframework.web.servlet.HandlerExecutionChain;
import org.springframework.web.servlet.HandlerMapping;


class CompositeHandlerMapping implements HandlerMapping {

	@Autowired
	private ListableBeanFactory beanFactory;

	private List<HandlerMapping> mappings;

	@Override
	public HandlerExecutionChain getHandler(HttpServletRequest request) throws Exception {
		if (this.mappings == null) {
			this.mappings = extractMappings();
		}
		for (HandlerMapping mapping : this.mappings) {
			HandlerExecutionChain handler = mapping.getHandler(request);
			if (handler != null) {
				return handler;
			}
		}
		return null;
	}

	private List<HandlerMapping> extractMappings() {
		List<HandlerMapping> list = new ArrayList<>();
		list.addAll(this.beanFactory.getBeansOfType(HandlerMapping.class).values());
		list.remove(this);
		AnnotationAwareOrderComparator.sort(list);
		return list;
	}

}

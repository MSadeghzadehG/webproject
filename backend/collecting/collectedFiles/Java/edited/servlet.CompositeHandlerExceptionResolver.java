

package org.springframework.boot.actuate.autoconfigure.web.servlet;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.core.annotation.AnnotationAwareOrderComparator;
import org.springframework.web.servlet.HandlerExceptionResolver;
import org.springframework.web.servlet.ModelAndView;
import org.springframework.web.servlet.mvc.support.DefaultHandlerExceptionResolver;


class CompositeHandlerExceptionResolver implements HandlerExceptionResolver {

	@Autowired
	private ListableBeanFactory beanFactory;

	private List<HandlerExceptionResolver> resolvers;

	@Override
	public ModelAndView resolveException(HttpServletRequest request,
			HttpServletResponse response, Object handler, Exception ex) {
		if (this.resolvers == null) {
			this.resolvers = extractResolvers();
		}
		return this.resolvers.stream().map(
				(resolver) -> resolver.resolveException(request, response, handler, ex))
				.filter(Objects::nonNull).findFirst().orElse(null);
	}

	private List<HandlerExceptionResolver> extractResolvers() {
		List<HandlerExceptionResolver> list = new ArrayList<>();
		list.addAll(
				this.beanFactory.getBeansOfType(HandlerExceptionResolver.class).values());
		list.remove(this);
		AnnotationAwareOrderComparator.sort(list);
		if (list.isEmpty()) {
			list.add(new DefaultHandlerExceptionResolver());
		}
		return list;
	}

}



package org.springframework.boot.actuate.autoconfigure.web.servlet;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.core.annotation.AnnotationAwareOrderComparator;
import org.springframework.web.servlet.HandlerAdapter;
import org.springframework.web.servlet.ModelAndView;


class CompositeHandlerAdapter implements HandlerAdapter {

	private final ListableBeanFactory beanFactory;

	private List<HandlerAdapter> adapters;

	CompositeHandlerAdapter(ListableBeanFactory beanFactory) {
		this.beanFactory = beanFactory;
	}

	@Override
	public boolean supports(Object handler) {
		return getAdapter(handler).isPresent();
	}

	@Override
	public ModelAndView handle(HttpServletRequest request, HttpServletResponse response,
			Object handler) throws Exception {
		Optional<HandlerAdapter> adapter = getAdapter(handler);
		if (adapter.isPresent()) {
			return adapter.get().handle(request, response, handler);
		}
		return null;
	}

	@Override
	public long getLastModified(HttpServletRequest request, Object handler) {
		Optional<HandlerAdapter> adapter = getAdapter(handler);
		if (adapter.isPresent()) {
			return adapter.get().getLastModified(request, handler);
		}
		return 0;
	}

	private Optional<HandlerAdapter> getAdapter(Object handler) {
		if (this.adapters == null) {
			this.adapters = extractAdapters();
		}
		return this.adapters.stream().filter((a) -> a.supports(handler)).findFirst();
	}

	private List<HandlerAdapter> extractAdapters() {
		List<HandlerAdapter> list = new ArrayList<>();
		list.addAll(this.beanFactory.getBeansOfType(HandlerAdapter.class).values());
		list.remove(this);
		AnnotationAwareOrderComparator.sort(list);
		return list;
	}

}

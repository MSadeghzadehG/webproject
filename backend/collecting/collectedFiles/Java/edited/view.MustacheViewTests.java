

package org.springframework.boot.web.reactive.result.view;

import java.nio.charset.StandardCharsets;
import java.util.Collections;

import com.samskivert.mustache.Mustache;
import org.junit.Before;
import org.junit.Test;

import org.springframework.context.support.GenericApplicationContext;
import org.springframework.http.MediaType;
import org.springframework.mock.http.server.reactive.MockServerHttpRequest;
import org.springframework.mock.web.server.MockServerWebExchange;

import static org.assertj.core.api.Assertions.assertThat;


public class MustacheViewTests {

	private final String templateUrl = "classpath:/"
			+ getClass().getPackage().getName().replace(".", "/") + "/template.html";

	private GenericApplicationContext context = new GenericApplicationContext();

	private MockServerWebExchange exchange;

	@Before
	public void init() {
		this.context.refresh();
	}

	@Test
	public void viewResolvesHandlebars() {
		this.exchange = MockServerWebExchange
				.from(MockServerHttpRequest.get("/test").build());
		MustacheView view = new MustacheView();
		view.setCompiler(Mustache.compiler());
		view.setUrl(this.templateUrl);
		view.setCharset(StandardCharsets.UTF_8.displayName());
		view.setApplicationContext(this.context);
		view.render(Collections.singletonMap("World", "Spring"), MediaType.TEXT_HTML,
				this.exchange).block();
		assertThat(this.exchange.getResponse().getBodyAsString().block())
				.isEqualTo("Hello Spring");
	}

}

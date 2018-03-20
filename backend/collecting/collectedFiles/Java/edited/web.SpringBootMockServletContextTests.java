

package org.springframework.boot.test.mock.web;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLDecoder;

import javax.servlet.ServletContext;

import org.junit.Test;
import org.junit.runner.RunWith;

import org.springframework.boot.test.context.SpringBootContextLoader;
import org.springframework.context.annotation.Configuration;
import org.springframework.test.annotation.DirtiesContext;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.context.web.WebAppConfiguration;
import org.springframework.web.context.ServletContextAware;

import static org.assertj.core.api.Assertions.assertThat;
import static org.hamcrest.Matchers.nullValue;


@RunWith(SpringRunner.class)
@DirtiesContext
@ContextConfiguration(loader = SpringBootContextLoader.class)
@WebAppConfiguration("src/test/webapp")
public class SpringBootMockServletContextTests implements ServletContextAware {

	private ServletContext servletContext;

	@Override
	public void setServletContext(ServletContext servletContext) {
		this.servletContext = servletContext;
	}

	@Test
	public void getResourceLocation() throws Exception {
		testResource("/inwebapp", "src/test/webapp");
		testResource("/inmetainfresources", "/META-INF/resources");
		testResource("/inresources", "/resources");
		testResource("/instatic", "/static");
		testResource("/inpublic", "/public");
	}

	private void testResource(String path, String expectedLocation)
			throws MalformedURLException {
		URL resource = this.servletContext.getResource(path);
		assertThat(resource).isNotNull();
		assertThat(resource.getPath()).contains(expectedLocation);
	}

		@Test
	public void getRootUrlExistsAndIsEmpty() throws Exception {
		SpringBootMockServletContext context = new SpringBootMockServletContext(
				"src/test/doesntexist") {
			@Override
			protected String getResourceLocation(String path) {
								return getResourceBasePathLocation(path);
			}
		};
		URL resource = context.getResource("/");
		assertThat(resource).isNotEqualTo(nullValue());
		File file = new File(URLDecoder.decode(resource.getPath(), "UTF-8"));
		assertThat(file).exists().isDirectory();
		String[] contents = file
				.list((dir, name) -> !(".".equals(name) || "..".equals(name)));
		assertThat(contents).isNotEqualTo(nullValue());
		assertThat(contents.length).isEqualTo(0);
	}

	@Configuration
	static class Config {

	}

}

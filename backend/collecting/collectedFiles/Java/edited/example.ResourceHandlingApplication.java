

package com.example;

import java.io.IOException;
import java.net.URL;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.builder.SpringApplicationBuilder;
import org.springframework.boot.web.context.WebServerPortFileWriter;
import org.springframework.boot.web.servlet.ServletRegistrationBean;
import org.springframework.context.annotation.Bean;


@SpringBootApplication
public class ResourceHandlingApplication {

	public static void main(String[] args) {
		new SpringApplicationBuilder(ResourceHandlingApplication.class)
				.properties("server.port:0")
				.listeners(new WebServerPortFileWriter("target/server.port")).run(args);
	}

	@Bean
	public ServletRegistrationBean<?> resourceServletRegistration() {
		ServletRegistrationBean<?> registration = new ServletRegistrationBean<HttpServlet>(
				new HttpServlet() {

					@Override
					protected void doGet(HttpServletRequest req, HttpServletResponse resp)
							throws ServletException, IOException {
						URL resource = getServletContext()
								.getResource(req.getQueryString());
						if (resource == null) {
							resp.sendError(404);
						}
						else {
							resp.getWriter().println(resource);
							resp.getWriter().flush();
						}
					}

				});
		registration.addUrlMappings("/servletContext");
		return registration;
	}

}

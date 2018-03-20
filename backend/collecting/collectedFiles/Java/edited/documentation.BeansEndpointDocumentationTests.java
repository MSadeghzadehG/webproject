

package org.springframework.boot.actuate.autoconfigure.endpoint.web.documentation;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.junit.Test;

import org.springframework.boot.actuate.beans.BeansEndpoint;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Import;
import org.springframework.restdocs.payload.FieldDescriptor;
import org.springframework.restdocs.payload.ResponseFieldsSnippet;
import org.springframework.util.CollectionUtils;

import static org.springframework.restdocs.mockmvc.MockMvcRestDocumentation.document;
import static org.springframework.restdocs.operation.preprocess.Preprocessors.preprocessResponse;
import static org.springframework.restdocs.payload.PayloadDocumentation.fieldWithPath;
import static org.springframework.restdocs.payload.PayloadDocumentation.responseFields;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;


public class BeansEndpointDocumentationTests extends MockMvcEndpointDocumentationTests {

	@Test
	public void beans() throws Exception {
		List<FieldDescriptor> beanFields = Arrays.asList(
				fieldWithPath("aliases").description("Names of any aliases."),
				fieldWithPath("scope").description("Scope of the bean."),
				fieldWithPath("type").description("Fully qualified type of the bean."),
				fieldWithPath("resource")
						.description("Resource in which the bean was defined, if any.")
						.optional(),
				fieldWithPath("dependencies").description("Names of any dependencies."));
		ResponseFieldsSnippet responseFields = responseFields(
				fieldWithPath("contexts")
						.description("Application contexts keyed by id."),
				parentIdField(),
				fieldWithPath("contexts.*.beans")
						.description("Beans in the application context keyed by name."))
								.andWithPrefix("contexts.*.beans.*.", beanFields);
		this.mockMvc.perform(get("/actuator/beans")).andExpect(status().isOk())
				.andDo(document("beans",
						preprocessResponse(limit(this::isIndependentBean, "contexts",
								getApplicationContext().getId(), "beans")),
						responseFields));
	}

	private boolean isIndependentBean(Entry<String, Map<String, Object>> bean) {
		return CollectionUtils.isEmpty((Collection<?>) bean.getValue().get("aliases"))
				&& CollectionUtils
						.isEmpty((Collection<?>) bean.getValue().get("dependencies"));
	}

	@Configuration
	@Import(BaseDocumentationConfiguration.class)
	static class TestConfiguration {

		@Bean
		public BeansEndpoint beansEndpoint(ConfigurableApplicationContext context) {
			return new BeansEndpoint(context);
		}

	}

}

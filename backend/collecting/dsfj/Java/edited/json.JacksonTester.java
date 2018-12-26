

package org.springframework.boot.test.json;

import java.io.IOException;
import java.io.InputStream;
import java.io.Reader;

import com.fasterxml.jackson.databind.JavaType;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.ObjectReader;
import com.fasterxml.jackson.databind.ObjectWriter;

import org.springframework.beans.factory.ObjectFactory;
import org.springframework.core.ResolvableType;
import org.springframework.util.Assert;


public class JacksonTester<T> extends AbstractJsonMarshalTester<T> {

	private final ObjectMapper objectMapper;

	private Class<?> view;

	
	protected JacksonTester(ObjectMapper objectMapper) {
		Assert.notNull(objectMapper, "ObjectMapper must not be null");
		this.objectMapper = objectMapper;
	}

	
	public JacksonTester(Class<?> resourceLoadClass, ResolvableType type,
			ObjectMapper objectMapper) {
		this(resourceLoadClass, type, objectMapper, null);
	}

	public JacksonTester(Class<?> resourceLoadClass, ResolvableType type,
			ObjectMapper objectMapper, Class<?> view) {
		super(resourceLoadClass, type);
		Assert.notNull(objectMapper, "ObjectMapper must not be null");
		this.objectMapper = objectMapper;
		this.view = view;
	}

	@Override
	protected T readObject(InputStream inputStream, ResolvableType type)
			throws IOException {
		return getObjectReader(type).readValue(inputStream);
	}

	@Override
	protected T readObject(Reader reader, ResolvableType type) throws IOException {
		return getObjectReader(type).readValue(reader);
	}

	private ObjectReader getObjectReader(ResolvableType type) {
		ObjectReader objectReader = this.objectMapper.readerFor(getType(type));
		if (this.view != null) {
			return objectReader.withView(this.view);
		}
		return objectReader;
	}

	@Override
	protected String writeObject(T value, ResolvableType type) throws IOException {
		return getObjectWriter(type).writeValueAsString(value);
	}

	private ObjectWriter getObjectWriter(ResolvableType type) {
		ObjectWriter objectWriter = this.objectMapper.writerFor(getType(type));
		if (this.view != null) {
			return objectWriter.withView(this.view);
		}
		return objectWriter;
	}

	private JavaType getType(ResolvableType type) {
		return this.objectMapper.constructType(type.getType());
	}

	
	public static void initFields(Object testInstance, ObjectMapper objectMapper) {
		new JacksonFieldInitializer().initFields(testInstance, objectMapper);
	}

	
	public static void initFields(Object testInstance,
			ObjectFactory<ObjectMapper> objectMapperFactory) {
		new JacksonFieldInitializer().initFields(testInstance, objectMapperFactory);
	}

	
	public JacksonTester<T> forView(Class<?> view) {
		return new JacksonTester<>(this.getResourceLoadClass(), this.getType(),
				this.objectMapper, view);
	}

	
	private static class JacksonFieldInitializer extends FieldInitializer<ObjectMapper> {

		protected JacksonFieldInitializer() {
			super(JacksonTester.class);
		}

		@Override
		protected AbstractJsonMarshalTester<Object> createTester(
				Class<?> resourceLoadClass, ResolvableType type,
				ObjectMapper marshaller) {
			return new JacksonTester<>(resourceLoadClass, type, marshaller);
		}

	}

}

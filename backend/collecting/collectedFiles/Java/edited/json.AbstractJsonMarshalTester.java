

package org.springframework.boot.test.json;

import java.io.BufferedReader;
import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.lang.reflect.Field;

import org.assertj.core.api.Assertions;

import org.springframework.beans.factory.ObjectFactory;
import org.springframework.core.ResolvableType;
import org.springframework.core.io.ByteArrayResource;
import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.FileSystemResource;
import org.springframework.core.io.InputStreamResource;
import org.springframework.core.io.Resource;
import org.springframework.util.Assert;
import org.springframework.util.ReflectionUtils;


public abstract class AbstractJsonMarshalTester<T> {

	private Class<?> resourceLoadClass;

	private ResolvableType type;

	
	protected AbstractJsonMarshalTester() {
	}

	
	public AbstractJsonMarshalTester(Class<?> resourceLoadClass, ResolvableType type) {
		Assert.notNull(resourceLoadClass, "ResourceLoadClass must not be null");
		Assert.notNull(type, "Type must not be null");
		initialize(resourceLoadClass, type);
	}

	
	protected final void initialize(Class<?> resourceLoadClass, ResolvableType type) {
		if (this.resourceLoadClass == null && this.type == null) {
			this.resourceLoadClass = resourceLoadClass;
			this.type = type;
		}
	}

	
	protected final ResolvableType getType() {
		return this.type;
	}

	
	protected final Class<?> getResourceLoadClass() {
		return this.resourceLoadClass;
	}

	
	public JsonContent<T> write(T value) throws IOException {
		verify();
		Assert.notNull(value, "Value must not be null");
		String json = writeObject(value, this.type);
		return new JsonContent<>(this.resourceLoadClass, this.type, json);
	}

	
	public T parseObject(byte[] jsonBytes) throws IOException {
		verify();
		return parse(jsonBytes).getObject();
	}

	
	public ObjectContent<T> parse(byte[] jsonBytes) throws IOException {
		verify();
		Assert.notNull(jsonBytes, "JsonBytes must not be null");
		return read(new ByteArrayResource(jsonBytes));
	}

	
	public T parseObject(String jsonString) throws IOException {
		verify();
		return parse(jsonString).getObject();
	}

	
	public ObjectContent<T> parse(String jsonString) throws IOException {
		verify();
		Assert.notNull(jsonString, "JsonString must not be null");
		return read(new StringReader(jsonString));
	}

	
	public T readObject(String resourcePath) throws IOException {
		verify();
		return read(resourcePath).getObject();
	}

	
	public ObjectContent<T> read(String resourcePath) throws IOException {
		verify();
		Assert.notNull(resourcePath, "ResourcePath must not be null");
		return read(new ClassPathResource(resourcePath, this.resourceLoadClass));
	}

	
	public T readObject(File file) throws IOException {
		verify();
		return read(file).getObject();
	}

	
	public ObjectContent<T> read(File file) throws IOException {
		verify();
		Assert.notNull(file, "File must not be null");
		return read(new FileSystemResource(file));
	}

	
	public T readObject(InputStream inputStream) throws IOException {
		verify();
		return read(inputStream).getObject();
	}

	
	public ObjectContent<T> read(InputStream inputStream) throws IOException {
		verify();
		Assert.notNull(inputStream, "InputStream must not be null");
		return read(new InputStreamResource(inputStream));
	}

	
	public T readObject(Resource resource) throws IOException {
		verify();
		return read(resource).getObject();
	}

	
	public ObjectContent<T> read(Resource resource) throws IOException {
		verify();
		Assert.notNull(resource, "Resource must not be null");
		InputStream inputStream = resource.getInputStream();
		T object = readObject(inputStream, this.type);
		closeQuietly(inputStream);
		return new ObjectContent<>(this.type, object);
	}

	
	public T readObject(Reader reader) throws IOException {
		verify();
		return read(reader).getObject();
	}

	
	public ObjectContent<T> read(Reader reader) throws IOException {
		verify();
		Assert.notNull(reader, "Reader must not be null");
		T object = readObject(reader, this.type);
		closeQuietly(reader);
		return new ObjectContent<>(this.type, object);
	}

	private void closeQuietly(Closeable closeable) {
		try {
			closeable.close();
		}
		catch (IOException ex) {
		}
	}

	private void verify() {
		Assert.state(this.resourceLoadClass != null,
				"Uninitialized JsonMarshalTester (ResourceLoadClass is null)");
		Assert.state(this.type != null, "Uninitialized JsonMarshalTester (Type is null)");
	}

	
	protected abstract String writeObject(T value, ResolvableType type)
			throws IOException;

	
	protected T readObject(InputStream inputStream, ResolvableType type)
			throws IOException {
		BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
		return readObject(reader, type);
	}

	
	protected abstract T readObject(Reader reader, ResolvableType type)
			throws IOException;

	
	protected abstract static class FieldInitializer<M> {

		private final Class<?> testerClass;

		@SuppressWarnings("rawtypes")
		protected FieldInitializer(
				Class<? extends AbstractJsonMarshalTester> testerClass) {
			Assert.notNull(testerClass, "TesterClass must not be null");
			this.testerClass = testerClass;
		}

		public void initFields(Object testInstance, M marshaller) {
			Assert.notNull(testInstance, "TestInstance must not be null");
			Assert.notNull(marshaller, "Marshaller must not be null");
			initFields(testInstance, () -> marshaller);
		}

		public void initFields(Object testInstance, final ObjectFactory<M> marshaller) {
			Assert.notNull(testInstance, "TestInstance must not be null");
			Assert.notNull(marshaller, "Marshaller must not be null");
			ReflectionUtils.doWithFields(testInstance.getClass(),
					(field) -> doWithField(field, testInstance, marshaller));
		}

		protected void doWithField(Field field, Object test,
				ObjectFactory<M> marshaller) {
			if (this.testerClass.isAssignableFrom(field.getType())) {
				ReflectionUtils.makeAccessible(field);
				Object existingValue = ReflectionUtils.getField(field, test);
				if (existingValue == null) {
					setupField(field, test, marshaller);
				}
			}
		}

		private void setupField(Field field, Object test, ObjectFactory<M> marshaller) {
			ResolvableType type = ResolvableType.forField(field).getGeneric();
			ReflectionUtils.setField(field, test,
					createTester(test.getClass(), type, marshaller.getObject()));
		}

		protected abstract AbstractJsonMarshalTester<Object> createTester(
				Class<?> resourceLoadClass, ResolvableType type, M marshaller);

	}

}

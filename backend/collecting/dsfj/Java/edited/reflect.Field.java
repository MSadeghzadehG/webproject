

package com.badlogic.gdx.utils.reflect;

import java.lang.reflect.GenericArrayType;
import java.lang.reflect.Modifier;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;


public final class Field {

	private final java.lang.reflect.Field field;

	Field (java.lang.reflect.Field field) {
		this.field = field;
	}

	
	public String getName () {
		return field.getName();
	}

	
	public Class getType () {
		return field.getType();
	}

	
	public Class getDeclaringClass () {
		return field.getDeclaringClass();
	}

	public boolean isAccessible () {
		return field.isAccessible();
	}

	public void setAccessible (boolean accessible) {
		field.setAccessible(accessible);
	}

	
	public boolean isDefaultAccess () {
		return !isPrivate() && !isProtected() && !isPublic();
	}

	
	public boolean isFinal () {
		return Modifier.isFinal(field.getModifiers());
	}

	
	public boolean isPrivate () {
		return Modifier.isPrivate(field.getModifiers());
	}

	
	public boolean isProtected () {
		return Modifier.isProtected(field.getModifiers());
	}

	
	public boolean isPublic () {
		return Modifier.isPublic(field.getModifiers());
	}

	
	public boolean isStatic () {
		return Modifier.isStatic(field.getModifiers());
	}

	
	public boolean isTransient () {
		return Modifier.isTransient(field.getModifiers());
	}

	
	public boolean isVolatile () {
		return Modifier.isVolatile(field.getModifiers());
	}

	
	public boolean isSynthetic () {
		return field.isSynthetic();
	}

	
	public Class getElementType (int index) {
		Type genericType = field.getGenericType();
		if (genericType instanceof ParameterizedType) {
			Type[] actualTypes = ((ParameterizedType)genericType).getActualTypeArguments();
			if (actualTypes.length - 1 >= index) {
				Type actualType = actualTypes[index];
				if (actualType instanceof Class)
					return (Class)actualType;
				else if (actualType instanceof ParameterizedType)
					return (Class)((ParameterizedType)actualType).getRawType();
				else if (actualType instanceof GenericArrayType) {
					Type componentType = ((GenericArrayType)actualType).getGenericComponentType();
					if (componentType instanceof Class) return ArrayReflection.newInstance((Class)componentType, 0).getClass();
				}
			}
		}
		return null;
	}

	
	public boolean isAnnotationPresent (Class<? extends java.lang.annotation.Annotation> annotationType) {
		return field.isAnnotationPresent(annotationType);
	}

	
	public Annotation[] getDeclaredAnnotations () {
		java.lang.annotation.Annotation[] annotations = field.getDeclaredAnnotations();
		Annotation[] result = new Annotation[annotations.length];
		for (int i = 0; i < annotations.length; i++) {
			result[i] = new Annotation(annotations[i]);
		}
		return result;
	}

	
	public Annotation getDeclaredAnnotation (Class<? extends java.lang.annotation.Annotation> annotationType) {
		java.lang.annotation.Annotation[] annotations = field.getDeclaredAnnotations();
		if (annotations == null) {
			return null;
		}
		for (java.lang.annotation.Annotation annotation : annotations) {
			if (annotation.annotationType().equals(annotationType)) {
				return new Annotation(annotation);
			}
		}
		return null;
	}

	
	public Object get (Object obj) throws ReflectionException {
		try {
			return field.get(obj);
		} catch (IllegalArgumentException e) {
			throw new ReflectionException("Object is not an instance of " + getDeclaringClass(), e);
		} catch (IllegalAccessException e) {
			throw new ReflectionException("Illegal access to field: " + getName(), e);
		}
	}

	
	public void set (Object obj, Object value) throws ReflectionException {
		try {
			field.set(obj, value);
		} catch (IllegalArgumentException e) {
			throw new ReflectionException("Argument not valid for field: " + getName(), e);
		} catch (IllegalAccessException e) {
			throw new ReflectionException("Illegal access to field: " + getName(), e);
		}
	}

}

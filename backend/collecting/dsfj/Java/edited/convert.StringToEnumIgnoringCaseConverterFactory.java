

package org.springframework.boot.convert;

import java.util.EnumSet;
import java.util.Set;

import org.springframework.core.convert.converter.Converter;
import org.springframework.core.convert.converter.ConverterFactory;
import org.springframework.util.Assert;


@SuppressWarnings({ "unchecked", "rawtypes" })
final class StringToEnumIgnoringCaseConverterFactory
		implements ConverterFactory<String, Enum> {

	@Override
	public <T extends Enum> Converter<String, T> getConverter(Class<T> targetType) {
		Class<?> enumType = targetType;
		while (enumType != null && !enumType.isEnum()) {
			enumType = enumType.getSuperclass();
		}
		Assert.notNull(enumType,
				"The target type " + targetType.getName() + " does not refer to an enum");
		return new StringToEnum(enumType);
	}

	private class StringToEnum<T extends Enum> implements Converter<String, T> {

		private final Class<T> enumType;

		StringToEnum(Class<T> enumType) {
			this.enumType = enumType;
		}

		@Override
		public T convert(String source) {
			if (source.isEmpty()) {
				return null;
			}
			source = source.trim();
			try {
				return (T) Enum.valueOf(this.enumType, source);
			}
			catch (Exception ex) {
				return findEnum(source);
			}
		}

		private T findEnum(String source) {
			String name = getLettersAndDigits(source);
			for (T candidate : (Set<T>) EnumSet.allOf(this.enumType)) {
				if (getLettersAndDigits(candidate.name()).equals(name)) {
					return candidate;
				}
			}
			throw new IllegalArgumentException("No enum constant "
					+ this.enumType.getCanonicalName() + "." + source);
		}

		private String getLettersAndDigits(String name) {
			StringBuilder canonicalName = new StringBuilder(name.length());
			name.chars().map((c) -> (char) c).filter(Character::isLetterOrDigit)
					.map(Character::toLowerCase).forEach(canonicalName::append);
			return canonicalName.toString();
		}

	}

}

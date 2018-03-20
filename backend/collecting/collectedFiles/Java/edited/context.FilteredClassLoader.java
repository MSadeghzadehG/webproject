

package org.springframework.boot.test.context;

import java.net.URL;
import java.net.URLClassLoader;
import java.util.function.Predicate;


public class FilteredClassLoader extends URLClassLoader {

	private final Predicate<String>[] filters;

	
	public FilteredClassLoader(Class<?>... hiddenClasses) {
		this(ClassFilter.of(hiddenClasses));
	}

	
	public FilteredClassLoader(String... hiddenPackages) {
		this(PackageFilter.of(hiddenPackages));
	}

	
	@SafeVarargs
	public FilteredClassLoader(Predicate<String>... filters) {
		super(new URL[0], FilteredClassLoader.class.getClassLoader());
		this.filters = filters;
	}

	@Override
	protected Class<?> loadClass(String name, boolean resolve)
			throws ClassNotFoundException {
		for (Predicate<String> filter : this.filters) {
			if (filter.test(name)) {
				throw new ClassNotFoundException();
			}
		}
		return super.loadClass(name, resolve);
	}

	
	public static final class ClassFilter implements Predicate<String> {

		private Class<?>[] hiddenClasses;

		private ClassFilter(Class<?>[] hiddenClasses) {
			this.hiddenClasses = hiddenClasses;
		}

		@Override
		public boolean test(String className) {
			for (Class<?> hiddenClass : this.hiddenClasses) {
				if (className.equals(hiddenClass.getName())) {
					return true;
				}
			}
			return false;
		}

		public static ClassFilter of(Class<?>... hiddenClasses) {
			return new ClassFilter(hiddenClasses);
		}

	}

	
	public static final class PackageFilter implements Predicate<String> {

		private final String[] hiddenPackages;

		private PackageFilter(String[] hiddenPackages) {
			this.hiddenPackages = hiddenPackages;
		}

		@Override
		public boolean test(String className) {
			for (String hiddenPackage : this.hiddenPackages) {
				if (className.startsWith(hiddenPackage)) {
					return true;
				}
			}
			return false;
		}

		public static PackageFilter of(String... hiddenPackages) {
			return new PackageFilter(hiddenPackages);
		}

	}
}



package com.badlogic.gdx.utils;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.MissingResourceException;

import com.badlogic.gdx.files.FileHandle;


public class I18NBundle {

	private static final String DEFAULT_ENCODING = "UTF-8";

		private static final Locale ROOT_LOCALE = new Locale("", "", "");

	private static boolean simpleFormatter = false;
	private static boolean exceptionOnMissingKey = true;

	
	private I18NBundle parent;

	
	private Locale locale;

	
	private ObjectMap<String, String> properties;

	
	private TextFormatter formatter;

	
	public static boolean getSimpleFormatter () {
		return simpleFormatter;
	}

	
	public static void setSimpleFormatter (boolean enabled) {
		simpleFormatter = enabled;
	}

	
	public static boolean getExceptionOnMissingKey () {
		return exceptionOnMissingKey;
	}

	
	public static void setExceptionOnMissingKey (boolean enabled) {
		exceptionOnMissingKey = enabled;
	}

	
	public static I18NBundle createBundle (FileHandle baseFileHandle) {
		return createBundleImpl(baseFileHandle, Locale.getDefault(), DEFAULT_ENCODING);
	}

	
	public static I18NBundle createBundle (FileHandle baseFileHandle, Locale locale) {
		return createBundleImpl(baseFileHandle, locale, DEFAULT_ENCODING);
	}

	
	public static I18NBundle createBundle (FileHandle baseFileHandle, String encoding) {
		return createBundleImpl(baseFileHandle, Locale.getDefault(), encoding);
	}

	
	public static I18NBundle createBundle (FileHandle baseFileHandle, Locale locale, String encoding) {
		return createBundleImpl(baseFileHandle, locale, encoding);
	}

	private static I18NBundle createBundleImpl (FileHandle baseFileHandle, Locale locale, String encoding) {
		if (baseFileHandle == null || locale == null || encoding == null) throw new NullPointerException();

		I18NBundle bundle = null;
		I18NBundle baseBundle = null;
		Locale targetLocale = locale;
		do {
						List<Locale> candidateLocales = getCandidateLocales(targetLocale);

						bundle = loadBundleChain(baseFileHandle, encoding, candidateLocales, 0, baseBundle);

						if (bundle != null) {
				Locale bundleLocale = bundle.getLocale(); 				boolean isBaseBundle = bundleLocale.equals(ROOT_LOCALE);

				if (!isBaseBundle || bundleLocale.equals(locale)) {
										break;
				}
				if (candidateLocales.size() == 1 && bundleLocale.equals(candidateLocales.get(0))) {
										break;
				}
				if (isBaseBundle && baseBundle == null) {
										baseBundle = bundle;
				}
			}

						targetLocale = getFallbackLocale(targetLocale);

		} while (targetLocale != null);

		if (bundle == null) {
			if (baseBundle == null) {
								throw new MissingResourceException("Can't find bundle for base file handle " + baseFileHandle.path() + ", locale "
					+ locale, baseFileHandle + "_" + locale, "");
			}
						bundle = baseBundle;
		}

		return bundle;
	}

	
	private static List<Locale> getCandidateLocales (Locale locale) {
		String language = locale.getLanguage();
		String country = locale.getCountry();
		String variant = locale.getVariant();

		List<Locale> locales = new ArrayList<Locale>(4);
		if (variant.length() > 0) {
			locales.add(locale);
		}
		if (country.length() > 0) {
			locales.add((locales.size() == 0) ? locale : new Locale(language, country));
		}
		if (language.length() > 0) {
			locales.add((locales.size() == 0) ? locale : new Locale(language));
		}
		locales.add(ROOT_LOCALE);
		return locales;
	}

	
	private static Locale getFallbackLocale (Locale locale) {
		Locale defaultLocale = Locale.getDefault();
		return locale.equals(defaultLocale) ? null : defaultLocale;
	}

	private static I18NBundle loadBundleChain (FileHandle baseFileHandle, String encoding, List<Locale> candidateLocales,
		int candidateIndex, I18NBundle baseBundle) {
		Locale targetLocale = candidateLocales.get(candidateIndex);
		I18NBundle parent = null;
		if (candidateIndex != candidateLocales.size() - 1) {
						parent = loadBundleChain(baseFileHandle, encoding, candidateLocales, candidateIndex + 1, baseBundle);
		} else if (baseBundle != null && targetLocale.equals(ROOT_LOCALE)) {
			return baseBundle;
		}

				I18NBundle bundle = loadBundle(baseFileHandle, encoding, targetLocale);
		if (bundle != null) {
			bundle.parent = parent;
			return bundle;
		}

		return parent;
	}

		private static I18NBundle loadBundle (FileHandle baseFileHandle, String encoding, Locale targetLocale) {
		I18NBundle bundle = null;
		Reader reader = null;
		try {
			FileHandle fileHandle = toFileHandle(baseFileHandle, targetLocale);
			if (checkFileExistence(fileHandle)) {
								bundle = new I18NBundle();

								reader = fileHandle.reader(encoding);
				bundle.load(reader);
			}
		} catch (IOException e) {
			throw new GdxRuntimeException(e);
		} 
		finally {
			StreamUtils.closeQuietly(reader);
		}
		if (bundle != null) {
			bundle.setLocale(targetLocale);
		}

		return bundle;
	}

			private static boolean checkFileExistence (FileHandle fh) {
		try {
			fh.read().close();
			return true;
		} catch (Exception e) {
			return false;
		}
	}

	
			protected void load (Reader reader) throws IOException {
		properties = new ObjectMap<String, String>();
		PropertiesUtils.load(properties, reader);
	}

	
	private static FileHandle toFileHandle (FileHandle baseFileHandle, Locale locale) {
		StringBuilder sb = new StringBuilder(baseFileHandle.name());
		if (!locale.equals(ROOT_LOCALE)) {
			String language = locale.getLanguage();
			String country = locale.getCountry();
			String variant = locale.getVariant();
			boolean emptyLanguage = "".equals(language);
			boolean emptyCountry = "".equals(country);
			boolean emptyVariant = "".equals(variant);

			if (!(emptyLanguage && emptyCountry && emptyVariant)) {
				sb.append('_');
				if (!emptyVariant) {
					sb.append(language).append('_').append(country).append('_').append(variant);
				} else if (!emptyCountry) {
					sb.append(language).append('_').append(country);
				} else {
					sb.append(language);
				}
			}
		}
		return baseFileHandle.sibling(sb.append(".properties").toString());
	}

	
	public Locale getLocale () {
		return locale;
	}

	
	private void setLocale (Locale locale) {
		this.locale = locale;
		this.formatter = new TextFormatter(locale, !simpleFormatter);
	}

	
	public final String get (String key) {
		String result = properties.get(key);
		if (result == null) {
			if (parent != null) result = parent.get(key);
			if (result == null) {
				if (exceptionOnMissingKey)
					throw new MissingResourceException("Can't find bundle key " + key, this.getClass().getName(), key);
				else
					return "???" + key + "???";
			}
		}
		return result;
	}

	
	public String format (String key, Object... args) {
		return formatter.format(get(key), args);
	}

}

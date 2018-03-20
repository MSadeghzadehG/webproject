

package com.google.common.util.concurrent;

import static com.google.common.base.StandardSystemProperty.JAVA_CLASS_PATH;
import static com.google.common.base.StandardSystemProperty.PATH_SEPARATOR;

import com.google.common.base.Splitter;
import com.google.common.collect.ImmutableList;
import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;


final class ClassPathUtil {
  private ClassPathUtil() {}

  
    static URL[] parseJavaClassPath() {
    ImmutableList.Builder<URL> urls = ImmutableList.builder();
    for (String entry : Splitter.on(PATH_SEPARATOR.value()).split(JAVA_CLASS_PATH.value())) {
      try {
        try {
          urls.add(new File(entry).toURI().toURL());
        } catch (SecurityException e) {           urls.add(new URL("file", null, new File(entry).getAbsolutePath()));
        }
      } catch (MalformedURLException e) {
        AssertionError error = new AssertionError("malformed class path entry: " + entry);
        error.initCause(e);
        throw error;
      }
    }
    return urls.build().toArray(new URL[0]);
  }

  
  static URL[] getClassPathUrls() {
    return ClassPathUtil.class.getClassLoader() instanceof URLClassLoader
        ? ((URLClassLoader) ClassPathUtil.class.getClassLoader()).getURLs()
        : parseJavaClassPath();
  }
}

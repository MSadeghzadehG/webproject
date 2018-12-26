

package com.google.common.base;

import com.google.common.annotations.GwtIncompatible;
import java.io.Serializable;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


@GwtIncompatible
final class JdkPattern extends CommonPattern implements Serializable {
  private final Pattern pattern;

  JdkPattern(Pattern pattern) {
    this.pattern = Preconditions.checkNotNull(pattern);
  }

  @Override
  CommonMatcher matcher(CharSequence t) {
    return new JdkMatcher(pattern.matcher(t));
  }

  @Override
  String pattern() {
    return pattern.pattern();
  }

  @Override
  int flags() {
    return pattern.flags();
  }

  @Override
  public String toString() {
    return pattern.toString();
  }

  @Override
  public int hashCode() {
    return pattern.hashCode();
  }

  @Override
  public boolean equals(Object o) {
    if (!(o instanceof JdkPattern)) {
      return false;
    }
    return pattern.equals(((JdkPattern) o).pattern);
  }

  private static final class JdkMatcher extends CommonMatcher {
    final Matcher matcher;

    JdkMatcher(Matcher matcher) {
      this.matcher = Preconditions.checkNotNull(matcher);
    }

    @Override
    boolean matches() {
      return matcher.matches();
    }

    @Override
    boolean find() {
      return matcher.find();
    }

    @Override
    boolean find(int index) {
      return matcher.find(index);
    }

    @Override
    String replaceAll(String replacement) {
      return matcher.replaceAll(replacement);
    }

    @Override
    int end() {
      return matcher.end();
    }

    @Override
    int start() {
      return matcher.start();
    }
  }

  private static final long serialVersionUID = 0;
}

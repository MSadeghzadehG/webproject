

package com.google.common.testing;

import com.google.common.annotations.GwtCompatible;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;


@GwtCompatible
final class ClusterException extends RuntimeException {

  public final Collection<? extends Throwable> exceptions;

  private ClusterException(Collection<? extends Throwable> exceptions) {
    super(
        exceptions.size() + " exceptions were thrown. The first exception is listed as a cause.",
        exceptions.iterator().next());
    ArrayList<Throwable> temp = new ArrayList<>();
    temp.addAll(exceptions);
    this.exceptions = Collections.unmodifiableCollection(temp);
  }

  
  public static RuntimeException create(Throwable... exceptions) {
    ArrayList<Throwable> temp = new ArrayList<>(Arrays.asList(exceptions));
    return create(temp);
  }

  
  public static RuntimeException create(Collection<? extends Throwable> exceptions) {
    if (exceptions.size() == 0) {
      throw new IllegalArgumentException("Can't create an ExceptionCollection with no exceptions");
    }
    if (exceptions.size() == 1) {
      Throwable temp = exceptions.iterator().next();
      if (temp instanceof RuntimeException) {
        return (RuntimeException) temp;
      } else {
        return new RuntimeException(temp);
      }
    }
    return new ClusterException(exceptions);
  }
}

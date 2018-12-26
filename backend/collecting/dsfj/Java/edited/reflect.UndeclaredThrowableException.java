

package java.lang.reflect;


public class UndeclaredThrowableException extends RuntimeException {
  public UndeclaredThrowableException(Throwable undeclaredThrowable) {
    super(undeclaredThrowable);
  }

  public UndeclaredThrowableException(Throwable undeclaredThrowable, String message) {
    super(message, undeclaredThrowable);
  }

  public Throwable getUndeclaredThrowable() {
    return getCause();
  }
}

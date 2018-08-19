

package io.reactivex.exceptions;

import io.reactivex.annotations.*;


@Beta
public final class OnErrorNotImplementedException extends RuntimeException {

    private static final long serialVersionUID = -6298857009889503852L;

    
    public OnErrorNotImplementedException(String message, @NonNull Throwable e) {
        super(message, e != null ? e : new NullPointerException());
    }

    
    public OnErrorNotImplementedException(@NonNull Throwable e) {
        super(e != null ? e.getMessage() : null, e != null ? e : new NullPointerException());
    }
}
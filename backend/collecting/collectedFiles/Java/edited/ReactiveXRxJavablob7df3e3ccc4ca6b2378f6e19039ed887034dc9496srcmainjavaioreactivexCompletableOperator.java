

package io.reactivex;

import io.reactivex.annotations.*;


public interface CompletableOperator {
    
    @NonNull
    CompletableObserver apply(@NonNull CompletableObserver observer) throws Exception;
}

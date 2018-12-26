

package io.reactivex;

import io.reactivex.annotations.*;


public interface CompletableTransformer {
    
    @NonNull
    CompletableSource apply(@NonNull Completable upstream);
}



package io.reactivex;

import io.reactivex.annotations.*;


public interface MaybeTransformer<Upstream, Downstream> {
    
    @NonNull
    MaybeSource<Downstream> apply(@NonNull Maybe<Upstream> upstream);
}

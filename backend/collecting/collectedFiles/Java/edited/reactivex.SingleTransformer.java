

package io.reactivex;

import io.reactivex.annotations.*;


public interface SingleTransformer<Upstream, Downstream> {
    
    @NonNull
    SingleSource<Downstream> apply(@NonNull Single<Upstream> upstream);
}

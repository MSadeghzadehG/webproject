

package io.reactivex;

import io.reactivex.annotations.*;


public interface ObservableTransformer<Upstream, Downstream> {
    
    @NonNull
    ObservableSource<Downstream> apply(@NonNull Observable<Upstream> upstream);
}

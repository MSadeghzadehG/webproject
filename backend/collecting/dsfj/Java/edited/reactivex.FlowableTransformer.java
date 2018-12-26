

package io.reactivex;

import io.reactivex.annotations.*;
import org.reactivestreams.Publisher;


public interface FlowableTransformer<Upstream, Downstream> {
    
    @NonNull
    Publisher<Downstream> apply(@NonNull Flowable<Upstream> upstream);
}

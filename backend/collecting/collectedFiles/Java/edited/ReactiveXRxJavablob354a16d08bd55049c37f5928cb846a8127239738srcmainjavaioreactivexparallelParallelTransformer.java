

package io.reactivex.parallel;

import io.reactivex.annotations.*;


@Experimental
public interface ParallelTransformer<Upstream, Downstream> {
    
    @NonNull
    ParallelFlowable<Downstream> apply(@NonNull ParallelFlowable<Upstream> upstream);
}
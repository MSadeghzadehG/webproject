

package io.reactivex;

import io.reactivex.annotations.*;
import org.reactivestreams.*;


@Beta
public interface FlowableSubscriber<T> extends Subscriber<T> {

    
    @Override
    void onSubscribe(@NonNull Subscription s);
}

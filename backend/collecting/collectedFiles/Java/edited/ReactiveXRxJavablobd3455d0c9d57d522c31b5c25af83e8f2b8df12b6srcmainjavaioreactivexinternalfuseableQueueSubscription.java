
package io.reactivex.internal.fuseable;

import java.util.Queue;

import org.reactivestreams.Subscription;


public interface QueueSubscription<T> extends QueueFuseable<T>, Subscription {
}

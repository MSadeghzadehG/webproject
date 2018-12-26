

package io.reactivex.parallel;

import java.util.*;
import java.util.concurrent.Callable;

import io.reactivex.*;
import io.reactivex.annotations.*;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.*;
import io.reactivex.internal.functions.*;
import io.reactivex.internal.operators.parallel.*;
import io.reactivex.internal.subscriptions.EmptySubscription;
import io.reactivex.internal.util.*;
import io.reactivex.plugins.RxJavaPlugins;
import org.reactivestreams.*;


@Beta
public abstract class ParallelFlowable<T> {

    
    public abstract void subscribe(@NonNull Subscriber<? super T>[] subscribers);

    
    public abstract int parallelism();

    
    protected final boolean validate(@NonNull Subscriber<?>[] subscribers) {
        int p = parallelism();
        if (subscribers.length != p) {
            Throwable iae = new IllegalArgumentException("parallelism = " + p + ", subscribers = " + subscribers.length);
            for (Subscriber<?> s : subscribers) {
                EmptySubscription.error(iae, s);
            }
            return false;
        }
        return true;
    }

    
    @CheckReturnValue
    public static <T> ParallelFlowable<T> from(@NonNull Publisher<? extends T> source) {
        return from(source, Runtime.getRuntime().availableProcessors(), Flowable.bufferSize());
    }

    
    @CheckReturnValue
    public static <T> ParallelFlowable<T> from(@NonNull Publisher<? extends T> source, int parallelism) {
        return from(source, parallelism, Flowable.bufferSize());
    }

    
    @CheckReturnValue
    @NonNull
    public static <T> ParallelFlowable<T> from(@NonNull Publisher<? extends T> source,
            int parallelism, int prefetch) {
        ObjectHelper.requireNonNull(source, "source");
        ObjectHelper.verifyPositive(parallelism, "parallelism");
        ObjectHelper.verifyPositive(prefetch, "prefetch");

        return RxJavaPlugins.onAssembly(new ParallelFromPublisher<T>(source, parallelism, prefetch));
    }

    
    @Experimental
    @CheckReturnValue
    @NonNull
    public final <R> R as(@NonNull ParallelFlowableConverter<T, R> converter) {
        return ObjectHelper.requireNonNull(converter, "converter is null").apply(this);
    }

    
    @CheckReturnValue
    @NonNull
    public final <R> ParallelFlowable<R> map(@NonNull Function<? super T, ? extends R> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper");
        return RxJavaPlugins.onAssembly(new ParallelMap<T, R>(this, mapper));
    }

    
    @CheckReturnValue
    @Experimental
    @NonNull
    public final <R> ParallelFlowable<R> map(@NonNull Function<? super T, ? extends R> mapper, @NonNull ParallelFailureHandling errorHandler) {
        ObjectHelper.requireNonNull(mapper, "mapper");
        ObjectHelper.requireNonNull(errorHandler, "errorHandler is null");
        return RxJavaPlugins.onAssembly(new ParallelMapTry<T, R>(this, mapper, errorHandler));
    }

    
    @CheckReturnValue
    @Experimental
    @NonNull
    public final <R> ParallelFlowable<R> map(@NonNull Function<? super T, ? extends R> mapper, @NonNull BiFunction<? super Long, ? super Throwable, ParallelFailureHandling> errorHandler) {
        ObjectHelper.requireNonNull(mapper, "mapper");
        ObjectHelper.requireNonNull(errorHandler, "errorHandler is null");
        return RxJavaPlugins.onAssembly(new ParallelMapTry<T, R>(this, mapper, errorHandler));
    }

    
    @CheckReturnValue
    public final ParallelFlowable<T> filter(@NonNull Predicate<? super T> predicate) {
        ObjectHelper.requireNonNull(predicate, "predicate");
        return RxJavaPlugins.onAssembly(new ParallelFilter<T>(this, predicate));
    }

    
    @CheckReturnValue
    @Experimental
    public final ParallelFlowable<T> filter(@NonNull Predicate<? super T> predicate, @NonNull ParallelFailureHandling errorHandler) {
        ObjectHelper.requireNonNull(predicate, "predicate");
        ObjectHelper.requireNonNull(errorHandler, "errorHandler is null");
        return RxJavaPlugins.onAssembly(new ParallelFilterTry<T>(this, predicate, errorHandler));
    }


    
    @CheckReturnValue
    @Experimental
    public final ParallelFlowable<T> filter(@NonNull Predicate<? super T> predicate, @NonNull BiFunction<? super Long, ? super Throwable, ParallelFailureHandling> errorHandler) {
        ObjectHelper.requireNonNull(predicate, "predicate");
        ObjectHelper.requireNonNull(errorHandler, "errorHandler is null");
        return RxJavaPlugins.onAssembly(new ParallelFilterTry<T>(this, predicate, errorHandler));
    }

    
    @CheckReturnValue
    @NonNull
    public final ParallelFlowable<T> runOn(@NonNull Scheduler scheduler) {
        return runOn(scheduler, Flowable.bufferSize());
    }

    
    @CheckReturnValue
    @NonNull
    public final ParallelFlowable<T> runOn(@NonNull Scheduler scheduler, int prefetch) {
        ObjectHelper.requireNonNull(scheduler, "scheduler");
        ObjectHelper.verifyPositive(prefetch, "prefetch");
        return RxJavaPlugins.onAssembly(new ParallelRunOn<T>(this, scheduler, prefetch));
    }

    
    @CheckReturnValue
    @NonNull
    public final Flowable<T> reduce(@NonNull BiFunction<T, T, T> reducer) {
        ObjectHelper.requireNonNull(reducer, "reducer");
        return RxJavaPlugins.onAssembly(new ParallelReduceFull<T>(this, reducer));
    }

    
    @CheckReturnValue
    @NonNull
    public final <R> ParallelFlowable<R> reduce(@NonNull Callable<R> initialSupplier, @NonNull BiFunction<R, ? super T, R> reducer) {
        ObjectHelper.requireNonNull(initialSupplier, "initialSupplier");
        ObjectHelper.requireNonNull(reducer, "reducer");
        return RxJavaPlugins.onAssembly(new ParallelReduce<T, R>(this, initialSupplier, reducer));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @CheckReturnValue
    public final Flowable<T> sequential() {
        return sequential(Flowable.bufferSize());
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @CheckReturnValue
    @NonNull
    public final Flowable<T> sequential(int prefetch) {
        ObjectHelper.verifyPositive(prefetch, "prefetch");
        return RxJavaPlugins.onAssembly(new ParallelJoin<T>(this, prefetch, false));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @CheckReturnValue
    @Experimental
    @NonNull
    public final Flowable<T> sequentialDelayError() {
        return sequentialDelayError(Flowable.bufferSize());
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @CheckReturnValue
    @NonNull
    public final Flowable<T> sequentialDelayError(int prefetch) {
        ObjectHelper.verifyPositive(prefetch, "prefetch");
        return RxJavaPlugins.onAssembly(new ParallelJoin<T>(this, prefetch, true));
    }

    
    @CheckReturnValue
    @NonNull
    public final Flowable<T> sorted(@NonNull Comparator<? super T> comparator) {
        return sorted(comparator, 16);
    }

    
    @CheckReturnValue
    @NonNull
    public final Flowable<T> sorted(@NonNull Comparator<? super T> comparator, int capacityHint) {
        ObjectHelper.requireNonNull(comparator, "comparator is null");
        ObjectHelper.verifyPositive(capacityHint, "capacityHint");
        int ch = capacityHint / parallelism() + 1;
        ParallelFlowable<List<T>> railReduced = reduce(Functions.<T>createArrayList(ch), ListAddBiConsumer.<T>instance());
        ParallelFlowable<List<T>> railSorted = railReduced.map(new SorterFunction<T>(comparator));

        return RxJavaPlugins.onAssembly(new ParallelSortedJoin<T>(railSorted, comparator));
    }

    
    @CheckReturnValue
    @NonNull
    public final Flowable<List<T>> toSortedList(@NonNull Comparator<? super T> comparator) {
        return toSortedList(comparator, 16);
    }
    
    @CheckReturnValue
    @NonNull
    public final Flowable<List<T>> toSortedList(@NonNull Comparator<? super T> comparator, int capacityHint) {
        ObjectHelper.requireNonNull(comparator, "comparator is null");
        ObjectHelper.verifyPositive(capacityHint, "capacityHint");

        int ch = capacityHint / parallelism() + 1;
        ParallelFlowable<List<T>> railReduced = reduce(Functions.<T>createArrayList(ch), ListAddBiConsumer.<T>instance());
        ParallelFlowable<List<T>> railSorted = railReduced.map(new SorterFunction<T>(comparator));

        Flowable<List<T>> merged = railSorted.reduce(new MergerBiFunction<T>(comparator));

        return RxJavaPlugins.onAssembly(merged);
    }

    
    @CheckReturnValue
    @NonNull
    public final ParallelFlowable<T> doOnNext(@NonNull Consumer<? super T> onNext) {
        ObjectHelper.requireNonNull(onNext, "onNext is null");
        return RxJavaPlugins.onAssembly(new ParallelPeek<T>(this,
                onNext,
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                Functions.EMPTY_ACTION,
                Functions.EMPTY_ACTION,
                Functions.emptyConsumer(),
                Functions.EMPTY_LONG_CONSUMER,
                Functions.EMPTY_ACTION
                ));
    }


    
    @CheckReturnValue
    @Experimental
    @NonNull
    public final ParallelFlowable<T> doOnNext(@NonNull Consumer<? super T> onNext, @NonNull ParallelFailureHandling errorHandler) {
        ObjectHelper.requireNonNull(onNext, "onNext is null");
        ObjectHelper.requireNonNull(errorHandler, "errorHandler is null");
        return RxJavaPlugins.onAssembly(new ParallelDoOnNextTry<T>(this, onNext, errorHandler));
    }

    
    @CheckReturnValue
    @Experimental
    @NonNull
    public final ParallelFlowable<T> doOnNext(@NonNull Consumer<? super T> onNext, @NonNull BiFunction<? super Long, ? super Throwable, ParallelFailureHandling> errorHandler) {
        ObjectHelper.requireNonNull(onNext, "onNext is null");
        ObjectHelper.requireNonNull(errorHandler, "errorHandler is null");
        return RxJavaPlugins.onAssembly(new ParallelDoOnNextTry<T>(this, onNext, errorHandler));
    }

    
    @CheckReturnValue
    @NonNull
    public final ParallelFlowable<T> doAfterNext(@NonNull Consumer<? super T> onAfterNext) {
        ObjectHelper.requireNonNull(onAfterNext, "onAfterNext is null");
        return RxJavaPlugins.onAssembly(new ParallelPeek<T>(this,
                Functions.emptyConsumer(),
                onAfterNext,
                Functions.emptyConsumer(),
                Functions.EMPTY_ACTION,
                Functions.EMPTY_ACTION,
                Functions.emptyConsumer(),
                Functions.EMPTY_LONG_CONSUMER,
                Functions.EMPTY_ACTION
                ));
    }

    
    @CheckReturnValue
    @NonNull
    public final ParallelFlowable<T> doOnError(@NonNull Consumer<Throwable> onError) {
        ObjectHelper.requireNonNull(onError, "onError is null");
        return RxJavaPlugins.onAssembly(new ParallelPeek<T>(this,
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                onError,
                Functions.EMPTY_ACTION,
                Functions.EMPTY_ACTION,
                Functions.emptyConsumer(),
                Functions.EMPTY_LONG_CONSUMER,
                Functions.EMPTY_ACTION
                ));
    }

    
    @CheckReturnValue
    @NonNull
    public final ParallelFlowable<T> doOnComplete(@NonNull Action onComplete) {
        ObjectHelper.requireNonNull(onComplete, "onComplete is null");
        return RxJavaPlugins.onAssembly(new ParallelPeek<T>(this,
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                onComplete,
                Functions.EMPTY_ACTION,
                Functions.emptyConsumer(),
                Functions.EMPTY_LONG_CONSUMER,
                Functions.EMPTY_ACTION
                ));
    }

    
    @CheckReturnValue
    @NonNull
    public final ParallelFlowable<T> doAfterTerminated(@NonNull Action onAfterTerminate) {
        ObjectHelper.requireNonNull(onAfterTerminate, "onAfterTerminate is null");
        return RxJavaPlugins.onAssembly(new ParallelPeek<T>(this,
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                Functions.EMPTY_ACTION,
                onAfterTerminate,
                Functions.emptyConsumer(),
                Functions.EMPTY_LONG_CONSUMER,
                Functions.EMPTY_ACTION
                ));
    }

    
    @CheckReturnValue
    @NonNull
    public final ParallelFlowable<T> doOnSubscribe(@NonNull Consumer<? super Subscription> onSubscribe) {
        ObjectHelper.requireNonNull(onSubscribe, "onSubscribe is null");
        return RxJavaPlugins.onAssembly(new ParallelPeek<T>(this,
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                Functions.EMPTY_ACTION,
                Functions.EMPTY_ACTION,
                onSubscribe,
                Functions.EMPTY_LONG_CONSUMER,
                Functions.EMPTY_ACTION
                ));
    }

    
    @CheckReturnValue
    @NonNull
    public final ParallelFlowable<T> doOnRequest(@NonNull LongConsumer onRequest) {
        ObjectHelper.requireNonNull(onRequest, "onRequest is null");
        return RxJavaPlugins.onAssembly(new ParallelPeek<T>(this,
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                Functions.EMPTY_ACTION,
                Functions.EMPTY_ACTION,
                Functions.emptyConsumer(),
                onRequest,
                Functions.EMPTY_ACTION
                ));
    }

    
    @CheckReturnValue
    @NonNull
    public final ParallelFlowable<T> doOnCancel(@NonNull Action onCancel) {
        ObjectHelper.requireNonNull(onCancel, "onCancel is null");
        return RxJavaPlugins.onAssembly(new ParallelPeek<T>(this,
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                Functions.EMPTY_ACTION,
                Functions.EMPTY_ACTION,
                Functions.emptyConsumer(),
                Functions.EMPTY_LONG_CONSUMER,
                onCancel
                ));
    }

    
    @CheckReturnValue
    @NonNull
    public final <C> ParallelFlowable<C> collect(@NonNull Callable<? extends C> collectionSupplier, @NonNull BiConsumer<? super C, ? super T> collector) {
        ObjectHelper.requireNonNull(collectionSupplier, "collectionSupplier is null");
        ObjectHelper.requireNonNull(collector, "collector is null");
        return RxJavaPlugins.onAssembly(new ParallelCollect<T, C>(this, collectionSupplier, collector));
    }

    
    @CheckReturnValue
    @NonNull
    public static <T> ParallelFlowable<T> fromArray(@NonNull Publisher<T>... publishers) {
        if (publishers.length == 0) {
            throw new IllegalArgumentException("Zero publishers not supported");
        }
        return RxJavaPlugins.onAssembly(new ParallelFromArray<T>(publishers));
    }

    
    @CheckReturnValue
    @NonNull
    public final <U> U to(@NonNull Function<? super ParallelFlowable<T>, U> converter) {
        try {
            return ObjectHelper.requireNonNull(converter, "converter is null").apply(this);
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            throw ExceptionHelper.wrapOrThrow(ex);
        }
    }

    
    @CheckReturnValue
    @NonNull
    public final <U> ParallelFlowable<U> compose(@NonNull ParallelTransformer<T, U> composer) {
        return RxJavaPlugins.onAssembly(ObjectHelper.requireNonNull(composer, "composer is null").apply(this));
    }

    
    @CheckReturnValue
    @NonNull
    public final <R> ParallelFlowable<R> flatMap(@NonNull Function<? super T, ? extends Publisher<? extends R>> mapper) {
        return flatMap(mapper, false, Integer.MAX_VALUE, Flowable.bufferSize());
    }

    
    @CheckReturnValue
    @NonNull
    public final <R> ParallelFlowable<R> flatMap(
            @NonNull Function<? super T, ? extends Publisher<? extends R>> mapper, boolean delayError) {
        return flatMap(mapper, delayError, Integer.MAX_VALUE, Flowable.bufferSize());
    }

    
    @CheckReturnValue
    @NonNull
    public final <R> ParallelFlowable<R> flatMap(
            @NonNull Function<? super T, ? extends Publisher<? extends R>> mapper, boolean delayError, int maxConcurrency) {
        return flatMap(mapper, delayError, maxConcurrency, Flowable.bufferSize());
    }

    
    @CheckReturnValue
    @NonNull
    public final <R> ParallelFlowable<R> flatMap(
            @NonNull Function<? super T, ? extends Publisher<? extends R>> mapper,
            boolean delayError, int maxConcurrency, int prefetch) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        ObjectHelper.verifyPositive(maxConcurrency, "maxConcurrency");
        ObjectHelper.verifyPositive(prefetch, "prefetch");
        return RxJavaPlugins.onAssembly(new ParallelFlatMap<T, R>(this, mapper, delayError, maxConcurrency, prefetch));
    }

    
    @CheckReturnValue
    @NonNull
    public final <R> ParallelFlowable<R> concatMap(
            @NonNull Function<? super T, ? extends Publisher<? extends R>> mapper) {
        return concatMap(mapper, 2);
    }

    
    @CheckReturnValue
    @NonNull
    public final <R> ParallelFlowable<R> concatMap(
            @NonNull Function<? super T, ? extends Publisher<? extends R>> mapper,
                    int prefetch) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        ObjectHelper.verifyPositive(prefetch, "prefetch");
        return RxJavaPlugins.onAssembly(new ParallelConcatMap<T, R>(this, mapper, prefetch, ErrorMode.IMMEDIATE));
    }

    
    @CheckReturnValue
    @NonNull
    public final <R> ParallelFlowable<R> concatMapDelayError(
            @NonNull Function<? super T, ? extends Publisher<? extends R>> mapper,
                    boolean tillTheEnd) {
        return concatMapDelayError(mapper, 2, tillTheEnd);
    }

    
    @CheckReturnValue
    @NonNull
    public final <R> ParallelFlowable<R> concatMapDelayError(
            @NonNull Function<? super T, ? extends Publisher<? extends R>> mapper,
                    int prefetch, boolean tillTheEnd) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        ObjectHelper.verifyPositive(prefetch, "prefetch");
        return RxJavaPlugins.onAssembly(new ParallelConcatMap<T, R>(
                this, mapper, prefetch, tillTheEnd ? ErrorMode.END : ErrorMode.BOUNDARY));
    }
}

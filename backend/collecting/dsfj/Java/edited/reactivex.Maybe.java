

package io.reactivex;

import java.util.NoSuchElementException;
import java.util.concurrent.*;

import org.reactivestreams.*;

import io.reactivex.annotations.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.*;
import io.reactivex.internal.functions.*;
import io.reactivex.internal.fuseable.*;
import io.reactivex.internal.observers.BlockingMultiObserver;
import io.reactivex.internal.operators.flowable.*;
import io.reactivex.internal.operators.maybe.*;
import io.reactivex.internal.util.*;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.schedulers.Schedulers;


public abstract class Maybe<T> implements MaybeSource<T> {

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> amb(final Iterable<? extends MaybeSource<? extends T>> sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        return RxJavaPlugins.onAssembly(new MaybeAmb<T>(null, sources));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Maybe<T> ambArray(final MaybeSource<? extends T>... sources) {
        if (sources.length == 0) {
            return empty();
        }
        if (sources.length == 1) {
            return wrap((MaybeSource<T>)sources[0]);
        }
        return RxJavaPlugins.onAssembly(new MaybeAmb<T>(sources, null));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> concat(Iterable<? extends MaybeSource<? extends T>> sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        return RxJavaPlugins.onAssembly(new MaybeConcatIterable<T>(sources));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> concat(MaybeSource<? extends T> source1, MaybeSource<? extends T> source2) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        return concatArray(source1, source2);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> concat(
            MaybeSource<? extends T> source1, MaybeSource<? extends T> source2, MaybeSource<? extends T> source3) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        return concatArray(source1, source2, source3);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> concat(
            MaybeSource<? extends T> source1, MaybeSource<? extends T> source2, MaybeSource<? extends T> source3, MaybeSource<? extends T> source4) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        return concatArray(source1, source2, source3, source4);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> concat(Publisher<? extends MaybeSource<? extends T>> sources) {
        return concat(sources, 2);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings({ "unchecked", "rawtypes" })
    public static <T> Flowable<T> concat(Publisher<? extends MaybeSource<? extends T>> sources, int prefetch) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        ObjectHelper.verifyPositive(prefetch, "prefetch");
        return RxJavaPlugins.onAssembly(new FlowableConcatMapPublisher(sources, MaybeToPublisher.instance(), prefetch, ErrorMode.IMMEDIATE));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> concatArray(MaybeSource<? extends T>... sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        if (sources.length == 0) {
            return Flowable.empty();
        }
        if (sources.length == 1) {
            return RxJavaPlugins.onAssembly(new MaybeToFlowable<T>((MaybeSource<T>)sources[0]));
        }
        return RxJavaPlugins.onAssembly(new MaybeConcatArray<T>(sources));
    }

    
    @SuppressWarnings("unchecked")
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> concatArrayDelayError(MaybeSource<? extends T>... sources) {
        if (sources.length == 0) {
            return Flowable.empty();
        } else
        if (sources.length == 1) {
            return RxJavaPlugins.onAssembly(new MaybeToFlowable<T>((MaybeSource<T>)sources[0]));
        }
        return RxJavaPlugins.onAssembly(new MaybeConcatArrayDelayError<T>(sources));
    }

    
    @SuppressWarnings({ "rawtypes", "unchecked" })
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> concatArrayEager(MaybeSource<? extends T>... sources) {
        return Flowable.fromArray(sources).concatMapEager((Function)MaybeToPublisher.instance());
    }

    
    @SuppressWarnings({ "unchecked", "rawtypes" })
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> concatDelayError(Iterable<? extends MaybeSource<? extends T>> sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        return Flowable.fromIterable(sources).concatMapDelayError((Function)MaybeToPublisher.instance());
    }

    
    @SuppressWarnings({ "unchecked", "rawtypes" })
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> concatDelayError(Publisher<? extends MaybeSource<? extends T>> sources) {
        return Flowable.fromPublisher(sources).concatMapDelayError((Function)MaybeToPublisher.instance());
    }

    
    @SuppressWarnings({ "rawtypes", "unchecked" })
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> concatEager(Iterable<? extends MaybeSource<? extends T>> sources) {
        return Flowable.fromIterable(sources).concatMapEager((Function)MaybeToPublisher.instance());
    }

    
    @SuppressWarnings({ "rawtypes", "unchecked" })
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> concatEager(Publisher<? extends MaybeSource<? extends T>> sources) {
        return Flowable.fromPublisher(sources).concatMapEager((Function)MaybeToPublisher.instance());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> create(MaybeOnSubscribe<T> onSubscribe) {
        ObjectHelper.requireNonNull(onSubscribe, "onSubscribe is null");
        return RxJavaPlugins.onAssembly(new MaybeCreate<T>(onSubscribe));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> defer(final Callable<? extends MaybeSource<? extends T>> maybeSupplier) {
        ObjectHelper.requireNonNull(maybeSupplier, "maybeSupplier is null");
        return RxJavaPlugins.onAssembly(new MaybeDefer<T>(maybeSupplier));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Maybe<T> empty() {
        return RxJavaPlugins.onAssembly((Maybe<T>)MaybeEmpty.INSTANCE);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> error(Throwable exception) {
        ObjectHelper.requireNonNull(exception, "exception is null");
        return RxJavaPlugins.onAssembly(new MaybeError<T>(exception));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> error(Callable<? extends Throwable> supplier) {
        ObjectHelper.requireNonNull(supplier, "errorSupplier is null");
        return RxJavaPlugins.onAssembly(new MaybeErrorCallable<T>(supplier));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> fromAction(final Action run) {
        ObjectHelper.requireNonNull(run, "run is null");
        return RxJavaPlugins.onAssembly(new MaybeFromAction<T>(run));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> fromCompletable(CompletableSource completableSource) {
        ObjectHelper.requireNonNull(completableSource, "completableSource is null");
        return RxJavaPlugins.onAssembly(new MaybeFromCompletable<T>(completableSource));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> fromSingle(SingleSource<T> singleSource) {
        ObjectHelper.requireNonNull(singleSource, "singleSource is null");
        return RxJavaPlugins.onAssembly(new MaybeFromSingle<T>(singleSource));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> fromCallable(@NonNull final Callable<? extends T> callable) {
        ObjectHelper.requireNonNull(callable, "callable is null");
        return RxJavaPlugins.onAssembly(new MaybeFromCallable<T>(callable));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> fromFuture(Future<? extends T> future) {
        ObjectHelper.requireNonNull(future, "future is null");
        return RxJavaPlugins.onAssembly(new MaybeFromFuture<T>(future, 0L, null));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> fromFuture(Future<? extends T> future, long timeout, TimeUnit unit) {
        ObjectHelper.requireNonNull(future, "future is null");
        ObjectHelper.requireNonNull(unit, "unit is null");
        return RxJavaPlugins.onAssembly(new MaybeFromFuture<T>(future, timeout, unit));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> fromRunnable(final Runnable run) {
        ObjectHelper.requireNonNull(run, "run is null");
        return RxJavaPlugins.onAssembly(new MaybeFromRunnable<T>(run));
    }


    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> just(T item) {
        ObjectHelper.requireNonNull(item, "item is null");
        return RxJavaPlugins.onAssembly(new MaybeJust<T>(item));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> merge(Iterable<? extends MaybeSource<? extends T>> sources) {
        return merge(Flowable.fromIterable(sources));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> merge(Publisher<? extends MaybeSource<? extends T>> sources) {
        return merge(sources, Integer.MAX_VALUE);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings({ "unchecked", "rawtypes" })
    public static <T> Flowable<T> merge(Publisher<? extends MaybeSource<? extends T>> sources, int maxConcurrency) {
        ObjectHelper.requireNonNull(sources, "source is null");
        ObjectHelper.verifyPositive(maxConcurrency, "maxConcurrency");
        return RxJavaPlugins.onAssembly(new FlowableFlatMapPublisher(sources, MaybeToPublisher.instance(), false, maxConcurrency, 1));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings({ "unchecked", "rawtypes" })
    public static <T> Maybe<T> merge(MaybeSource<? extends MaybeSource<? extends T>> source) {
        ObjectHelper.requireNonNull(source, "source is null");
        return RxJavaPlugins.onAssembly(new MaybeFlatten(source, Functions.identity()));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> merge(
            MaybeSource<? extends T> source1, MaybeSource<? extends T> source2
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        return mergeArray(source1, source2);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> merge(
            MaybeSource<? extends T> source1, MaybeSource<? extends T> source2,
            MaybeSource<? extends T> source3
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        return mergeArray(source1, source2, source3);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> merge(
            MaybeSource<? extends T> source1, MaybeSource<? extends T> source2,
            MaybeSource<? extends T> source3, MaybeSource<? extends T> source4
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        return mergeArray(source1, source2, source3, source4);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> mergeArray(MaybeSource<? extends T>... sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        if (sources.length == 0) {
            return Flowable.empty();
        }
        if (sources.length == 1) {
            return RxJavaPlugins.onAssembly(new MaybeToFlowable<T>((MaybeSource<T>)sources[0]));
        }
        return RxJavaPlugins.onAssembly(new MaybeMergeArray<T>(sources));
    }

    
    @SuppressWarnings({ "unchecked", "rawtypes" })
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> mergeArrayDelayError(MaybeSource<? extends T>... sources) {
        if (sources.length == 0) {
            return Flowable.empty();
        }
        return Flowable.fromArray(sources).flatMap((Function)MaybeToPublisher.instance(), true, sources.length);
    }


    
    @SuppressWarnings({ "unchecked", "rawtypes" })
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> mergeDelayError(Iterable<? extends MaybeSource<? extends T>> sources) {
        return Flowable.fromIterable(sources).flatMap((Function)MaybeToPublisher.instance(), true);
    }


    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> mergeDelayError(Publisher<? extends MaybeSource<? extends T>> sources) {
        return mergeDelayError(sources, Integer.MAX_VALUE);
    }


    
    @SuppressWarnings({ "unchecked", "rawtypes" })
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @Experimental
    public static <T> Flowable<T> mergeDelayError(Publisher<? extends MaybeSource<? extends T>> sources, int maxConcurrency) {
        ObjectHelper.requireNonNull(sources, "source is null");
        ObjectHelper.verifyPositive(maxConcurrency, "maxConcurrency");
        return RxJavaPlugins.onAssembly(new FlowableFlatMapPublisher(sources, MaybeToPublisher.instance(), true, maxConcurrency, 1));
    }

    
    @SuppressWarnings({ "unchecked" })
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> mergeDelayError(MaybeSource<? extends T> source1, MaybeSource<? extends T> source2) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        return mergeArrayDelayError(source1, source2);
    }

    
    @SuppressWarnings({ "unchecked" })
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> mergeDelayError(MaybeSource<? extends T> source1,
            MaybeSource<? extends T> source2, MaybeSource<? extends T> source3) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        return mergeArrayDelayError(source1, source2, source3);
    }


    
    @SuppressWarnings({ "unchecked" })
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> mergeDelayError(
            MaybeSource<? extends T> source1, MaybeSource<? extends T> source2,
            MaybeSource<? extends T> source3, MaybeSource<? extends T> source4) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        return mergeArrayDelayError(source1, source2, source3, source4);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Maybe<T> never() {
        return RxJavaPlugins.onAssembly((Maybe<T>)MaybeNever.INSTANCE);
    }


    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<Boolean> sequenceEqual(MaybeSource<? extends T> source1, MaybeSource<? extends T> source2) {
        return sequenceEqual(source1, source2, ObjectHelper.equalsPredicate());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<Boolean> sequenceEqual(MaybeSource<? extends T> source1, MaybeSource<? extends T> source2,
            BiPredicate<? super T, ? super T> isEqual) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(isEqual, "isEqual is null");
        return RxJavaPlugins.onAssembly(new MaybeEqualSingle<T>(source1, source2, isEqual));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public static Maybe<Long> timer(long delay, TimeUnit unit) {
        return timer(delay, unit, Schedulers.computation());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public static Maybe<Long> timer(long delay, TimeUnit unit, Scheduler scheduler) {
        ObjectHelper.requireNonNull(unit, "unit is null");
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");

        return RxJavaPlugins.onAssembly(new MaybeTimer(Math.max(0L, delay), unit, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> unsafeCreate(MaybeSource<T> onSubscribe) {
        if (onSubscribe instanceof Maybe) {
            throw new IllegalArgumentException("unsafeCreate(Maybe) should be upgraded");
        }
        ObjectHelper.requireNonNull(onSubscribe, "onSubscribe is null");
        return RxJavaPlugins.onAssembly(new MaybeUnsafeCreate<T>(onSubscribe));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T, D> Maybe<T> using(Callable<? extends D> resourceSupplier,
            Function<? super D, ? extends MaybeSource<? extends T>> sourceSupplier,
                    Consumer<? super D> resourceDisposer) {
        return using(resourceSupplier, sourceSupplier, resourceDisposer, true);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T, D> Maybe<T> using(Callable<? extends D> resourceSupplier,
            Function<? super D, ? extends MaybeSource<? extends T>> sourceSupplier,
                    Consumer<? super D> resourceDisposer, boolean eager) {
        ObjectHelper.requireNonNull(resourceSupplier, "resourceSupplier is null");
        ObjectHelper.requireNonNull(sourceSupplier, "sourceSupplier is null");
        ObjectHelper.requireNonNull(resourceDisposer, "disposer is null");
        return RxJavaPlugins.onAssembly(new MaybeUsing<T, D>(resourceSupplier, sourceSupplier, resourceDisposer, eager));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Maybe<T> wrap(MaybeSource<T> source) {
        if (source instanceof Maybe) {
            return RxJavaPlugins.onAssembly((Maybe<T>)source);
        }
        ObjectHelper.requireNonNull(source, "onSubscribe is null");
        return RxJavaPlugins.onAssembly(new MaybeUnsafeCreate<T>(source));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T, R> Maybe<R> zip(Iterable<? extends MaybeSource<? extends T>> sources, Function<? super Object[], ? extends R> zipper) {
        ObjectHelper.requireNonNull(zipper, "zipper is null");
        ObjectHelper.requireNonNull(sources, "sources is null");
        return RxJavaPlugins.onAssembly(new MaybeZipIterable<T, R>(sources, zipper));
    }

    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T1, T2, R> Maybe<R> zip(
            MaybeSource<? extends T1> source1, MaybeSource<? extends T2> source2,
            BiFunction<? super T1, ? super T2, ? extends R> zipper) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2);
    }

    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T1, T2, T3, R> Maybe<R> zip(
            MaybeSource<? extends T1> source1, MaybeSource<? extends T2> source2, MaybeSource<? extends T3> source3,
            Function3<? super T1, ? super T2, ? super T3, ? extends R> zipper) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3);
    }

    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T1, T2, T3, T4, R> Maybe<R> zip(
            MaybeSource<? extends T1> source1, MaybeSource<? extends T2> source2, MaybeSource<? extends T3> source3,
            MaybeSource<? extends T4> source4,
            Function4<? super T1, ? super T2, ? super T3, ? super T4, ? extends R> zipper) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3, source4);
    }

    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T1, T2, T3, T4, T5, R> Maybe<R> zip(
            MaybeSource<? extends T1> source1, MaybeSource<? extends T2> source2, MaybeSource<? extends T3> source3,
            MaybeSource<? extends T4> source4, MaybeSource<? extends T5> source5,
            Function5<? super T1, ? super T2, ? super T3, ? super T4, ? super T5, ? extends R> zipper) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        ObjectHelper.requireNonNull(source5, "source5 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3, source4, source5);
    }

    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T1, T2, T3, T4, T5, T6, R> Maybe<R> zip(
            MaybeSource<? extends T1> source1, MaybeSource<? extends T2> source2, MaybeSource<? extends T3> source3,
            MaybeSource<? extends T4> source4, MaybeSource<? extends T5> source5, MaybeSource<? extends T6> source6,
            Function6<? super T1, ? super T2, ? super T3, ? super T4, ? super T5, ? super T6, ? extends R> zipper) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        ObjectHelper.requireNonNull(source5, "source5 is null");
        ObjectHelper.requireNonNull(source6, "source6 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3, source4, source5, source6);
    }

    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T1, T2, T3, T4, T5, T6, T7, R> Maybe<R> zip(
            MaybeSource<? extends T1> source1, MaybeSource<? extends T2> source2, MaybeSource<? extends T3> source3,
            MaybeSource<? extends T4> source4, MaybeSource<? extends T5> source5, MaybeSource<? extends T6> source6,
            MaybeSource<? extends T7> source7,
            Function7<? super T1, ? super T2, ? super T3, ? super T4, ? super T5, ? super T6, ? super T7, ? extends R> zipper) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        ObjectHelper.requireNonNull(source5, "source5 is null");
        ObjectHelper.requireNonNull(source6, "source6 is null");
        ObjectHelper.requireNonNull(source7, "source7 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3, source4, source5, source6, source7);
    }

    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T1, T2, T3, T4, T5, T6, T7, T8, R> Maybe<R> zip(
            MaybeSource<? extends T1> source1, MaybeSource<? extends T2> source2, MaybeSource<? extends T3> source3,
            MaybeSource<? extends T4> source4, MaybeSource<? extends T5> source5, MaybeSource<? extends T6> source6,
            MaybeSource<? extends T7> source7, MaybeSource<? extends T8> source8,
            Function8<? super T1, ? super T2, ? super T3, ? super T4, ? super T5, ? super T6, ? super T7, ? super T8, ? extends R> zipper) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        ObjectHelper.requireNonNull(source5, "source5 is null");
        ObjectHelper.requireNonNull(source6, "source6 is null");
        ObjectHelper.requireNonNull(source7, "source7 is null");
        ObjectHelper.requireNonNull(source8, "source8 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3, source4, source5, source6, source7, source8);
    }

    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T1, T2, T3, T4, T5, T6, T7, T8, T9, R> Maybe<R> zip(
            MaybeSource<? extends T1> source1, MaybeSource<? extends T2> source2, MaybeSource<? extends T3> source3,
            MaybeSource<? extends T4> source4, MaybeSource<? extends T5> source5, MaybeSource<? extends T6> source6,
            MaybeSource<? extends T7> source7, MaybeSource<? extends T8> source8, MaybeSource<? extends T9> source9,
            Function9<? super T1, ? super T2, ? super T3, ? super T4, ? super T5, ? super T6, ? super T7, ? super T8, ? super T9, ? extends R> zipper) {

        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        ObjectHelper.requireNonNull(source5, "source5 is null");
        ObjectHelper.requireNonNull(source6, "source6 is null");
        ObjectHelper.requireNonNull(source7, "source7 is null");
        ObjectHelper.requireNonNull(source8, "source8 is null");
        ObjectHelper.requireNonNull(source9, "source9 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3, source4, source5, source6, source7, source8, source9);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T, R> Maybe<R> zipArray(Function<? super Object[], ? extends R> zipper,
            MaybeSource<? extends T>... sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        if (sources.length == 0) {
            return empty();
        }
        ObjectHelper.requireNonNull(zipper, "zipper is null");
        return RxJavaPlugins.onAssembly(new MaybeZipArray<T, R>(sources, zipper));
    }

            
    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> ambWith(MaybeSource<? extends T> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return ambArray(this, other);
    }

    
    @Experimental
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> R as(@NonNull MaybeConverter<T, ? extends R> converter) {
        return ObjectHelper.requireNonNull(converter, "converter is null").apply(this);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final T blockingGet() {
        BlockingMultiObserver<T> observer = new BlockingMultiObserver<T>();
        subscribe(observer);
        return observer.blockingGet();
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final T blockingGet(T defaultValue) {
        ObjectHelper.requireNonNull(defaultValue, "defaultValue is null");
        BlockingMultiObserver<T> observer = new BlockingMultiObserver<T>();
        subscribe(observer);
        return observer.blockingGet(defaultValue);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> cache() {
        return RxJavaPlugins.onAssembly(new MaybeCache<T>(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Maybe<U> cast(final Class<? extends U> clazz) {
        ObjectHelper.requireNonNull(clazz, "clazz is null");
        return map(Functions.castFunction(clazz));
    }

    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Maybe<R> compose(MaybeTransformer<? super T, ? extends R> transformer) {
        return wrap(((MaybeTransformer<T, R>) ObjectHelper.requireNonNull(transformer, "transformer is null")).apply(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Maybe<R> concatMap(Function<? super T, ? extends MaybeSource<? extends R>> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new MaybeFlatten<T, R>(this, mapper));
    }


    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Flowable<T> concatWith(MaybeSource<? extends T> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return concat(this, other);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<Boolean> contains(final Object item) {
        ObjectHelper.requireNonNull(item, "item is null");
        return RxJavaPlugins.onAssembly(new MaybeContains<T>(this, item));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<Long> count() {
        return RxJavaPlugins.onAssembly(new MaybeCount<T>(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> defaultIfEmpty(T defaultItem) {
        ObjectHelper.requireNonNull(defaultItem, "item is null");
        return switchIfEmpty(just(defaultItem));
    }


    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Maybe<T> delay(long delay, TimeUnit unit) {
        return delay(delay, unit, Schedulers.computation());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Maybe<T> delay(long delay, TimeUnit unit, Scheduler scheduler) {
        ObjectHelper.requireNonNull(unit, "unit is null");
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new MaybeDelay<T>(this, Math.max(0L, delay), unit, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @BackpressureSupport(BackpressureKind.UNBOUNDED_IN)
    public final <U, V> Maybe<T> delay(Publisher<U> delayIndicator) {
        ObjectHelper.requireNonNull(delayIndicator, "delayIndicator is null");
        return RxJavaPlugins.onAssembly(new MaybeDelayOtherPublisher<T, U>(this, delayIndicator));
    }

    
    @BackpressureSupport(BackpressureKind.UNBOUNDED_IN)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Maybe<T> delaySubscription(Publisher<U> subscriptionIndicator) {
        ObjectHelper.requireNonNull(subscriptionIndicator, "subscriptionIndicator is null");
        return RxJavaPlugins.onAssembly(new MaybeDelaySubscriptionOtherPublisher<T, U>(this, subscriptionIndicator));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Maybe<T> delaySubscription(long delay, TimeUnit unit) {
        return delaySubscription(delay, unit, Schedulers.computation());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Maybe<T> delaySubscription(long delay, TimeUnit unit, Scheduler scheduler) {
        return delaySubscription(Flowable.timer(delay, unit, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> doAfterSuccess(Consumer<? super T> onAfterSuccess) {
        ObjectHelper.requireNonNull(onAfterSuccess, "doAfterSuccess is null");
        return RxJavaPlugins.onAssembly(new MaybeDoAfterSuccess<T>(this, onAfterSuccess));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> doAfterTerminate(Action onAfterTerminate) {
        return RxJavaPlugins.onAssembly(new MaybePeek<T>(this,
                Functions.emptyConsumer(),                 Functions.emptyConsumer(),                 Functions.emptyConsumer(),                 Functions.EMPTY_ACTION,                    ObjectHelper.requireNonNull(onAfterTerminate, "onAfterTerminate is null"),
                Functions.EMPTY_ACTION             ));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> doFinally(Action onFinally) {
        ObjectHelper.requireNonNull(onFinally, "onFinally is null");
        return RxJavaPlugins.onAssembly(new MaybeDoFinally<T>(this, onFinally));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> doOnDispose(Action onDispose) {
        return RxJavaPlugins.onAssembly(new MaybePeek<T>(this,
                Functions.emptyConsumer(),                 Functions.emptyConsumer(),                 Functions.emptyConsumer(),                 Functions.EMPTY_ACTION,                    Functions.EMPTY_ACTION,                    ObjectHelper.requireNonNull(onDispose, "onDispose is null")
        ));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> doOnComplete(Action onComplete) {
        return RxJavaPlugins.onAssembly(new MaybePeek<T>(this,
                Functions.emptyConsumer(),                 Functions.emptyConsumer(),                 Functions.emptyConsumer(),                 ObjectHelper.requireNonNull(onComplete, "onComplete is null"),
                Functions.EMPTY_ACTION,                    Functions.EMPTY_ACTION             ));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> doOnError(Consumer<? super Throwable> onError) {
        return RxJavaPlugins.onAssembly(new MaybePeek<T>(this,
                Functions.emptyConsumer(),                 Functions.emptyConsumer(),                 ObjectHelper.requireNonNull(onError, "onError is null"),
                Functions.EMPTY_ACTION,                    Functions.EMPTY_ACTION,                    Functions.EMPTY_ACTION             ));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> doOnEvent(BiConsumer<? super T, ? super Throwable> onEvent) {
        ObjectHelper.requireNonNull(onEvent, "onEvent is null");
        return RxJavaPlugins.onAssembly(new MaybeDoOnEvent<T>(this, onEvent));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> doOnSubscribe(Consumer<? super Disposable> onSubscribe) {
        return RxJavaPlugins.onAssembly(new MaybePeek<T>(this,
                ObjectHelper.requireNonNull(onSubscribe, "onSubscribe is null"),
                Functions.emptyConsumer(),                 Functions.emptyConsumer(),                 Functions.EMPTY_ACTION,                    Functions.EMPTY_ACTION,                    Functions.EMPTY_ACTION             ));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> doOnSuccess(Consumer<? super T> onSuccess) {
        return RxJavaPlugins.onAssembly(new MaybePeek<T>(this,
                Functions.emptyConsumer(),                 ObjectHelper.requireNonNull(onSuccess, "onSubscribe is null"),
                Functions.emptyConsumer(),                 Functions.EMPTY_ACTION,                    Functions.EMPTY_ACTION,                    Functions.EMPTY_ACTION             ));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> filter(Predicate<? super T> predicate) {
        ObjectHelper.requireNonNull(predicate, "predicate is null");
        return RxJavaPlugins.onAssembly(new MaybeFilter<T>(this, predicate));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Maybe<R> flatMap(Function<? super T, ? extends MaybeSource<? extends R>> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new MaybeFlatten<T, R>(this, mapper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Maybe<R> flatMap(
            Function<? super T, ? extends MaybeSource<? extends R>> onSuccessMapper,
            Function<? super Throwable, ? extends MaybeSource<? extends R>> onErrorMapper,
            Callable<? extends MaybeSource<? extends R>> onCompleteSupplier) {
        ObjectHelper.requireNonNull(onSuccessMapper, "onSuccessMapper is null");
        ObjectHelper.requireNonNull(onErrorMapper, "onErrorMapper is null");
        ObjectHelper.requireNonNull(onCompleteSupplier, "onCompleteSupplier is null");
        return RxJavaPlugins.onAssembly(new MaybeFlatMapNotification<T, R>(this, onSuccessMapper, onErrorMapper, onCompleteSupplier));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U, R> Maybe<R> flatMap(Function<? super T, ? extends MaybeSource<? extends U>> mapper,
            BiFunction<? super T, ? super U, ? extends R> resultSelector) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        ObjectHelper.requireNonNull(resultSelector, "resultSelector is null");
        return RxJavaPlugins.onAssembly(new MaybeFlatMapBiSelector<T, U, R>(this, mapper, resultSelector));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Flowable<U> flattenAsFlowable(final Function<? super T, ? extends Iterable<? extends U>> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new MaybeFlatMapIterableFlowable<T, U>(this, mapper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Observable<U> flattenAsObservable(final Function<? super T, ? extends Iterable<? extends U>> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new MaybeFlatMapIterableObservable<T, U>(this, mapper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Observable<R> flatMapObservable(Function<? super T, ? extends ObservableSource<? extends R>> mapper) {
        return toObservable().flatMap(mapper);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Flowable<R> flatMapPublisher(Function<? super T, ? extends Publisher<? extends R>> mapper) {
        return toFlowable().flatMap(mapper);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Single<R> flatMapSingle(final Function<? super T, ? extends SingleSource<? extends R>> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new MaybeFlatMapSingle<T, R>(this, mapper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Maybe<R> flatMapSingleElement(final Function<? super T, ? extends SingleSource<? extends R>> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new MaybeFlatMapSingleElement<T, R>(this, mapper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable flatMapCompletable(final Function<? super T, ? extends CompletableSource> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new MaybeFlatMapCompletable<T>(this, mapper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> hide() {
        return RxJavaPlugins.onAssembly(new MaybeHide<T>(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable ignoreElement() {
        return RxJavaPlugins.onAssembly(new MaybeIgnoreElementCompletable<T>(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<Boolean> isEmpty() {
        return RxJavaPlugins.onAssembly(new MaybeIsEmptySingle<T>(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Maybe<R> lift(final MaybeOperator<? extends R, ? super T> lift) {
        ObjectHelper.requireNonNull(lift, "onLift is null");
        return RxJavaPlugins.onAssembly(new MaybeLift<T, R>(this, lift));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Maybe<R> map(Function<? super T, ? extends R> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new MaybeMap<T, R>(this, mapper));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Flowable<T> mergeWith(MaybeSource<? extends T> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return merge(this, other);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Maybe<T> observeOn(final Scheduler scheduler) {
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new MaybeObserveOn<T>(this, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Maybe<U> ofType(final Class<U> clazz) {
        ObjectHelper.requireNonNull(clazz, "clazz is null");
        return filter(Functions.isInstanceOf(clazz)).cast(clazz);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> R to(Function<? super Maybe<T>, R> convert) {
        try {
            return ObjectHelper.requireNonNull(convert, "convert is null").apply(this);
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            throw ExceptionHelper.wrapOrThrow(ex);
        }
    }

    
    @SuppressWarnings("unchecked")
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Flowable<T> toFlowable() {
        if (this instanceof FuseToFlowable) {
            return ((FuseToFlowable<T>)this).fuseToFlowable();
        }
        return RxJavaPlugins.onAssembly(new MaybeToFlowable<T>(this));
    }

    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Observable<T> toObservable() {
        if (this instanceof FuseToObservable) {
            return ((FuseToObservable<T>)this).fuseToObservable();
        }
        return RxJavaPlugins.onAssembly(new MaybeToObservable<T>(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> toSingle(T defaultValue) {
        ObjectHelper.requireNonNull(defaultValue, "defaultValue is null");
        return RxJavaPlugins.onAssembly(new MaybeToSingle<T>(this, defaultValue));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> toSingle() {
        return RxJavaPlugins.onAssembly(new MaybeToSingle<T>(this, null));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> onErrorComplete() {
        return onErrorComplete(Functions.alwaysTrue());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> onErrorComplete(final Predicate<? super Throwable> predicate) {
        ObjectHelper.requireNonNull(predicate, "predicate is null");

        return RxJavaPlugins.onAssembly(new MaybeOnErrorComplete<T>(this, predicate));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> onErrorResumeNext(final MaybeSource<? extends T> next) {
        ObjectHelper.requireNonNull(next, "next is null");
        return onErrorResumeNext(Functions.justFunction(next));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> onErrorResumeNext(Function<? super Throwable, ? extends MaybeSource<? extends T>> resumeFunction) {
        ObjectHelper.requireNonNull(resumeFunction, "resumeFunction is null");
        return RxJavaPlugins.onAssembly(new MaybeOnErrorNext<T>(this, resumeFunction, true));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> onErrorReturn(Function<? super Throwable, ? extends T> valueSupplier) {
        ObjectHelper.requireNonNull(valueSupplier, "valueSupplier is null");
        return RxJavaPlugins.onAssembly(new MaybeOnErrorReturn<T>(this, valueSupplier));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> onErrorReturnItem(final T item) {
        ObjectHelper.requireNonNull(item, "item is null");
        return onErrorReturn(Functions.justFunction(item));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> onExceptionResumeNext(final MaybeSource<? extends T> next) {
        ObjectHelper.requireNonNull(next, "next is null");
        return RxJavaPlugins.onAssembly(new MaybeOnErrorNext<T>(this, Functions.justFunction(next), false));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> onTerminateDetach() {
        return RxJavaPlugins.onAssembly(new MaybeDetach<T>(this));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Flowable<T> repeat() {
        return repeat(Long.MAX_VALUE);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Flowable<T> repeat(long times) {
        return toFlowable().repeat(times);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Flowable<T> repeatUntil(BooleanSupplier stop) {
        return toFlowable().repeatUntil(stop);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Flowable<T> repeatWhen(final Function<? super Flowable<Object>, ? extends Publisher<?>> handler) {
        return toFlowable().repeatWhen(handler);
    }


    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> retry() {
        return retry(Long.MAX_VALUE, Functions.alwaysTrue());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> retry(BiPredicate<? super Integer, ? super Throwable> predicate) {
        return toFlowable().retry(predicate).singleElement();
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> retry(long count) {
        return retry(count, Functions.alwaysTrue());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> retry(long times, Predicate<? super Throwable> predicate) {
        return toFlowable().retry(times, predicate).singleElement();
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> retry(Predicate<? super Throwable> predicate) {
        return retry(Long.MAX_VALUE, predicate);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> retryUntil(final BooleanSupplier stop) {
        ObjectHelper.requireNonNull(stop, "stop is null");
        return retry(Long.MAX_VALUE, Functions.predicateReverseFor(stop));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> retryWhen(
            final Function<? super Flowable<Throwable>, ? extends Publisher<?>> handler) {
        return toFlowable().retryWhen(handler).singleElement();
    }

    
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Disposable subscribe() {
        return subscribe(Functions.emptyConsumer(), Functions.ON_ERROR_MISSING, Functions.EMPTY_ACTION);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Disposable subscribe(Consumer<? super T> onSuccess) {
        return subscribe(onSuccess, Functions.ON_ERROR_MISSING, Functions.EMPTY_ACTION);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Disposable subscribe(Consumer<? super T> onSuccess, Consumer<? super Throwable> onError) {
        return subscribe(onSuccess, onError, Functions.EMPTY_ACTION);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Disposable subscribe(Consumer<? super T> onSuccess, Consumer<? super Throwable> onError,
            Action onComplete) {
        ObjectHelper.requireNonNull(onSuccess, "onSuccess is null");
        ObjectHelper.requireNonNull(onError, "onError is null");
        ObjectHelper.requireNonNull(onComplete, "onComplete is null");
        return subscribeWith(new MaybeCallbackObserver<T>(onSuccess, onError, onComplete));
    }

    @SchedulerSupport(SchedulerSupport.NONE)
    @Override
    public final void subscribe(MaybeObserver<? super T> observer) {
        ObjectHelper.requireNonNull(observer, "observer is null");

        observer = RxJavaPlugins.onSubscribe(this, observer);

        ObjectHelper.requireNonNull(observer, "observer returned by the RxJavaPlugins hook is null");

        try {
            subscribeActual(observer);
        } catch (NullPointerException ex) {
            throw ex;
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            NullPointerException npe = new NullPointerException("subscribeActual failed");
            npe.initCause(ex);
            throw npe;
        }
    }

    
    protected abstract void subscribeActual(MaybeObserver<? super T> observer);

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Maybe<T> subscribeOn(Scheduler scheduler) {
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new MaybeSubscribeOn<T>(this, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <E extends MaybeObserver<? super T>> E subscribeWith(E observer) {
        subscribe(observer);
        return observer;
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> switchIfEmpty(MaybeSource<? extends T> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return RxJavaPlugins.onAssembly(new MaybeSwitchIfEmpty<T>(this, other));
    }

    
    @Experimental
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> switchIfEmpty(SingleSource<? extends T> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return RxJavaPlugins.onAssembly(new MaybeSwitchIfEmptySingle<T>(this, other));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Maybe<T> takeUntil(MaybeSource<U> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return RxJavaPlugins.onAssembly(new MaybeTakeUntilMaybe<T, U>(this, other));
    }

    
    @BackpressureSupport(BackpressureKind.UNBOUNDED_IN)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Maybe<T> takeUntil(Publisher<U> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return RxJavaPlugins.onAssembly(new MaybeTakeUntilPublisher<T, U>(this, other));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Maybe<T> timeout(long timeout, TimeUnit timeUnit) {
        return timeout(timeout, timeUnit, Schedulers.computation());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Maybe<T> timeout(long timeout, TimeUnit timeUnit, MaybeSource<? extends T> fallback) {
        ObjectHelper.requireNonNull(fallback, "other is null");
        return timeout(timeout, timeUnit, Schedulers.computation(), fallback);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Maybe<T> timeout(long timeout, TimeUnit timeUnit, Scheduler scheduler, MaybeSource<? extends T> fallback) {
        ObjectHelper.requireNonNull(fallback, "fallback is null");
        return timeout(timer(timeout, timeUnit, scheduler), fallback);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Maybe<T> timeout(long timeout, TimeUnit timeUnit, Scheduler scheduler) {
        return timeout(timer(timeout, timeUnit, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Maybe<T> timeout(MaybeSource<U> timeoutIndicator) {
        ObjectHelper.requireNonNull(timeoutIndicator, "timeoutIndicator is null");
        return RxJavaPlugins.onAssembly(new MaybeTimeoutMaybe<T, U>(this, timeoutIndicator, null));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Maybe<T> timeout(MaybeSource<U> timeoutIndicator, MaybeSource<? extends T> fallback) {
        ObjectHelper.requireNonNull(timeoutIndicator, "timeoutIndicator is null");
        ObjectHelper.requireNonNull(fallback, "fallback is null");
        return RxJavaPlugins.onAssembly(new MaybeTimeoutMaybe<T, U>(this, timeoutIndicator, fallback));
    }

    
    @BackpressureSupport(BackpressureKind.UNBOUNDED_IN)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Maybe<T> timeout(Publisher<U> timeoutIndicator) {
        ObjectHelper.requireNonNull(timeoutIndicator, "timeoutIndicator is null");
        return RxJavaPlugins.onAssembly(new MaybeTimeoutPublisher<T, U>(this, timeoutIndicator, null));
    }

    
    @BackpressureSupport(BackpressureKind.UNBOUNDED_IN)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Maybe<T> timeout(Publisher<U> timeoutIndicator, MaybeSource<? extends T> fallback) {
        ObjectHelper.requireNonNull(timeoutIndicator, "timeoutIndicator is null");
        ObjectHelper.requireNonNull(fallback, "fallback is null");
        return RxJavaPlugins.onAssembly(new MaybeTimeoutPublisher<T, U>(this, timeoutIndicator, fallback));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Maybe<T> unsubscribeOn(final Scheduler scheduler) {
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new MaybeUnsubscribeOn<T>(this, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U, R> Maybe<R> zipWith(MaybeSource<? extends U> other, BiFunction<? super T, ? super U, ? extends R> zipper) {
        ObjectHelper.requireNonNull(other, "other is null");
        return zip(this, other, zipper);
    }

            
    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final TestObserver<T> test() {
        TestObserver<T> to = new TestObserver<T>();
        subscribe(to);
        return to;
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final TestObserver<T> test(boolean cancelled) {
        TestObserver<T> to = new TestObserver<T>();

        if (cancelled) {
            to.cancel();
        }

        subscribe(to);
        return to;
    }
}

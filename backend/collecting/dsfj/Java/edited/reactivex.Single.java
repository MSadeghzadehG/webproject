

package io.reactivex;

import java.util.NoSuchElementException;
import java.util.concurrent.*;

import org.reactivestreams.Publisher;

import io.reactivex.annotations.*;
import io.reactivex.disposables.Disposable;
import io.reactivex.exceptions.Exceptions;
import io.reactivex.functions.*;
import io.reactivex.internal.functions.*;
import io.reactivex.internal.fuseable.*;
import io.reactivex.internal.observers.*;
import io.reactivex.internal.operators.completable.*;
import io.reactivex.internal.operators.flowable.*;
import io.reactivex.internal.operators.maybe.*;
import io.reactivex.internal.operators.observable.*;
import io.reactivex.internal.operators.single.*;
import io.reactivex.internal.util.*;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.schedulers.Schedulers;


public abstract class Single<T> implements SingleSource<T> {

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> amb(final Iterable<? extends SingleSource<? extends T>> sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        return RxJavaPlugins.onAssembly(new SingleAmb<T>(null, sources));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Single<T> ambArray(final SingleSource<? extends T>... sources) {
        if (sources.length == 0) {
            return error(SingleInternalHelper.<T>emptyThrower());
        }
        if (sources.length == 1) {
            return wrap((SingleSource<T>)sources[0]);
        }
        return RxJavaPlugins.onAssembly(new SingleAmb<T>(sources, null));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @BackpressureSupport(BackpressureKind.FULL)
    public static <T> Flowable<T> concat(Iterable<? extends SingleSource<? extends T>> sources) {
        return concat(Flowable.fromIterable(sources));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings({ "unchecked", "rawtypes" })
    public static <T> Observable<T> concat(ObservableSource<? extends SingleSource<? extends T>> sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        return RxJavaPlugins.onAssembly(new ObservableConcatMap(sources, SingleInternalHelper.toObservable(), 2, ErrorMode.IMMEDIATE));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> concat(Publisher<? extends SingleSource<? extends T>> sources) {
        return concat(sources, 2);
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings({ "unchecked", "rawtypes" })
    public static <T> Flowable<T> concat(Publisher<? extends SingleSource<? extends T>> sources, int prefetch) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        ObjectHelper.verifyPositive(prefetch, "prefetch");
        return RxJavaPlugins.onAssembly(new FlowableConcatMapPublisher(sources, SingleInternalHelper.toFlowable(), prefetch, ErrorMode.IMMEDIATE));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> concat(
            SingleSource<? extends T> source1, SingleSource<? extends T> source2
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        return concat(Flowable.fromArray(source1, source2));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> concat(
            SingleSource<? extends T> source1, SingleSource<? extends T> source2,
            SingleSource<? extends T> source3
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        return concat(Flowable.fromArray(source1, source2, source3));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> concat(
            SingleSource<? extends T> source1, SingleSource<? extends T> source2,
            SingleSource<? extends T> source3, SingleSource<? extends T> source4
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        return concat(Flowable.fromArray(source1, source2, source3, source4));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings({ "unchecked", "rawtypes" })
    public static <T> Flowable<T> concatArray(SingleSource<? extends T>... sources) {
        return RxJavaPlugins.onAssembly(new FlowableConcatMap(Flowable.fromArray(sources), SingleInternalHelper.toFlowable(), 2, ErrorMode.BOUNDARY));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> create(SingleOnSubscribe<T> source) {
        ObjectHelper.requireNonNull(source, "source is null");
        return RxJavaPlugins.onAssembly(new SingleCreate<T>(source));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> defer(final Callable<? extends SingleSource<? extends T>> singleSupplier) {
        ObjectHelper.requireNonNull(singleSupplier, "singleSupplier is null");
        return RxJavaPlugins.onAssembly(new SingleDefer<T>(singleSupplier));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> error(final Callable<? extends Throwable> errorSupplier) {
        ObjectHelper.requireNonNull(errorSupplier, "errorSupplier is null");
        return RxJavaPlugins.onAssembly(new SingleError<T>(errorSupplier));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> error(final Throwable exception) {
        ObjectHelper.requireNonNull(exception, "error is null");
        return error(Functions.justCallable(exception));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> fromCallable(final Callable<? extends T> callable) {
        ObjectHelper.requireNonNull(callable, "callable is null");
        return RxJavaPlugins.onAssembly(new SingleFromCallable<T>(callable));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> fromFuture(Future<? extends T> future) {
        return toSingle(Flowable.<T>fromFuture(future));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> fromFuture(Future<? extends T> future, long timeout, TimeUnit unit) {
        return toSingle(Flowable.<T>fromFuture(future, timeout, unit));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public static <T> Single<T> fromFuture(Future<? extends T> future, long timeout, TimeUnit unit, Scheduler scheduler) {
        return toSingle(Flowable.<T>fromFuture(future, timeout, unit, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public static <T> Single<T> fromFuture(Future<? extends T> future, Scheduler scheduler) {
        return toSingle(Flowable.<T>fromFuture(future, scheduler));
    }

    
    @BackpressureSupport(BackpressureKind.UNBOUNDED_IN)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> fromPublisher(final Publisher<? extends T> publisher) {
        ObjectHelper.requireNonNull(publisher, "publisher is null");
        return RxJavaPlugins.onAssembly(new SingleFromPublisher<T>(publisher));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> fromObservable(ObservableSource<? extends T> observableSource) {
        ObjectHelper.requireNonNull(observableSource, "observableSource is null");
        return RxJavaPlugins.onAssembly(new ObservableSingleSingle<T>(observableSource, null));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> just(final T item) {
        ObjectHelper.requireNonNull(item, "value is null");
        return RxJavaPlugins.onAssembly(new SingleJust<T>(item));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Flowable<T> merge(Iterable<? extends SingleSource<? extends T>> sources) {
        return merge(Flowable.fromIterable(sources));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings({ "unchecked", "rawtypes" })
    public static <T> Flowable<T> merge(Publisher<? extends SingleSource<? extends T>> sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        return RxJavaPlugins.onAssembly(new FlowableFlatMapPublisher(sources, SingleInternalHelper.toFlowable(), false, Integer.MAX_VALUE, Flowable.bufferSize()));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings({ "unchecked", "rawtypes" })
    public static <T> Single<T> merge(SingleSource<? extends SingleSource<? extends T>> source) {
        ObjectHelper.requireNonNull(source, "source is null");
        return RxJavaPlugins.onAssembly(new SingleFlatMap<SingleSource<? extends T>, T>(source, (Function)Functions.identity()));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> merge(
            SingleSource<? extends T> source1, SingleSource<? extends T> source2
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        return merge(Flowable.fromArray(source1, source2));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> merge(
            SingleSource<? extends T> source1, SingleSource<? extends T> source2,
            SingleSource<? extends T> source3
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        return merge(Flowable.fromArray(source1, source2, source3));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Flowable<T> merge(
            SingleSource<? extends T> source1, SingleSource<? extends T> source2,
            SingleSource<? extends T> source3, SingleSource<? extends T> source4
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        return merge(Flowable.fromArray(source1, source2, source3, source4));
    }


    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @Experimental
    public static <T> Flowable<T> mergeDelayError(Iterable<? extends SingleSource<? extends T>> sources) {
        return mergeDelayError(Flowable.fromIterable(sources));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings({ "unchecked", "rawtypes" })
    @Experimental
    public static <T> Flowable<T> mergeDelayError(Publisher<? extends SingleSource<? extends T>> sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        return RxJavaPlugins.onAssembly(new FlowableFlatMapPublisher(sources, SingleInternalHelper.toFlowable(), true, Integer.MAX_VALUE, Flowable.bufferSize()));
    }


    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    @Experimental
    public static <T> Flowable<T> mergeDelayError(
            SingleSource<? extends T> source1, SingleSource<? extends T> source2
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        return mergeDelayError(Flowable.fromArray(source1, source2));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    @Experimental
    public static <T> Flowable<T> mergeDelayError(
            SingleSource<? extends T> source1, SingleSource<? extends T> source2,
            SingleSource<? extends T> source3
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        return mergeDelayError(Flowable.fromArray(source1, source2, source3));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    @Experimental
    public static <T> Flowable<T> mergeDelayError(
            SingleSource<? extends T> source1, SingleSource<? extends T> source2,
            SingleSource<? extends T> source3, SingleSource<? extends T> source4
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        return mergeDelayError(Flowable.fromArray(source1, source2, source3, source4));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T> Single<T> never() {
        return RxJavaPlugins.onAssembly((Single<T>) SingleNever.INSTANCE);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public static Single<Long> timer(long delay, TimeUnit unit) {
        return timer(delay, unit, Schedulers.computation());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public static Single<Long> timer(final long delay, final TimeUnit unit, final Scheduler scheduler) {
        ObjectHelper.requireNonNull(unit, "unit is null");
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new SingleTimer(delay, unit, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<Boolean> equals(final SingleSource<? extends T> first, final SingleSource<? extends T> second) {         ObjectHelper.requireNonNull(first, "first is null");
        ObjectHelper.requireNonNull(second, "second is null");
        return RxJavaPlugins.onAssembly(new SingleEquals<T>(first, second));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> unsafeCreate(SingleSource<T> onSubscribe) {
        ObjectHelper.requireNonNull(onSubscribe, "onSubscribe is null");
        if (onSubscribe instanceof Single) {
            throw new IllegalArgumentException("unsafeCreate(Single) should be upgraded");
        }
        return RxJavaPlugins.onAssembly(new SingleFromUnsafeSource<T>(onSubscribe));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T, U> Single<T> using(Callable<U> resourceSupplier,
                                         Function<? super U, ? extends SingleSource<? extends T>> singleFunction,
                                         Consumer<? super U> disposer) {
        return using(resourceSupplier, singleFunction, disposer, true);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T, U> Single<T> using(
            final Callable<U> resourceSupplier,
            final Function<? super U, ? extends SingleSource<? extends T>> singleFunction,
            final Consumer<? super U> disposer,
            final boolean eager) {
        ObjectHelper.requireNonNull(resourceSupplier, "resourceSupplier is null");
        ObjectHelper.requireNonNull(singleFunction, "singleFunction is null");
        ObjectHelper.requireNonNull(disposer, "disposer is null");

        return RxJavaPlugins.onAssembly(new SingleUsing<T, U>(resourceSupplier, singleFunction, disposer, eager));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Single<T> wrap(SingleSource<T> source) {
        ObjectHelper.requireNonNull(source, "source is null");
        if (source instanceof Single) {
            return RxJavaPlugins.onAssembly((Single<T>)source);
        }
        return RxJavaPlugins.onAssembly(new SingleFromUnsafeSource<T>(source));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T, R> Single<R> zip(final Iterable<? extends SingleSource<? extends T>> sources, Function<? super Object[], ? extends R> zipper) {
        ObjectHelper.requireNonNull(zipper, "zipper is null");
        ObjectHelper.requireNonNull(sources, "sources is null");
        return RxJavaPlugins.onAssembly(new SingleZipIterable<T, R>(sources, zipper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T1, T2, R> Single<R> zip(
            SingleSource<? extends T1> source1, SingleSource<? extends T2> source2,
            BiFunction<? super T1, ? super T2, ? extends R> zipper
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T1, T2, T3, R> Single<R> zip(
            SingleSource<? extends T1> source1, SingleSource<? extends T2> source2,
            SingleSource<? extends T3> source3,
            Function3<? super T1, ? super T2, ? super T3, ? extends R> zipper
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T1, T2, T3, T4, R> Single<R> zip(
            SingleSource<? extends T1> source1, SingleSource<? extends T2> source2,
            SingleSource<? extends T3> source3, SingleSource<? extends T4> source4,
            Function4<? super T1, ? super T2, ? super T3, ? super T4, ? extends R> zipper
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3, source4);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T1, T2, T3, T4, T5, R> Single<R> zip(
            SingleSource<? extends T1> source1, SingleSource<? extends T2> source2,
            SingleSource<? extends T3> source3, SingleSource<? extends T4> source4,
            SingleSource<? extends T5> source5,
            Function5<? super T1, ? super T2, ? super T3, ? super T4, ? super T5, ? extends R> zipper
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        ObjectHelper.requireNonNull(source5, "source5 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3, source4, source5);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T1, T2, T3, T4, T5, T6, R> Single<R> zip(
            SingleSource<? extends T1> source1, SingleSource<? extends T2> source2,
            SingleSource<? extends T3> source3, SingleSource<? extends T4> source4,
            SingleSource<? extends T5> source5, SingleSource<? extends T6> source6,
            Function6<? super T1, ? super T2, ? super T3, ? super T4, ? super T5, ? super T6, ? extends R> zipper
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        ObjectHelper.requireNonNull(source5, "source5 is null");
        ObjectHelper.requireNonNull(source6, "source6 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3, source4, source5, source6);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T1, T2, T3, T4, T5, T6, T7, R> Single<R> zip(
            SingleSource<? extends T1> source1, SingleSource<? extends T2> source2,
            SingleSource<? extends T3> source3, SingleSource<? extends T4> source4,
            SingleSource<? extends T5> source5, SingleSource<? extends T6> source6,
            SingleSource<? extends T7> source7,
            Function7<? super T1, ? super T2, ? super T3, ? super T4, ? super T5, ? super T6, ? super T7, ? extends R> zipper
     ) {
        ObjectHelper.requireNonNull(source1, "source1 is null");
        ObjectHelper.requireNonNull(source2, "source2 is null");
        ObjectHelper.requireNonNull(source3, "source3 is null");
        ObjectHelper.requireNonNull(source4, "source4 is null");
        ObjectHelper.requireNonNull(source5, "source5 is null");
        ObjectHelper.requireNonNull(source6, "source6 is null");
        ObjectHelper.requireNonNull(source7, "source7 is null");
        return zipArray(Functions.toFunction(zipper), source1, source2, source3, source4, source5, source6, source7);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T1, T2, T3, T4, T5, T6, T7, T8, R> Single<R> zip(
            SingleSource<? extends T1> source1, SingleSource<? extends T2> source2,
            SingleSource<? extends T3> source3, SingleSource<? extends T4> source4,
            SingleSource<? extends T5> source5, SingleSource<? extends T6> source6,
            SingleSource<? extends T7> source7, SingleSource<? extends T8> source8,
            Function8<? super T1, ? super T2, ? super T3, ? super T4, ? super T5, ? super T6, ? super T7, ? super T8, ? extends R> zipper
     ) {
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

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public static <T1, T2, T3, T4, T5, T6, T7, T8, T9, R> Single<R> zip(
            SingleSource<? extends T1> source1, SingleSource<? extends T2> source2,
            SingleSource<? extends T3> source3, SingleSource<? extends T4> source4,
            SingleSource<? extends T5> source5, SingleSource<? extends T6> source6,
            SingleSource<? extends T7> source7, SingleSource<? extends T8> source8,
            SingleSource<? extends T9> source9,
            Function9<? super T1, ? super T2, ? super T3, ? super T4, ? super T5, ? super T6, ? super T7, ? super T8, ? super T9, ? extends R> zipper
     ) {
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
    public static <T, R> Single<R> zipArray(Function<? super Object[], ? extends R> zipper, SingleSource<? extends T>... sources) {
        ObjectHelper.requireNonNull(zipper, "zipper is null");
        ObjectHelper.requireNonNull(sources, "sources is null");
        if (sources.length == 0) {
            return error(new NoSuchElementException());
        }
        return RxJavaPlugins.onAssembly(new SingleZipArray<T, R>(sources, zipper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public final Single<T> ambWith(SingleSource<? extends T> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return ambArray(this, other);
    }

    
    @Experimental
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> R as(@NonNull SingleConverter<T, ? extends R> converter) {
        return ObjectHelper.requireNonNull(converter, "converter is null").apply(this);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> hide() {
        return RxJavaPlugins.onAssembly(new SingleHide<T>(this));
    }

    
    @SuppressWarnings("unchecked")
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Single<R> compose(SingleTransformer<? super T, ? extends R> transformer) {
        return wrap(((SingleTransformer<T, R>) ObjectHelper.requireNonNull(transformer, "transformer is null")).apply(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> cache() {
        return RxJavaPlugins.onAssembly(new SingleCache<T>(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Single<U> cast(final Class<? extends U> clazz) {
        ObjectHelper.requireNonNull(clazz, "clazz is null");
        return map(Functions.castFunction(clazz));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Flowable<T> concatWith(SingleSource<? extends T> other) {
        return concat(this, other);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Single<T> delay(long time, TimeUnit unit) {
        return delay(time, unit, Schedulers.computation(), false);
    }

    
    @Experimental
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Single<T> delay(long time, TimeUnit unit, boolean delayError) {
        return delay(time, unit, Schedulers.computation(), delayError);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Single<T> delay(final long time, final TimeUnit unit, final Scheduler scheduler) {
        return delay(time, unit, scheduler, false);
    }

    
    @Experimental
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Single<T> delay(final long time, final TimeUnit unit, final Scheduler scheduler, boolean delayError) {
        ObjectHelper.requireNonNull(unit, "unit is null");
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new SingleDelay<T>(this, time, unit, scheduler, delayError));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> delaySubscription(CompletableSource other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return RxJavaPlugins.onAssembly(new SingleDelayWithCompletable<T>(this, other));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Single<T> delaySubscription(SingleSource<U> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return RxJavaPlugins.onAssembly(new SingleDelayWithSingle<T, U>(this, other));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Single<T> delaySubscription(ObservableSource<U> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return RxJavaPlugins.onAssembly(new SingleDelayWithObservable<T, U>(this, other));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Single<T> delaySubscription(Publisher<U> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return RxJavaPlugins.onAssembly(new SingleDelayWithPublisher<T, U>(this, other));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Single<T> delaySubscription(long time, TimeUnit unit) {
        return delaySubscription(time, unit, Schedulers.computation());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Single<T> delaySubscription(long time, TimeUnit unit, Scheduler scheduler) {
        return delaySubscription(Observable.timer(time, unit, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> doAfterSuccess(Consumer<? super T> onAfterSuccess) {
        ObjectHelper.requireNonNull(onAfterSuccess, "doAfterSuccess is null");
        return RxJavaPlugins.onAssembly(new SingleDoAfterSuccess<T>(this, onAfterSuccess));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> doAfterTerminate(Action onAfterTerminate) {
        ObjectHelper.requireNonNull(onAfterTerminate, "onAfterTerminate is null");
        return RxJavaPlugins.onAssembly(new SingleDoAfterTerminate<T>(this, onAfterTerminate));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> doFinally(Action onFinally) {
        ObjectHelper.requireNonNull(onFinally, "onFinally is null");
        return RxJavaPlugins.onAssembly(new SingleDoFinally<T>(this, onFinally));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> doOnSubscribe(final Consumer<? super Disposable> onSubscribe) {
        ObjectHelper.requireNonNull(onSubscribe, "onSubscribe is null");
        return RxJavaPlugins.onAssembly(new SingleDoOnSubscribe<T>(this, onSubscribe));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> doOnSuccess(final Consumer<? super T> onSuccess) {
        ObjectHelper.requireNonNull(onSuccess, "onSuccess is null");
        return RxJavaPlugins.onAssembly(new SingleDoOnSuccess<T>(this, onSuccess));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> doOnEvent(final BiConsumer<? super T, ? super Throwable> onEvent) {
        ObjectHelper.requireNonNull(onEvent, "onEvent is null");
        return RxJavaPlugins.onAssembly(new SingleDoOnEvent<T>(this, onEvent));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> doOnError(final Consumer<? super Throwable> onError) {
        ObjectHelper.requireNonNull(onError, "onError is null");
        return RxJavaPlugins.onAssembly(new SingleDoOnError<T>(this, onError));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> doOnDispose(final Action onDispose) {
        ObjectHelper.requireNonNull(onDispose, "onDispose is null");
        return RxJavaPlugins.onAssembly(new SingleDoOnDispose<T>(this, onDispose));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Maybe<T> filter(Predicate<? super T> predicate) {
        ObjectHelper.requireNonNull(predicate, "predicate is null");
        return RxJavaPlugins.onAssembly(new MaybeFilterSingle<T>(this, predicate));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Single<R> flatMap(Function<? super T, ? extends SingleSource<? extends R>> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new SingleFlatMap<T, R>(this, mapper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Maybe<R> flatMapMaybe(final Function<? super T, ? extends MaybeSource<? extends R>> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new SingleFlatMapMaybe<T, R>(this, mapper));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Flowable<R> flatMapPublisher(Function<? super T, ? extends Publisher<? extends R>> mapper) {
        return toFlowable().flatMap(mapper);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Flowable<U> flattenAsFlowable(final Function<? super T, ? extends Iterable<? extends U>> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new SingleFlatMapIterableFlowable<T, U>(this, mapper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> Observable<U> flattenAsObservable(final Function<? super T, ? extends Iterable<? extends U>> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new SingleFlatMapIterableObservable<T, U>(this, mapper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Observable<R> flatMapObservable(Function<? super T, ? extends ObservableSource<? extends R>> mapper) {
        return toObservable().flatMap(mapper);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable flatMapCompletable(final Function<? super T, ? extends CompletableSource> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new SingleFlatMapCompletable<T>(this, mapper));
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
    public final <R> Single<R> lift(final SingleOperator<? extends R, ? super T> lift) {
        ObjectHelper.requireNonNull(lift, "onLift is null");
        return RxJavaPlugins.onAssembly(new SingleLift<T, R>(this, lift));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> Single<R> map(Function<? super T, ? extends R> mapper) {
        ObjectHelper.requireNonNull(mapper, "mapper is null");
        return RxJavaPlugins.onAssembly(new SingleMap<T, R>(this, mapper));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<Boolean> contains(Object value) {
        return contains(value, ObjectHelper.equalsPredicate());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<Boolean> contains(final Object value, final BiPredicate<Object, Object> comparer) {
        ObjectHelper.requireNonNull(value, "value is null");
        ObjectHelper.requireNonNull(comparer, "comparer is null");
        return RxJavaPlugins.onAssembly(new SingleContains<T>(this, value, comparer));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Flowable<T> mergeWith(SingleSource<? extends T> other) {
        return merge(this, other);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Single<T> observeOn(final Scheduler scheduler) {
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new SingleObserveOn<T>(this, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> onErrorReturn(final Function<Throwable, ? extends T> resumeFunction) {
        ObjectHelper.requireNonNull(resumeFunction, "resumeFunction is null");
        return RxJavaPlugins.onAssembly(new SingleOnErrorReturn<T>(this, resumeFunction, null));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> onErrorReturnItem(final T value) {
        ObjectHelper.requireNonNull(value, "value is null");
        return RxJavaPlugins.onAssembly(new SingleOnErrorReturn<T>(this, null, value));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> onErrorResumeNext(final Single<? extends T> resumeSingleInCaseOfError) {
        ObjectHelper.requireNonNull(resumeSingleInCaseOfError, "resumeSingleInCaseOfError is null");
        return onErrorResumeNext(Functions.justFunction(resumeSingleInCaseOfError));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> onErrorResumeNext(
            final Function<? super Throwable, ? extends SingleSource<? extends T>> resumeFunctionInCaseOfError) {
        ObjectHelper.requireNonNull(resumeFunctionInCaseOfError, "resumeFunctionInCaseOfError is null");
        return RxJavaPlugins.onAssembly(new SingleResumeNext<T>(this, resumeFunctionInCaseOfError));
    }

    
    @Experimental
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> onTerminateDetach() {
        return RxJavaPlugins.onAssembly(new SingleDetach<T>(this));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Flowable<T> repeat() {
        return toFlowable().repeat();
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
    public final Flowable<T> repeatWhen(Function<? super Flowable<Object>, ? extends Publisher<?>> handler) {
        return toFlowable().repeatWhen(handler);
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Flowable<T> repeatUntil(BooleanSupplier stop) {
        return toFlowable().repeatUntil(stop);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> retry() {
        return toSingle(toFlowable().retry());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> retry(long times) {
        return toSingle(toFlowable().retry(times));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> retry(BiPredicate<? super Integer, ? super Throwable> predicate) {
        return toSingle(toFlowable().retry(predicate));
    }

    
    @Experimental
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> retry(long times, Predicate<? super Throwable> predicate) {
        return toSingle(toFlowable().retry(times, predicate));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> retry(Predicate<? super Throwable> predicate) {
        return toSingle(toFlowable().retry(predicate));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> retryWhen(Function<? super Flowable<Throwable>, ? extends Publisher<?>> handler) {
        return toSingle(toFlowable().retryWhen(handler));
    }

    
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Disposable subscribe() {
        return subscribe(Functions.emptyConsumer(), Functions.ON_ERROR_MISSING);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Disposable subscribe(final BiConsumer<? super T, ? super Throwable> onCallback) {
        ObjectHelper.requireNonNull(onCallback, "onCallback is null");

        BiConsumerSingleObserver<T> s = new BiConsumerSingleObserver<T>(onCallback);
        subscribe(s);
        return s;
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Disposable subscribe(Consumer<? super T> onSuccess) {
        return subscribe(onSuccess, Functions.ON_ERROR_MISSING);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Disposable subscribe(final Consumer<? super T> onSuccess, final Consumer<? super Throwable> onError) {
        ObjectHelper.requireNonNull(onSuccess, "onSuccess is null");
        ObjectHelper.requireNonNull(onError, "onError is null");

        ConsumerSingleObserver<T> s = new ConsumerSingleObserver<T>(onSuccess, onError);
        subscribe(s);
        return s;
    }

    @SchedulerSupport(SchedulerSupport.NONE)
    @Override
    public final void subscribe(SingleObserver<? super T> subscriber) {
        ObjectHelper.requireNonNull(subscriber, "subscriber is null");

        subscriber = RxJavaPlugins.onSubscribe(this, subscriber);

        ObjectHelper.requireNonNull(subscriber, "subscriber returned by the RxJavaPlugins hook is null");

        try {
            subscribeActual(subscriber);
        } catch (NullPointerException ex) {
            throw ex;
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            NullPointerException npe = new NullPointerException("subscribeActual failed");
            npe.initCause(ex);
            throw npe;
        }
    }

    
    protected abstract void subscribeActual(@NonNull SingleObserver<? super T> observer);

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <E extends SingleObserver<? super T>> E subscribeWith(E observer) {
        subscribe(observer);
        return observer;
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Single<T> subscribeOn(final Scheduler scheduler) {
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new SingleSubscribeOn<T>(this, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Single<T> takeUntil(final CompletableSource other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return takeUntil(new CompletableToFlowable<T>(other));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <E> Single<T> takeUntil(final Publisher<E> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return RxJavaPlugins.onAssembly(new SingleTakeUntil<T, E>(this, other));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <E> Single<T> takeUntil(final SingleSource<? extends E> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return takeUntil(new SingleToFlowable<E>(other));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Single<T> timeout(long timeout, TimeUnit unit) {
        return timeout0(timeout, unit, Schedulers.computation(), null);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Single<T> timeout(long timeout, TimeUnit unit, Scheduler scheduler) {
        return timeout0(timeout, unit, scheduler, null);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Single<T> timeout(long timeout, TimeUnit unit, Scheduler scheduler, SingleSource<? extends T> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return timeout0(timeout, unit, scheduler, other);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Single<T> timeout(long timeout, TimeUnit unit, SingleSource<? extends T> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return timeout0(timeout, unit, Schedulers.computation(), other);
    }

    private Single<T> timeout0(final long timeout, final TimeUnit unit, final Scheduler scheduler, final SingleSource<? extends T> other) {
        ObjectHelper.requireNonNull(unit, "unit is null");
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new SingleTimeout<T>(this, timeout, unit, scheduler, other));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> R to(Function<? super Single<T>, R> convert) {
        try {
            return ObjectHelper.requireNonNull(convert, "convert is null").apply(this);
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            throw ExceptionHelper.wrapOrThrow(ex);
        }
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable toCompletable() {
        return RxJavaPlugins.onAssembly(new CompletableFromSingle<T>(this));
    }

    
    @BackpressureSupport(BackpressureKind.FULL)
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public final Flowable<T> toFlowable() {
        if (this instanceof FuseToFlowable) {
            return ((FuseToFlowable<T>)this).fuseToFlowable();
        }
        return RxJavaPlugins.onAssembly(new SingleToFlowable<T>(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Future<T> toFuture() {
        return subscribeWith(new FutureSingleObserver<T>());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public final Maybe<T> toMaybe() {
        if (this instanceof FuseToMaybe) {
            return ((FuseToMaybe<T>)this).fuseToMaybe();
        }
        return RxJavaPlugins.onAssembly(new MaybeFromSingle<T>(this));
    }
    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @SuppressWarnings("unchecked")
    public final Observable<T> toObservable() {
        if (this instanceof FuseToObservable) {
            return ((FuseToObservable<T>)this).fuseToObservable();
        }
        return RxJavaPlugins.onAssembly(new SingleToObservable<T>(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    @Experimental
    public final Single<T> unsubscribeOn(final Scheduler scheduler) {
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new SingleUnsubscribeOn<T>(this, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U, R> Single<R> zipWith(SingleSource<U> other, BiFunction<? super T, ? super U, ? extends R> zipper) {
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

    private static <T> Single<T> toSingle(Flowable<T> source) {
        return RxJavaPlugins.onAssembly(new FlowableSingleSingle<T>(source, null));
    }
}

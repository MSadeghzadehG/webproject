
package io.reactivex;

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
import io.reactivex.internal.operators.flowable.FlowableDelaySubscriptionOther;
import io.reactivex.internal.operators.maybe.*;
import io.reactivex.internal.operators.observable.ObservableDelaySubscriptionOther;
import io.reactivex.internal.operators.single.SingleDelayWithCompletable;
import io.reactivex.internal.util.ExceptionHelper;
import io.reactivex.observers.TestObserver;
import io.reactivex.plugins.RxJavaPlugins;
import io.reactivex.schedulers.Schedulers;


public abstract class Completable implements CompletableSource {
    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable ambArray(final CompletableSource... sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        if (sources.length == 0) {
            return complete();
        }
        if (sources.length == 1) {
            return wrap(sources[0]);
        }

        return RxJavaPlugins.onAssembly(new CompletableAmb(sources, null));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable amb(final Iterable<? extends CompletableSource> sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");

        return RxJavaPlugins.onAssembly(new CompletableAmb(null, sources));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable complete() {
        return RxJavaPlugins.onAssembly(CompletableEmpty.INSTANCE);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable concatArray(CompletableSource... sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        if (sources.length == 0) {
            return complete();
        } else
        if (sources.length == 1) {
            return wrap(sources[0]);
        }
        return RxJavaPlugins.onAssembly(new CompletableConcatArray(sources));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable concat(Iterable<? extends CompletableSource> sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");

        return RxJavaPlugins.onAssembly(new CompletableConcatIterable(sources));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @BackpressureSupport(BackpressureKind.FULL)
    public static Completable concat(Publisher<? extends CompletableSource> sources) {
        return concat(sources, 2);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @BackpressureSupport(BackpressureKind.FULL)
    public static Completable concat(Publisher<? extends CompletableSource> sources, int prefetch) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        ObjectHelper.verifyPositive(prefetch, "prefetch");
        return RxJavaPlugins.onAssembly(new CompletableConcat(sources, prefetch));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable create(CompletableOnSubscribe source) {
        ObjectHelper.requireNonNull(source, "source is null");
        return RxJavaPlugins.onAssembly(new CompletableCreate(source));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable unsafeCreate(CompletableSource source) {
        ObjectHelper.requireNonNull(source, "source is null");
        if (source instanceof Completable) {
            throw new IllegalArgumentException("Use of unsafeCreate(Completable)!");
        }
        return RxJavaPlugins.onAssembly(new CompletableFromUnsafeSource(source));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable defer(final Callable<? extends CompletableSource> completableSupplier) {
        ObjectHelper.requireNonNull(completableSupplier, "completableSupplier");
        return RxJavaPlugins.onAssembly(new CompletableDefer(completableSupplier));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable error(final Callable<? extends Throwable> errorSupplier) {
        ObjectHelper.requireNonNull(errorSupplier, "errorSupplier is null");
        return RxJavaPlugins.onAssembly(new CompletableErrorSupplier(errorSupplier));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable error(final Throwable error) {
        ObjectHelper.requireNonNull(error, "error is null");
        return RxJavaPlugins.onAssembly(new CompletableError(error));
    }


    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable fromAction(final Action run) {
        ObjectHelper.requireNonNull(run, "run is null");
        return RxJavaPlugins.onAssembly(new CompletableFromAction(run));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable fromCallable(final Callable<?> callable) {
        ObjectHelper.requireNonNull(callable, "callable is null");
        return RxJavaPlugins.onAssembly(new CompletableFromCallable(callable));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable fromFuture(final Future<?> future) {
        ObjectHelper.requireNonNull(future, "future is null");
        return fromAction(Functions.futureAction(future));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable fromRunnable(final Runnable run) {
        ObjectHelper.requireNonNull(run, "run is null");
        return RxJavaPlugins.onAssembly(new CompletableFromRunnable(run));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Completable fromObservable(final ObservableSource<T> observable) {
        ObjectHelper.requireNonNull(observable, "observable is null");
        return RxJavaPlugins.onAssembly(new CompletableFromObservable<T>(observable));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.UNBOUNDED_IN)
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Completable fromPublisher(final Publisher<T> publisher) {
        ObjectHelper.requireNonNull(publisher, "publisher is null");
        return RxJavaPlugins.onAssembly(new CompletableFromPublisher<T>(publisher));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <T> Completable fromSingle(final SingleSource<T> single) {
        ObjectHelper.requireNonNull(single, "single is null");
        return RxJavaPlugins.onAssembly(new CompletableFromSingle<T>(single));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable mergeArray(CompletableSource... sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        if (sources.length == 0) {
            return complete();
        } else
        if (sources.length == 1) {
            return wrap(sources[0]);
        }
        return RxJavaPlugins.onAssembly(new CompletableMergeArray(sources));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable merge(Iterable<? extends CompletableSource> sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        return RxJavaPlugins.onAssembly(new CompletableMergeIterable(sources));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @BackpressureSupport(BackpressureKind.UNBOUNDED_IN)
    public static Completable merge(Publisher<? extends CompletableSource> sources) {
        return merge0(sources, Integer.MAX_VALUE, false);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @BackpressureSupport(BackpressureKind.FULL)
    public static Completable merge(Publisher<? extends CompletableSource> sources, int maxConcurrency) {
        return merge0(sources, maxConcurrency, false);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @BackpressureSupport(BackpressureKind.FULL)
    private static Completable merge0(Publisher<? extends CompletableSource> sources, int maxConcurrency, boolean delayErrors) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        ObjectHelper.verifyPositive(maxConcurrency, "maxConcurrency");
        return RxJavaPlugins.onAssembly(new CompletableMerge(sources, maxConcurrency, delayErrors));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable mergeArrayDelayError(CompletableSource... sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        return RxJavaPlugins.onAssembly(new CompletableMergeDelayErrorArray(sources));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable mergeDelayError(Iterable<? extends CompletableSource> sources) {
        ObjectHelper.requireNonNull(sources, "sources is null");
        return RxJavaPlugins.onAssembly(new CompletableMergeDelayErrorIterable(sources));
    }


    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @BackpressureSupport(BackpressureKind.UNBOUNDED_IN)
    public static Completable mergeDelayError(Publisher<? extends CompletableSource> sources) {
        return merge0(sources, Integer.MAX_VALUE, true);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    @BackpressureSupport(BackpressureKind.FULL)
    public static Completable mergeDelayError(Publisher<? extends CompletableSource> sources, int maxConcurrency) {
        return merge0(sources, maxConcurrency, true);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable never() {
        return RxJavaPlugins.onAssembly(CompletableNever.INSTANCE);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public static Completable timer(long delay, TimeUnit unit) {
        return timer(delay, unit, Schedulers.computation());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public static Completable timer(final long delay, final TimeUnit unit, final Scheduler scheduler) {
        ObjectHelper.requireNonNull(unit, "unit is null");
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new CompletableTimer(delay, unit, scheduler));
    }

    
    private static NullPointerException toNpe(Throwable ex) {
        NullPointerException npe = new NullPointerException("Actually not, but can't pass out an exception otherwise...");
        npe.initCause(ex);
        return npe;
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <R> Completable using(Callable<R> resourceSupplier,
            Function<? super R, ? extends CompletableSource> completableFunction,
            Consumer<? super R> disposer) {
        return using(resourceSupplier, completableFunction, disposer, true);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static <R> Completable using(
            final Callable<R> resourceSupplier,
            final Function<? super R, ? extends CompletableSource> completableFunction,
            final Consumer<? super R> disposer,
            final boolean eager) {
        ObjectHelper.requireNonNull(resourceSupplier, "resourceSupplier is null");
        ObjectHelper.requireNonNull(completableFunction, "completableFunction is null");
        ObjectHelper.requireNonNull(disposer, "disposer is null");

        return RxJavaPlugins.onAssembly(new CompletableUsing<R>(resourceSupplier, completableFunction, disposer, eager));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public static Completable wrap(CompletableSource source) {
        ObjectHelper.requireNonNull(source, "source is null");
        if (source instanceof Completable) {
            return RxJavaPlugins.onAssembly((Completable)source);
        }
        return RxJavaPlugins.onAssembly(new CompletableFromUnsafeSource(source));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable ambWith(CompletableSource other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return ambArray(this, other);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <T> Observable<T> andThen(ObservableSource<T> next) {
        ObjectHelper.requireNonNull(next, "next is null");
        return RxJavaPlugins.onAssembly(new ObservableDelaySubscriptionOther<T, Object>(next, toObservable()));
    }

    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <T> Flowable<T> andThen(Publisher<T> next) {
        ObjectHelper.requireNonNull(next, "next is null");
        return RxJavaPlugins.onAssembly(new FlowableDelaySubscriptionOther<T, Object>(next, toFlowable()));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <T> Single<T> andThen(SingleSource<T> next) {
        ObjectHelper.requireNonNull(next, "next is null");
        return RxJavaPlugins.onAssembly(new SingleDelayWithCompletable<T>(next, this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <T> Maybe<T> andThen(MaybeSource<T> next) {
        ObjectHelper.requireNonNull(next, "next is null");
        return RxJavaPlugins.onAssembly(new MaybeDelayWithCompletable<T>(next, this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable andThen(CompletableSource next) {
        return concatWith(next);
    }

    
    @Experimental
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <R> R as(@NonNull CompletableConverter<? extends R> converter) {
        return ObjectHelper.requireNonNull(converter, "converter is null").apply(this);
    }

    
    @SchedulerSupport(SchedulerSupport.NONE)
    public final void blockingAwait() {
        BlockingMultiObserver<Void> observer = new BlockingMultiObserver<Void>();
        subscribe(observer);
        observer.blockingGet();
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final boolean blockingAwait(long timeout, TimeUnit unit) {
        ObjectHelper.requireNonNull(unit, "unit is null");
        BlockingMultiObserver<Void> observer = new BlockingMultiObserver<Void>();
        subscribe(observer);
        return observer.blockingAwait(timeout, unit);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Throwable blockingGet() {
        BlockingMultiObserver<Void> observer = new BlockingMultiObserver<Void>();
        subscribe(observer);
        return observer.blockingGetError();
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Throwable blockingGet(long timeout, TimeUnit unit) {
        ObjectHelper.requireNonNull(unit, "unit is null");
        BlockingMultiObserver<Void> observer = new BlockingMultiObserver<Void>();
        subscribe(observer);
        return observer.blockingGetError(timeout, unit);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable cache() {
        return RxJavaPlugins.onAssembly(new CompletableCache(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable compose(CompletableTransformer transformer) {
        return wrap(ObjectHelper.requireNonNull(transformer, "transformer is null").apply(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable concatWith(CompletableSource other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return concatArray(this, other);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Completable delay(long delay, TimeUnit unit) {
        return delay(delay, unit, Schedulers.computation(), false);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Completable delay(long delay, TimeUnit unit, Scheduler scheduler) {
        return delay(delay, unit, scheduler, false);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Completable delay(final long delay, final TimeUnit unit, final Scheduler scheduler, final boolean delayError) {
        ObjectHelper.requireNonNull(unit, "unit is null");
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new CompletableDelay(this, delay, unit, scheduler, delayError));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable doOnComplete(Action onComplete) {
        return doOnLifecycle(Functions.emptyConsumer(), Functions.emptyConsumer(),
                onComplete, Functions.EMPTY_ACTION,
                Functions.EMPTY_ACTION, Functions.EMPTY_ACTION);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable doOnDispose(Action onDispose) {
        return doOnLifecycle(Functions.emptyConsumer(), Functions.emptyConsumer(),
                Functions.EMPTY_ACTION, Functions.EMPTY_ACTION,
                Functions.EMPTY_ACTION, onDispose);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable doOnError(Consumer<? super Throwable> onError) {
        return doOnLifecycle(Functions.emptyConsumer(), onError,
                Functions.EMPTY_ACTION, Functions.EMPTY_ACTION,
                Functions.EMPTY_ACTION, Functions.EMPTY_ACTION);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable doOnEvent(final Consumer<? super Throwable> onEvent) {
        ObjectHelper.requireNonNull(onEvent, "onEvent is null");
        return RxJavaPlugins.onAssembly(new CompletableDoOnEvent(this, onEvent));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    private Completable doOnLifecycle(
            final Consumer<? super Disposable> onSubscribe,
            final Consumer<? super Throwable> onError,
            final Action onComplete,
            final Action onTerminate,
            final Action onAfterTerminate,
            final Action onDispose) {
        ObjectHelper.requireNonNull(onSubscribe, "onSubscribe is null");
        ObjectHelper.requireNonNull(onError, "onError is null");
        ObjectHelper.requireNonNull(onComplete, "onComplete is null");
        ObjectHelper.requireNonNull(onTerminate, "onTerminate is null");
        ObjectHelper.requireNonNull(onAfterTerminate, "onAfterTerminate is null");
        ObjectHelper.requireNonNull(onDispose, "onDispose is null");
        return RxJavaPlugins.onAssembly(new CompletablePeek(this, onSubscribe, onError, onComplete, onTerminate, onAfterTerminate, onDispose));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable doOnSubscribe(Consumer<? super Disposable> onSubscribe) {
        return doOnLifecycle(onSubscribe, Functions.emptyConsumer(),
                Functions.EMPTY_ACTION, Functions.EMPTY_ACTION,
                Functions.EMPTY_ACTION, Functions.EMPTY_ACTION);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable doOnTerminate(final Action onTerminate) {
        return doOnLifecycle(Functions.emptyConsumer(), Functions.emptyConsumer(),
                Functions.EMPTY_ACTION, onTerminate,
                Functions.EMPTY_ACTION, Functions.EMPTY_ACTION);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable doAfterTerminate(final Action onAfterTerminate) {
        return doOnLifecycle(
                Functions.emptyConsumer(),
                Functions.emptyConsumer(),
                Functions.EMPTY_ACTION,
                Functions.EMPTY_ACTION,
                onAfterTerminate,
                Functions.EMPTY_ACTION);
    }
    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable doFinally(Action onFinally) {
        ObjectHelper.requireNonNull(onFinally, "onFinally is null");
        return RxJavaPlugins.onAssembly(new CompletableDoFinally(this, onFinally));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable lift(final CompletableOperator onLift) {
        ObjectHelper.requireNonNull(onLift, "onLift is null");
        return RxJavaPlugins.onAssembly(new CompletableLift(this, onLift));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable mergeWith(CompletableSource other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return mergeArray(this, other);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Completable observeOn(final Scheduler scheduler) {
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new CompletableObserveOn(this, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable onErrorComplete() {
        return onErrorComplete(Functions.alwaysTrue());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable onErrorComplete(final Predicate<? super Throwable> predicate) {
        ObjectHelper.requireNonNull(predicate, "predicate is null");

        return RxJavaPlugins.onAssembly(new CompletableOnErrorComplete(this, predicate));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable onErrorResumeNext(final Function<? super Throwable, ? extends CompletableSource> errorMapper) {
        ObjectHelper.requireNonNull(errorMapper, "errorMapper is null");
        return RxJavaPlugins.onAssembly(new CompletableResumeNext(this, errorMapper));
    }

    
    @Experimental
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable onTerminateDetach() {
        return RxJavaPlugins.onAssembly(new CompletableDetach(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable repeat() {
        return fromPublisher(toFlowable().repeat());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable repeat(long times) {
        return fromPublisher(toFlowable().repeat(times));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable repeatUntil(BooleanSupplier stop) {
        return fromPublisher(toFlowable().repeatUntil(stop));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable repeatWhen(Function<? super Flowable<Object>, ? extends Publisher<?>> handler) {
        return fromPublisher(toFlowable().repeatWhen(handler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable retry() {
        return fromPublisher(toFlowable().retry());
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable retry(BiPredicate<? super Integer, ? super Throwable> predicate) {
        return fromPublisher(toFlowable().retry(predicate));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable retry(long times) {
        return fromPublisher(toFlowable().retry(times));
    }

    
    @Experimental
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable retry(long times, Predicate<? super Throwable> predicate) {
        return fromPublisher(toFlowable().retry(times, predicate));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable retry(Predicate<? super Throwable> predicate) {
        return fromPublisher(toFlowable().retry(predicate));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable retryWhen(Function<? super Flowable<Throwable>, ? extends Publisher<?>> handler) {
        return fromPublisher(toFlowable().retryWhen(handler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable startWith(CompletableSource other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return concatArray(other, this);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <T> Observable<T> startWith(Observable<T> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return other.concatWith(this.<T>toObservable());
    }
    
    @CheckReturnValue
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <T> Flowable<T> startWith(Publisher<T> other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return this.<T>toFlowable().startWith(other);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Completable hide() {
        return RxJavaPlugins.onAssembly(new CompletableHide(this));
    }

    
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Disposable subscribe() {
        EmptyCompletableObserver s = new EmptyCompletableObserver();
        subscribe(s);
        return s;
    }

    @SchedulerSupport(SchedulerSupport.NONE)
    @Override
    public final void subscribe(CompletableObserver s) {
        ObjectHelper.requireNonNull(s, "s is null");
        try {

            s = RxJavaPlugins.onSubscribe(this, s);

            subscribeActual(s);
        } catch (NullPointerException ex) {             throw ex;
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            RxJavaPlugins.onError(ex);
            throw toNpe(ex);
        }
    }

    
    protected abstract void subscribeActual(CompletableObserver s);

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <E extends CompletableObserver> E subscribeWith(E observer) {
        subscribe(observer);
        return observer;
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Disposable subscribe(final Action onComplete, final Consumer<? super Throwable> onError) {
        ObjectHelper.requireNonNull(onError, "onError is null");
        ObjectHelper.requireNonNull(onComplete, "onComplete is null");

        CallbackCompletableObserver s = new CallbackCompletableObserver(onError, onComplete);
        subscribe(s);
        return s;
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final Disposable subscribe(final Action onComplete) {
        ObjectHelper.requireNonNull(onComplete, "onComplete is null");

        CallbackCompletableObserver s = new CallbackCompletableObserver(onComplete);
        subscribe(s);
        return s;
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Completable subscribeOn(final Scheduler scheduler) {
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");

        return RxJavaPlugins.onAssembly(new CompletableSubscribeOn(this, scheduler));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Completable timeout(long timeout, TimeUnit unit) {
        return timeout0(timeout, unit, Schedulers.computation(), null);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.COMPUTATION)
    public final Completable timeout(long timeout, TimeUnit unit, CompletableSource other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return timeout0(timeout, unit, Schedulers.computation(), other);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Completable timeout(long timeout, TimeUnit unit, Scheduler scheduler) {
        return timeout0(timeout, unit, scheduler, null);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Completable timeout(long timeout, TimeUnit unit, Scheduler scheduler, CompletableSource other) {
        ObjectHelper.requireNonNull(other, "other is null");
        return timeout0(timeout, unit, scheduler, other);
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    private Completable timeout0(long timeout, TimeUnit unit, Scheduler scheduler, CompletableSource other) {
        ObjectHelper.requireNonNull(unit, "unit is null");
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new CompletableTimeout(this, timeout, unit, scheduler, other));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <U> U to(Function<? super Completable, U> converter) {
        try {
            return ObjectHelper.requireNonNull(converter, "converter is null").apply(this);
        } catch (Throwable ex) {
            Exceptions.throwIfFatal(ex);
            throw ExceptionHelper.wrapOrThrow(ex);
        }
    }

    
    @CheckReturnValue
    @SuppressWarnings("unchecked")
    @BackpressureSupport(BackpressureKind.FULL)
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <T> Flowable<T> toFlowable() {
        if (this instanceof FuseToFlowable) {
            return ((FuseToFlowable<T>)this).fuseToFlowable();
        }
        return RxJavaPlugins.onAssembly(new CompletableToFlowable<T>(this));
    }

    
    @CheckReturnValue
    @SuppressWarnings("unchecked")
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <T> Maybe<T> toMaybe() {
        if (this instanceof FuseToMaybe) {
            return ((FuseToMaybe<T>)this).fuseToMaybe();
        }
        return RxJavaPlugins.onAssembly(new MaybeFromCompletable<T>(this));
    }

    
    @CheckReturnValue
    @SuppressWarnings("unchecked")
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <T> Observable<T> toObservable() {
        if (this instanceof FuseToObservable) {
            return ((FuseToObservable<T>)this).fuseToObservable();
        }
        return RxJavaPlugins.onAssembly(new CompletableToObservable<T>(this));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <T> Single<T> toSingle(final Callable<? extends T> completionValueSupplier) {
        ObjectHelper.requireNonNull(completionValueSupplier, "completionValueSupplier is null");
        return RxJavaPlugins.onAssembly(new CompletableToSingle<T>(this, completionValueSupplier, null));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final <T> Single<T> toSingleDefault(final T completionValue) {
        ObjectHelper.requireNonNull(completionValue, "completionValue is null");
        return RxJavaPlugins.onAssembly(new CompletableToSingle<T>(this, null, completionValue));
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.CUSTOM)
    public final Completable unsubscribeOn(final Scheduler scheduler) {
        ObjectHelper.requireNonNull(scheduler, "scheduler is null");
        return RxJavaPlugins.onAssembly(new CompletableDisposeOn(this, scheduler));
    }
            
    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final TestObserver<Void> test() {
        TestObserver<Void> to = new TestObserver<Void>();
        subscribe(to);
        return to;
    }

    
    @CheckReturnValue
    @SchedulerSupport(SchedulerSupport.NONE)
    public final TestObserver<Void> test(boolean cancelled) {
        TestObserver<Void> to = new TestObserver<Void>();

        if (cancelled) {
            to.cancel();
        }
        subscribe(to);
        return to;
    }
}

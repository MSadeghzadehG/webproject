

package io.reactivex.plugins;

import static org.junit.Assert.*;

import java.io.*;
import java.lang.Thread.UncaughtExceptionHandler;
import java.lang.reflect.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.*;

import org.junit.*;
import org.reactivestreams.*;

import io.reactivex.*;
import io.reactivex.Observable;
import io.reactivex.Observer;
import io.reactivex.Scheduler.Worker;
import io.reactivex.disposables.*;
import io.reactivex.exceptions.*;
import io.reactivex.flowables.ConnectableFlowable;
import io.reactivex.functions.*;
import io.reactivex.internal.functions.Functions;
import io.reactivex.internal.operators.completable.CompletableError;
import io.reactivex.internal.operators.flowable.FlowableRange;
import io.reactivex.internal.operators.maybe.MaybeError;
import io.reactivex.internal.operators.observable.ObservableRange;
import io.reactivex.internal.operators.parallel.ParallelFromPublisher;
import io.reactivex.internal.operators.single.SingleJust;
import io.reactivex.internal.schedulers.ImmediateThinScheduler;
import io.reactivex.internal.subscriptions.ScalarSubscription;
import io.reactivex.observables.ConnectableObservable;
import io.reactivex.parallel.ParallelFlowable;
import io.reactivex.schedulers.Schedulers;

public class RxJavaPluginsTest {


    @Test
    public void constructorShouldBePrivate() {
        TestHelper.checkUtilityClass(RxJavaPlugins.class);
    }

    @Test
    @Ignore("Not part of 2.0")
    public void assemblyTrackingObservable() {
    }


    @Test
    @Ignore("Not part of 2.0")
    public void assemblyTrackingSingle() {
    }


    @Test
    @Ignore("Not part of 2.0")
    public void assemblyTrackingCompletable() {
    }

    @SuppressWarnings({ "rawtypes" })
    @Test
    public void lockdown() throws Exception {
        RxJavaPlugins.reset();
        RxJavaPlugins.lockdown();
        try {
            assertTrue(RxJavaPlugins.isLockdown());
            Consumer a1 = Functions.emptyConsumer();
            Callable f0 = new Callable() {
                @Override
                public Object call() {
                    return null;
                }
            };
            Function f1 = Functions.identity();
            BiFunction f2 = new BiFunction() {
                @Override
                public Object apply(Object t1, Object t2) {
                    return t2;
                }
            };

            BooleanSupplier bs = new BooleanSupplier() {
                @Override
                public boolean getAsBoolean() throws Exception {
                    return true;
                }
            };

            for (Method m : RxJavaPlugins.class.getMethods()) {
                if (m.getName().startsWith("set")) {

                    Method getter;

                    Class<?> paramType = m.getParameterTypes()[0];

                    if (paramType == Boolean.TYPE) {
                        getter = RxJavaPlugins.class.getMethod("is" + m.getName().substring(3));
                    } else {
                        getter = RxJavaPlugins.class.getMethod("get" + m.getName().substring(3));
                    }

                    Object before = getter.invoke(null);

                    try {
                        if (paramType.isAssignableFrom(Boolean.TYPE)) {
                            m.invoke(null, true);
                        } else
                        if (paramType.isAssignableFrom(Callable.class)) {
                            m.invoke(null, f0);
                        } else
                        if (paramType.isAssignableFrom(Function.class)) {
                            m.invoke(null, f1);
                        } else
                        if (paramType.isAssignableFrom(Consumer.class)) {
                            m.invoke(null, a1);
                        } else
                        if (paramType.isAssignableFrom(BooleanSupplier.class)) {
                            m.invoke(null, bs);
                        } else {
                            m.invoke(null, f2);
                        }
                        fail("Should have thrown InvocationTargetException(IllegalStateException)");
                    } catch (InvocationTargetException ex) {
                        if (ex.getCause() instanceof IllegalStateException) {
                            assertEquals("Plugins can't be changed anymore",ex.getCause().getMessage());
                        } else {
                            fail("Should have thrown InvocationTargetException(IllegalStateException)");
                        }
                    }

                    Object after = getter.invoke(null);

                    if (paramType.isPrimitive()) {
                        assertEquals(m.toString(), before, after);
                    } else {
                        assertSame(m.toString(), before, after);
                    }
                }
            }


        } finally {
            RxJavaPlugins.unlock();
            RxJavaPlugins.reset();
            assertFalse(RxJavaPlugins.isLockdown());
        }
    }

    Function<Scheduler, Scheduler> replaceWithImmediate = new Function<Scheduler, Scheduler>() {
        @Override
        public Scheduler apply(Scheduler t) {
            return ImmediateThinScheduler.INSTANCE;
        }
    };

    @Test
    public void overrideSingleScheduler() {
        try {
            RxJavaPlugins.setSingleSchedulerHandler(replaceWithImmediate);

            assertSame(ImmediateThinScheduler.INSTANCE, Schedulers.single());
        } finally {
            RxJavaPlugins.reset();
        }
                assertNotSame(ImmediateThinScheduler.INSTANCE, Schedulers.single());
    }

    @Test
    public void overrideComputationScheduler() {
        try {
            RxJavaPlugins.setComputationSchedulerHandler(replaceWithImmediate);

            assertSame(ImmediateThinScheduler.INSTANCE, Schedulers.computation());
        } finally {
            RxJavaPlugins.reset();
        }
                assertNotSame(ImmediateThinScheduler.INSTANCE, Schedulers.computation());
    }

    @Test
    public void overrideIoScheduler() {
        try {
            RxJavaPlugins.setIoSchedulerHandler(replaceWithImmediate);

            assertSame(ImmediateThinScheduler.INSTANCE, Schedulers.io());
        } finally {
            RxJavaPlugins.reset();
        }
                assertNotSame(ImmediateThinScheduler.INSTANCE, Schedulers.io());
    }

    @Test
    public void overrideNewThreadScheduler() {
        try {
            RxJavaPlugins.setNewThreadSchedulerHandler(replaceWithImmediate);

            assertSame(ImmediateThinScheduler.INSTANCE, Schedulers.newThread());
        } finally {
            RxJavaPlugins.reset();
        }
                assertNotSame(ImmediateThinScheduler.INSTANCE, Schedulers.newThread());
    }

    Function<Callable<Scheduler>, Scheduler> initReplaceWithImmediate = new Function<Callable<Scheduler>, Scheduler>() {
        @Override
        public Scheduler apply(Callable<Scheduler> t) {
            return ImmediateThinScheduler.INSTANCE;
        }
    };

    @Test
    public void overrideInitSingleScheduler() {
        final Scheduler s = Schedulers.single();         Callable<Scheduler> c = new Callable<Scheduler>() {
            @Override
            public Scheduler call() throws Exception {
                return s;
            }
        };
        try {
            RxJavaPlugins.setInitSingleSchedulerHandler(initReplaceWithImmediate);

            assertSame(ImmediateThinScheduler.INSTANCE, RxJavaPlugins.initSingleScheduler(c));
        } finally {
            RxJavaPlugins.reset();
        }
                assertSame(s, RxJavaPlugins.initSingleScheduler(c));
    }

    @Test
    public void overrideInitComputationScheduler() {
        final Scheduler s = Schedulers.computation();         Callable<Scheduler> c = new Callable<Scheduler>() {
            @Override
            public Scheduler call() throws Exception {
                return s;
            }
        };
        try {
            RxJavaPlugins.setInitComputationSchedulerHandler(initReplaceWithImmediate);

            assertSame(ImmediateThinScheduler.INSTANCE, RxJavaPlugins.initComputationScheduler(c));
        } finally {
            RxJavaPlugins.reset();
        }
                assertSame(s, RxJavaPlugins.initComputationScheduler(c));
    }

    @Test
    public void overrideInitIoScheduler() {
        final Scheduler s = Schedulers.io();         Callable<Scheduler> c = new Callable<Scheduler>() {
            @Override
            public Scheduler call() throws Exception {
                return s;
            }
        };
        try {
            RxJavaPlugins.setInitIoSchedulerHandler(initReplaceWithImmediate);

            assertSame(ImmediateThinScheduler.INSTANCE, RxJavaPlugins.initIoScheduler(c));
        } finally {
            RxJavaPlugins.reset();
        }
                assertSame(s, RxJavaPlugins.initIoScheduler(c));
    }

    @Test
    public void overrideInitNewThreadScheduler() {
        final Scheduler s = Schedulers.newThread();         Callable<Scheduler> c = new Callable<Scheduler>() {
            @Override
            public Scheduler call() throws Exception {
                return s;
            }
        };
        try {
            RxJavaPlugins.setInitNewThreadSchedulerHandler(initReplaceWithImmediate);

            assertSame(ImmediateThinScheduler.INSTANCE, RxJavaPlugins.initNewThreadScheduler(c));
        } finally {
            RxJavaPlugins.reset();
        }
                assertSame(s, RxJavaPlugins.initNewThreadScheduler(c));
    }

    Callable<Scheduler> nullResultCallable = new Callable<Scheduler>() {
        @Override
        public Scheduler call() throws Exception {
            return null;
        }
    };

    @Test
    public void overrideInitSingleSchedulerCrashes() {
                try {
            RxJavaPlugins.initSingleScheduler(null);
            fail("Should have thrown NullPointerException");
        } catch (NullPointerException npe) {
            assertEquals("Scheduler Callable can't be null", npe.getMessage());
        }

                try {
            RxJavaPlugins.initSingleScheduler(nullResultCallable);
            fail("Should have thrown NullPointerException");
        } catch (NullPointerException npe) {
            assertEquals("Scheduler Callable result can't be null", npe.getMessage());
        }
    }

    @Test
    public void overrideInitComputationSchedulerCrashes() {
                try {
            RxJavaPlugins.initComputationScheduler(null);
            fail("Should have thrown NullPointerException");
        } catch (NullPointerException npe) {
            assertEquals("Scheduler Callable can't be null", npe.getMessage());
        }

                try {
            RxJavaPlugins.initComputationScheduler(nullResultCallable);
            fail("Should have thrown NullPointerException");
        } catch (NullPointerException npe) {
            assertEquals("Scheduler Callable result can't be null", npe.getMessage());
        }
    }

    @Test
    public void overrideInitIoSchedulerCrashes() {
                try {
            RxJavaPlugins.initIoScheduler(null);
            fail("Should have thrown NullPointerException");
        } catch (NullPointerException npe) {
            assertEquals("Scheduler Callable can't be null", npe.getMessage());
        }

                try {
            RxJavaPlugins.initIoScheduler(nullResultCallable);
            fail("Should have thrown NullPointerException");
        } catch (NullPointerException npe) {
            assertEquals("Scheduler Callable result can't be null", npe.getMessage());
        }
    }

    @Test
    public void overrideInitNewThreadSchedulerCrashes() {
                try {
            RxJavaPlugins.initNewThreadScheduler(null);
            fail("Should have thrown NullPointerException");
        } catch (NullPointerException npe) {
                        assertEquals("Scheduler Callable can't be null", npe.getMessage());
        }

                try {
            RxJavaPlugins.initNewThreadScheduler(nullResultCallable);
            fail("Should have thrown NullPointerException");
        } catch (NullPointerException npe) {
            assertEquals("Scheduler Callable result can't be null", npe.getMessage());
        }
    }

    Callable<Scheduler> unsafeDefault = new Callable<Scheduler>() {
        @Override
        public Scheduler call() throws Exception {
            throw new AssertionError("Default Scheduler instance should not have been evaluated");
        }
    };

    @Test
    public void testDefaultSingleSchedulerIsInitializedLazily() {
                try {
            RxJavaPlugins.setInitSingleSchedulerHandler(initReplaceWithImmediate);
            RxJavaPlugins.initSingleScheduler(unsafeDefault);
        } finally {
            RxJavaPlugins.reset();
        }

                assertNotSame(ImmediateThinScheduler.INSTANCE, Schedulers.single());
    }

    @Test
    public void testDefaultIoSchedulerIsInitializedLazily() {
                try {
            RxJavaPlugins.setInitIoSchedulerHandler(initReplaceWithImmediate);
            RxJavaPlugins.initIoScheduler(unsafeDefault);
        } finally {
            RxJavaPlugins.reset();
        }

                assertNotSame(ImmediateThinScheduler.INSTANCE, Schedulers.io());
    }

    @Test
    public void testDefaultComputationSchedulerIsInitializedLazily() {
                try {
            RxJavaPlugins.setInitComputationSchedulerHandler(initReplaceWithImmediate);
            RxJavaPlugins.initComputationScheduler(unsafeDefault);
        } finally {
            RxJavaPlugins.reset();
        }

                assertNotSame(ImmediateThinScheduler.INSTANCE, Schedulers.computation());
    }

    @Test
    public void testDefaultNewThreadSchedulerIsInitializedLazily() {
                try {
            RxJavaPlugins.setInitNewThreadSchedulerHandler(initReplaceWithImmediate);
            RxJavaPlugins.initNewThreadScheduler(unsafeDefault);
        } finally {
            RxJavaPlugins.reset();
        }

                assertNotSame(ImmediateThinScheduler.INSTANCE, Schedulers.newThread());
    }

    @SuppressWarnings("rawtypes")
    @Test
    public void observableCreate() {
        try {
            RxJavaPlugins.setOnObservableAssembly(new Function<Observable, Observable>() {
                @Override
                public Observable apply(Observable t) {
                    return new ObservableRange(1, 2);
                }
            });

            Observable.range(10, 3)
            .test()
            .assertValues(1, 2)
            .assertNoErrors()
            .assertComplete();
        } finally {
            RxJavaPlugins.reset();
        }
                Observable.range(10, 3)
        .test()
        .assertValues(10, 11, 12)
        .assertNoErrors()
        .assertComplete();
    }

    @SuppressWarnings("rawtypes")
    @Test
    public void flowableCreate() {
        try {
            RxJavaPlugins.setOnFlowableAssembly(new Function<Flowable, Flowable>() {
                @Override
                public Flowable apply(Flowable t) {
                    return new FlowableRange(1, 2);
                }
            });

            Flowable.range(10, 3)
            .test()
            .assertValues(1, 2)
            .assertNoErrors()
            .assertComplete();
        } finally {
            RxJavaPlugins.reset();
        }
                Flowable.range(10, 3)
        .test()
        .assertValues(10, 11, 12)
        .assertNoErrors()
        .assertComplete();
    }

    @SuppressWarnings("rawtypes")
    @Test
    public void observableStart() {
        try {
            RxJavaPlugins.setOnObservableSubscribe(new BiFunction<Observable, Observer, Observer>() {
                @Override
                public Observer apply(Observable o, final Observer t) {
                    return new Observer() {

                        @Override
                        public void onSubscribe(Disposable d) {
                            t.onSubscribe(d);
                        }

                        @SuppressWarnings("unchecked")
                        @Override
                        public void onNext(Object value) {
                            t.onNext((Integer)value - 9);
                        }

                        @Override
                        public void onError(Throwable e) {
                            t.onError(e);
                        }

                        @Override
                        public void onComplete() {
                            t.onComplete();
                        }

                    };
                }
            });

            Observable.range(10, 3)
            .test()
            .assertValues(1, 2, 3)
            .assertNoErrors()
            .assertComplete();
        } finally {
            RxJavaPlugins.reset();
        }
                Observable.range(10, 3)
        .test()
        .assertValues(10, 11, 12)
        .assertNoErrors()
        .assertComplete();
    }

    @SuppressWarnings("rawtypes")
    @Test
    public void flowableStart() {
        try {
            RxJavaPlugins.setOnFlowableSubscribe(new BiFunction<Flowable, Subscriber, Subscriber>() {
                @Override
                public Subscriber apply(Flowable o, final Subscriber t) {
                    return new Subscriber() {

                        @Override
                        public void onSubscribe(Subscription d) {
                            t.onSubscribe(d);
                        }

                        @SuppressWarnings("unchecked")
                        @Override
                        public void onNext(Object value) {
                            t.onNext((Integer)value - 9);
                        }

                        @Override
                        public void onError(Throwable e) {
                            t.onError(e);
                        }

                        @Override
                        public void onComplete() {
                            t.onComplete();
                        }

                    };
                }
            });

            Flowable.range(10, 3)
            .test()
            .assertValues(1, 2, 3)
            .assertNoErrors()
            .assertComplete();
        } finally {
            RxJavaPlugins.reset();
        }
                Flowable.range(10, 3)
        .test()
        .assertValues(10, 11, 12)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    @Ignore("Different architecture, no longer supported")
    public void observableReturn() {
    }

    @SuppressWarnings("rawtypes")
    @Test
    public void singleCreate() {
        try {
            RxJavaPlugins.setOnSingleAssembly(new Function<Single, Single>() {
                @Override
                public Single apply(Single t) {
                    return new SingleJust<Integer>(10);
                }
            });

            Single.just(1)
            .test()
            .assertValue(10)
            .assertNoErrors()
            .assertComplete();
        } finally {
            RxJavaPlugins.reset();
        }
                Single.just(1)
        .test()
        .assertValue(1)
        .assertNoErrors()
        .assertComplete();
    }

    @SuppressWarnings("rawtypes")
    @Test
    public void singleStart() {
        try {
            RxJavaPlugins.setOnSingleSubscribe(new BiFunction<Single, SingleObserver, SingleObserver>() {
                @Override
                public SingleObserver apply(Single o, final SingleObserver t) {
                    return new SingleObserver<Object>() {

                        @Override
                        public void onSubscribe(Disposable d) {
                            t.onSubscribe(d);
                        }

                        @SuppressWarnings("unchecked")
                        @Override
                        public void onSuccess(Object value) {
                            t.onSuccess(10);
                        }

                        @Override
                        public void onError(Throwable e) {
                            t.onError(e);
                        }

                    };
                }
            });

            Single.just(1)
            .test()
            .assertValue(10)
            .assertNoErrors()
            .assertComplete();
        } finally {
            RxJavaPlugins.reset();
        }
                Single.just(1)
        .test()
        .assertValue(1)
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    @Ignore("Different architecture, no longer supported")
    public void singleReturn() {
    }

    @Test
    public void completableCreate() {
        try {
            RxJavaPlugins.setOnCompletableAssembly(new Function<Completable, Completable>() {
                @Override
                public Completable apply(Completable t) {
                    return new CompletableError(new TestException());
                }
            });

            Completable.complete()
            .test()
            .assertNoValues()
            .assertNotComplete()
            .assertError(TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
                Completable.complete()
        .test()
        .assertNoValues()
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void completableStart() {
        try {
            RxJavaPlugins.setOnCompletableSubscribe(new BiFunction<Completable, CompletableObserver, CompletableObserver>() {
                @Override
                public CompletableObserver apply(Completable o, final CompletableObserver t) {
                    return new CompletableObserver() {
                        @Override
                        public void onSubscribe(Disposable d) {
                            t.onSubscribe(d);
                        }

                        @Override
                        public void onError(Throwable e) {
                            t.onError(e);
                        }

                        @Override
                        public void onComplete() {
                            t.onError(new TestException());
                        }
                    };
                }
            });

            Completable.complete()
            .test()
            .assertNoValues()
            .assertNotComplete()
            .assertError(TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
        
        Completable.complete()
        .test()
        .assertNoValues()
        .assertNoErrors()
        .assertComplete();
    }

    void onSchedule(Worker w) throws InterruptedException {
        try {
            try {
                final AtomicInteger value = new AtomicInteger();
                final CountDownLatch cdl = new CountDownLatch(1);

                RxJavaPlugins.setScheduleHandler(new Function<Runnable, Runnable>() {
                    @Override
                    public Runnable apply(Runnable t) {
                        return new Runnable() {
                            @Override
                            public void run() {
                                value.set(10);
                                cdl.countDown();
                            }
                        };
                    }
                });

                w.schedule(new Runnable() {
                    @Override
                    public void run() {
                        value.set(1);
                        cdl.countDown();
                    }
                });

                cdl.await();

                assertEquals(10, value.get());

            } finally {

                RxJavaPlugins.reset();
            }

                        final AtomicInteger value = new AtomicInteger();
            final CountDownLatch cdl = new CountDownLatch(1);

            w.schedule(new Runnable() {
                @Override
                public void run() {
                    value.set(1);
                    cdl.countDown();
                }
            });

            cdl.await();

            assertEquals(1, value.get());
        } finally {
            w.dispose();
        }
    }

    @Test
    public void onScheduleComputation() throws InterruptedException {
        onSchedule(Schedulers.computation().createWorker());
    }

    @Test
    public void onScheduleIO() throws InterruptedException {
        onSchedule(Schedulers.io().createWorker());
    }

    @Test
    public void onScheduleNewThread() throws InterruptedException {
        onSchedule(Schedulers.newThread().createWorker());
    }

    @Test
    public void onError() {
        try {
            final List<Throwable> list = new ArrayList<Throwable>();

            RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
                @Override
                public void accept(Throwable t) {
                    list.add(t);
                }
            });

            RxJavaPlugins.onError(new TestException("Forced failure"));

            assertEquals(1, list.size());
            assertUndeliverableTestException(list, 0, "Forced failure");
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @Test
    @Ignore("No (need for) clear() method in 2.0")
    public void clear() throws Exception {
    }

    @Test
    public void onErrorNoHandler() {
        try {
            final List<Throwable> list = new ArrayList<Throwable>();

            RxJavaPlugins.setErrorHandler(null);

            Thread.currentThread().setUncaughtExceptionHandler(new UncaughtExceptionHandler() {

                @Override
                public void uncaughtException(Thread t, Throwable e) {
                    list.add(e);

                }
            });

            RxJavaPlugins.onError(new TestException("Forced failure"));

            Thread.currentThread().setUncaughtExceptionHandler(null);

                        RxJavaPlugins.onError(new TestException("Forced failure 3"));

            assertEquals(1, list.size());
            assertUndeliverableTestException(list, 0, "Forced failure");
        } finally {
            RxJavaPlugins.reset();
            Thread.currentThread().setUncaughtExceptionHandler(null);
        }
    }

    @Test
    public void onErrorCrashes() {
        try {
            final List<Throwable> list = new ArrayList<Throwable>();

            RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
                @Override
                public void accept(Throwable t) {
                    throw new TestException("Forced failure 2");
                }
            });

            Thread.currentThread().setUncaughtExceptionHandler(new UncaughtExceptionHandler() {

                @Override
                public void uncaughtException(Thread t, Throwable e) {
                    list.add(e);

                }
            });

            RxJavaPlugins.onError(new TestException("Forced failure"));

            assertEquals(2, list.size());
            assertTestException(list, 0, "Forced failure 2");
            assertUndeliverableTestException(list, 1, "Forced failure");

            Thread.currentThread().setUncaughtExceptionHandler(null);

        } finally {
            RxJavaPlugins.reset();
            Thread.currentThread().setUncaughtExceptionHandler(null);
        }
    }

    @Test
    public void onErrorWithNull() {
        try {
            final List<Throwable> list = new ArrayList<Throwable>();

            RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
                @Override
                public void accept(Throwable t) {
                    throw new TestException("Forced failure 2");
                }
            });

            Thread.currentThread().setUncaughtExceptionHandler(new UncaughtExceptionHandler() {

                @Override
                public void uncaughtException(Thread t, Throwable e) {
                    list.add(e);

                }
            });

            RxJavaPlugins.onError(null);

            assertEquals(2, list.size());
            assertTestException(list, 0, "Forced failure 2");
            assertNPE(list, 1);

            RxJavaPlugins.reset();

            RxJavaPlugins.onError(null);

            assertNPE(list, 2);

        } finally {
            RxJavaPlugins.reset();

            Thread.currentThread().setUncaughtExceptionHandler(null);
        }
    }

    
    @Test
    @SuppressWarnings("rawtypes")
    public void onErrorWithSuper() throws Exception {
        try {
            Consumer<? super Throwable> errorHandler = new Consumer<Throwable>() {
                @Override
                public void accept(Throwable t) {
                    throw new TestException("Forced failure 2");
                }
            };
            RxJavaPlugins.setErrorHandler(errorHandler);

            Consumer<? super Throwable> errorHandler1 = RxJavaPlugins.getErrorHandler();
            assertSame(errorHandler, errorHandler1);

            Function<? super Scheduler, ? extends Scheduler> scheduler2scheduler = new Function<Scheduler, Scheduler>() {
                @Override
                public Scheduler apply(Scheduler scheduler) throws Exception {
                    return scheduler;
                }
            };
            Function<? super Callable<Scheduler>, ? extends Scheduler> callable2scheduler = new Function<Callable<Scheduler>, Scheduler>() {
                @Override
                public Scheduler apply(Callable<Scheduler> schedulerCallable) throws Exception {
                    return schedulerCallable.call();
                }
            };
            Function<? super ConnectableFlowable, ? extends ConnectableFlowable> connectableFlowable2ConnectableFlowable = new Function<ConnectableFlowable, ConnectableFlowable>() {
                @Override
                public ConnectableFlowable apply(ConnectableFlowable connectableFlowable) throws Exception {
                    return connectableFlowable;
                }
            };
            Function<? super ConnectableObservable, ? extends ConnectableObservable> connectableObservable2ConnectableObservable = new Function<ConnectableObservable, ConnectableObservable>() {
                @Override
                public ConnectableObservable apply(ConnectableObservable connectableObservable) throws Exception {
                    return connectableObservable;
                }
            };
            Function<? super Flowable, ? extends Flowable> flowable2Flowable = new Function<Flowable, Flowable>() {
                @Override
                public Flowable apply(Flowable flowable) throws Exception {
                    return flowable;
                }
            };
            BiFunction<? super Flowable, ? super Subscriber, ? extends Subscriber> flowable2subscriber = new BiFunction<Flowable, Subscriber, Subscriber>() {
                @Override
                public Subscriber apply(Flowable flowable, Subscriber subscriber) throws Exception {
                    return subscriber;
                }
            };
            Function<Maybe, Maybe> maybe2maybe = new Function<Maybe, Maybe>() {
                @Override
                public Maybe apply(Maybe maybe) throws Exception {
                    return maybe;
                }
            };
            BiFunction<Maybe, MaybeObserver, MaybeObserver> maybe2observer = new BiFunction<Maybe, MaybeObserver, MaybeObserver>() {
                @Override
                public MaybeObserver apply(Maybe maybe, MaybeObserver maybeObserver) throws Exception {
                    return maybeObserver;
                }
            };
            Function<Observable, Observable> observable2observable = new Function<Observable, Observable>() {
                @Override
                public Observable apply(Observable observable) throws Exception {
                    return observable;
                }
            };
            BiFunction<? super Observable, ? super Observer, ? extends Observer> observable2observer = new BiFunction<Observable, Observer, Observer>() {
                @Override
                public Observer apply(Observable observable, Observer observer) throws Exception {
                    return observer;
                }
            };
            Function<? super ParallelFlowable, ? extends ParallelFlowable> parallelFlowable2parallelFlowable = new Function<ParallelFlowable, ParallelFlowable>() {
                @Override
                public ParallelFlowable apply(ParallelFlowable parallelFlowable) throws Exception {
                    return parallelFlowable;
                }
            };
            Function<Single, Single> single2single = new Function<Single, Single>() {
                @Override
                public Single apply(Single single) throws Exception {
                    return single;
                }
            };
            BiFunction<? super Single, ? super SingleObserver, ? extends SingleObserver> single2observer = new BiFunction<Single, SingleObserver, SingleObserver>() {
                @Override
                public SingleObserver apply(Single single, SingleObserver singleObserver) throws Exception {
                    return singleObserver;
                }
            };
            Function<? super Runnable, ? extends Runnable> runnable2runnable = new Function<Runnable, Runnable>() {
                @Override
                public Runnable apply(Runnable runnable) throws Exception {
                    return runnable;
                }
            };
            BiFunction<? super Completable, ? super CompletableObserver, ? extends CompletableObserver> completableObserver2completableObserver = new BiFunction<Completable, CompletableObserver, CompletableObserver>() {
                @Override
                public CompletableObserver apply(Completable completable, CompletableObserver completableObserver) throws Exception {
                    return completableObserver;
                }
            };
            Function<? super Completable, ? extends Completable> completable2completable = new Function<Completable, Completable>() {
                @Override
                public Completable apply(Completable completable) throws Exception {
                    return completable;
                }
            };


            RxJavaPlugins.setInitComputationSchedulerHandler(callable2scheduler);
            RxJavaPlugins.setComputationSchedulerHandler(scheduler2scheduler);
            RxJavaPlugins.setIoSchedulerHandler(scheduler2scheduler);
            RxJavaPlugins.setNewThreadSchedulerHandler(scheduler2scheduler);
            RxJavaPlugins.setOnConnectableFlowableAssembly(connectableFlowable2ConnectableFlowable);
            RxJavaPlugins.setOnConnectableObservableAssembly(connectableObservable2ConnectableObservable);
            RxJavaPlugins.setOnFlowableAssembly(flowable2Flowable);
            RxJavaPlugins.setOnFlowableSubscribe(flowable2subscriber);
            RxJavaPlugins.setOnMaybeAssembly(maybe2maybe);
            RxJavaPlugins.setOnMaybeSubscribe(maybe2observer);
            RxJavaPlugins.setOnObservableAssembly(observable2observable);
            RxJavaPlugins.setOnObservableSubscribe(observable2observer);
            RxJavaPlugins.setOnParallelAssembly(parallelFlowable2parallelFlowable);
            RxJavaPlugins.setOnSingleAssembly(single2single);
            RxJavaPlugins.setOnSingleSubscribe(single2observer);
            RxJavaPlugins.setScheduleHandler(runnable2runnable);
            RxJavaPlugins.setSingleSchedulerHandler(scheduler2scheduler);
            RxJavaPlugins.setOnCompletableSubscribe(completableObserver2completableObserver);
            RxJavaPlugins.setOnCompletableAssembly(completable2completable);
            RxJavaPlugins.setInitSingleSchedulerHandler(callable2scheduler);
            RxJavaPlugins.setInitNewThreadSchedulerHandler(callable2scheduler);
            RxJavaPlugins.setInitIoSchedulerHandler(callable2scheduler);
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @SuppressWarnings({"rawtypes", "unchecked" })
    @Test
    public void clearIsPassthrough() {
        try {
            RxJavaPlugins.reset();

            assertNull(RxJavaPlugins.onAssembly((Observable)null));

            assertNull(RxJavaPlugins.onAssembly((ConnectableObservable)null));

            assertNull(RxJavaPlugins.onAssembly((Flowable)null));

            assertNull(RxJavaPlugins.onAssembly((ConnectableFlowable)null));

            Observable oos = new Observable() {
                @Override
                public void subscribeActual(Observer t) {

                }
            };

            Flowable fos = new Flowable() {
                @Override
                public void subscribeActual(Subscriber t) {

                }
            };

            assertSame(oos, RxJavaPlugins.onAssembly(oos));

            assertSame(fos, RxJavaPlugins.onAssembly(fos));

            assertNull(RxJavaPlugins.onAssembly((Single)null));

            Single sos = new Single() {
                @Override
                public void subscribeActual(SingleObserver t) {

                }
            };

            assertSame(sos, RxJavaPlugins.onAssembly(sos));

            assertNull(RxJavaPlugins.onAssembly((Completable)null));

            Completable cos = new Completable() {
                @Override
                public void subscribeActual(CompletableObserver t) {

                }
            };

            assertSame(cos, RxJavaPlugins.onAssembly(cos));

            assertNull(RxJavaPlugins.onAssembly((Maybe)null));

            Maybe myb = new Maybe() {
                @Override
                public void subscribeActual(MaybeObserver t) {

                }
            };

            assertSame(myb, RxJavaPlugins.onAssembly(myb));


            Runnable action = Functions.EMPTY_RUNNABLE;
            assertSame(action, RxJavaPlugins.onSchedule(action));

            class AllSubscriber implements Subscriber, Observer, SingleObserver, CompletableObserver, MaybeObserver {

                @Override
                public void onSuccess(Object value) {

                }

                @Override
                public void onSubscribe(Disposable d) {

                }

                @Override
                public void onSubscribe(Subscription s) {

                }

                @Override
                public void onNext(Object t) {

                }

                @Override
                public void onError(Throwable t) {

                }

                @Override
                public void onComplete() {

                }

            }

            AllSubscriber all = new AllSubscriber();

            assertNull(RxJavaPlugins.onSubscribe(Observable.never(), null));

            assertSame(all, RxJavaPlugins.onSubscribe(Observable.never(), all));

            assertNull(RxJavaPlugins.onSubscribe(Flowable.never(), null));

            assertSame(all, RxJavaPlugins.onSubscribe(Flowable.never(), all));

            assertNull(RxJavaPlugins.onSubscribe(Single.just(1), null));

            assertSame(all, RxJavaPlugins.onSubscribe(Single.just(1), all));

            assertNull(RxJavaPlugins.onSubscribe(Completable.never(), null));

            assertSame(all, RxJavaPlugins.onSubscribe(Completable.never(), all));

            assertNull(RxJavaPlugins.onSubscribe(Maybe.never(), null));

            assertSame(all, RxJavaPlugins.onSubscribe(Maybe.never(), all));

            
            final Scheduler s = ImmediateThinScheduler.INSTANCE;
            Callable<Scheduler> c = new Callable<Scheduler>() {
                @Override
                public Scheduler call() throws Exception {
                    return s;
                }
            };
            assertSame(s, RxJavaPlugins.onComputationScheduler(s));

            assertSame(s, RxJavaPlugins.onIoScheduler(s));

            assertSame(s, RxJavaPlugins.onNewThreadScheduler(s));

            assertSame(s, RxJavaPlugins.onSingleScheduler(s));

            assertSame(s, RxJavaPlugins.initComputationScheduler(c));

            assertSame(s, RxJavaPlugins.initIoScheduler(c));

            assertSame(s, RxJavaPlugins.initNewThreadScheduler(c));

            assertSame(s, RxJavaPlugins.initSingleScheduler(c));

        } finally {
            RxJavaPlugins.reset();
        }
    }

    static void assertTestException(List<Throwable> list, int index, String message) {
        assertTrue(list.get(index).toString(), list.get(index) instanceof TestException);
        assertEquals(message, list.get(index).getMessage());
    }

    static void assertUndeliverableTestException(List<Throwable> list, int index, String message) {
        assertTrue(list.get(index).toString(), list.get(index).getCause() instanceof TestException);
        assertEquals(message, list.get(index).getCause().getMessage());
    }

    static void assertNPE(List<Throwable> list, int index) {
        assertTrue(list.get(index).toString(), list.get(index) instanceof NullPointerException);
    }

    @Test
    @Ignore("Not present in 2.0")
    public void onXError() {
    }

    @Test
    @Ignore("Not present in 2.0")
    public void onPluginsXError() {
    }

    @Test
    @Ignore("Not present in 2.0")
    public void onXLift() {
    }

    @Test
    @Ignore("Not present in 2.0")
    public void onPluginsXLift() {
    }

    @SuppressWarnings("rawtypes")
    @Test
    public void overrideConnectableObservable() {
        try {
            RxJavaPlugins.setOnConnectableObservableAssembly(new Function<ConnectableObservable, ConnectableObservable>() {
                @Override
                public ConnectableObservable apply(ConnectableObservable co) throws Exception {
                    return new ConnectableObservable() {

                        @Override
                        public void connect(Consumer connection) {

                        }

                        @SuppressWarnings("unchecked")
                        @Override
                        protected void subscribeActual(Observer observer) {
                            observer.onSubscribe(Disposables.empty());
                            observer.onNext(10);
                            observer.onComplete();
                        }
                    };
                }
            });

            Observable
            .just(1)
            .publish()
            .autoConnect()
            .test()
            .assertResult(10);

        } finally {
            RxJavaPlugins.reset();
        }

        Observable
        .just(1)
        .publish()
        .autoConnect()
        .test()
        .assertResult(1);
    }

    @SuppressWarnings("rawtypes")
    @Test
    public void overrideConnectableFlowable() {
        try {
            RxJavaPlugins.setOnConnectableFlowableAssembly(new Function<ConnectableFlowable, ConnectableFlowable>() {
                @Override
                public ConnectableFlowable apply(ConnectableFlowable co) throws Exception {
                    return new ConnectableFlowable() {

                        @Override
                        public void connect(Consumer connection) {

                        }

                        @SuppressWarnings("unchecked")
                        @Override
                        protected void subscribeActual(Subscriber subscriber) {
                            subscriber.onSubscribe(new ScalarSubscription(subscriber, 10));
                        }
                    };
                }
            });

            Flowable
            .just(1)
            .publish()
            .autoConnect()
            .test()
            .assertResult(10);

        } finally {
            RxJavaPlugins.reset();
        }

        Flowable
        .just(1)
        .publish()
        .autoConnect()
        .test()
        .assertResult(1);
    }

    @SuppressWarnings("rawtypes")
    @Test
    public void assemblyHookCrashes() {
        try {
            RxJavaPlugins.setOnFlowableAssembly(new Function<Flowable, Flowable>() {
                @Override
                public Flowable apply(Flowable f) throws Exception {
                    throw new IllegalArgumentException();
                }
            });

            try {
                Flowable.empty();
                fail("Should have thrown!");
            } catch (IllegalArgumentException ex) {
                            }

            RxJavaPlugins.setOnFlowableAssembly(new Function<Flowable, Flowable>() {
                @Override
                public Flowable apply(Flowable f) throws Exception {
                    throw new InternalError();
                }
            });

            try {
                Flowable.empty();
                fail("Should have thrown!");
            } catch (InternalError ex) {
                            }

            RxJavaPlugins.setOnFlowableAssembly(new Function<Flowable, Flowable>() {
                @Override
                public Flowable apply(Flowable f) throws Exception {
                    throw new IOException();
                }
            });

            try {
                Flowable.empty();
                fail("Should have thrown!");
            } catch (RuntimeException ex) {
                if (!(ex.getCause() instanceof IOException)) {
                    fail(ex.getCause().toString() + ": Should have thrown RuntimeException(IOException)");
                }
            }
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @SuppressWarnings("rawtypes")
    @Test
    public void subscribeHookCrashes() {
        try {
            RxJavaPlugins.setOnFlowableSubscribe(new BiFunction<Flowable, Subscriber, Subscriber>() {
                @Override
                public Subscriber apply(Flowable f, Subscriber s) throws Exception {
                    throw new IllegalArgumentException();
                }
            });

            try {
                Flowable.empty().test();
                fail("Should have thrown!");
            } catch (NullPointerException ex) {
                if (!(ex.getCause() instanceof IllegalArgumentException)) {
                    fail(ex.getCause().toString() + ": Should have thrown NullPointerException(IllegalArgumentException)");
                }
            }

            RxJavaPlugins.setOnFlowableSubscribe(new BiFunction<Flowable, Subscriber, Subscriber>() {
                @Override
                public Subscriber apply(Flowable f, Subscriber s) throws Exception {
                    throw new InternalError();
                }
            });

            try {
                Flowable.empty().test();
                fail("Should have thrown!");
            } catch (InternalError ex) {
                            }

            RxJavaPlugins.setOnFlowableSubscribe(new BiFunction<Flowable, Subscriber, Subscriber>() {
                @Override
                public Subscriber apply(Flowable f, Subscriber s) throws Exception {
                    throw new IOException();
                }
            });

            try {
                Flowable.empty().test();
                fail("Should have thrown!");
            } catch (NullPointerException ex) {
                if (!(ex.getCause() instanceof RuntimeException)) {
                    fail(ex.getCause().toString() + ": Should have thrown NullPointerException(RuntimeException(IOException))");
                }
                if (!(ex.getCause().getCause() instanceof IOException)) {
                    fail(ex.getCause().toString() + ": Should have thrown NullPointerException(RuntimeException(IOException))");
                }
            }
        } finally {
            RxJavaPlugins.reset();
        }
    }


    @SuppressWarnings("rawtypes")
    @Test
    public void maybeCreate() {
        try {
            RxJavaPlugins.setOnMaybeAssembly(new Function<Maybe, Maybe>() {
                @Override
                public Maybe apply(Maybe t) {
                    return new MaybeError(new TestException());
                }
            });

            Maybe.empty()
            .test()
            .assertNoValues()
            .assertNotComplete()
            .assertError(TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
                Maybe.empty()
        .test()
        .assertNoValues()
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    @SuppressWarnings("rawtypes")
    public void maybeStart() {
        try {
            RxJavaPlugins.setOnMaybeSubscribe(new BiFunction<Maybe, MaybeObserver, MaybeObserver>() {
                @Override
                public MaybeObserver apply(Maybe o, final MaybeObserver t) {
                    return new MaybeObserver() {
                        @Override
                        public void onSubscribe(Disposable d) {
                            t.onSubscribe(d);
                        }

                        @SuppressWarnings("unchecked")
                        @Override
                        public void onSuccess(Object value) {
                            t.onSuccess(value);
                        }

                        @Override
                        public void onError(Throwable e) {
                            t.onError(e);
                        }

                        @Override
                        public void onComplete() {
                            t.onError(new TestException());
                        }
                    };
                }
            });

            Maybe.empty()
            .test()
            .assertNoValues()
            .assertNotComplete()
            .assertError(TestException.class);
        } finally {
            RxJavaPlugins.reset();
        }
        
        Maybe.empty()
        .test()
        .assertNoValues()
        .assertNoErrors()
        .assertComplete();
    }

    @Test
    public void onErrorNull() {
        try {
            final AtomicReference<Throwable> t = new AtomicReference<Throwable>();

            RxJavaPlugins.setErrorHandler(new Consumer<Throwable>() {
                @Override
                public void accept(final Throwable throwable) throws Exception {
                    t.set(throwable);
                }
            });

            RxJavaPlugins.onError(null);

            final Throwable throwable = t.get();
            assertEquals("onError called with null. Null values are generally not allowed in 2.x operators and sources.", throwable.getMessage());
            assertTrue(throwable instanceof NullPointerException);
        } finally {
            RxJavaPlugins.reset();
        }
    }

    private static void verifyThread(Scheduler scheduler, String expectedThreadName)
            throws AssertionError {
        assertNotNull(scheduler);
        Worker w = scheduler.createWorker();
        try {
            final AtomicReference<Thread> value = new AtomicReference<Thread>();
            final CountDownLatch cdl = new CountDownLatch(1);

            w.schedule(new Runnable() {
                @Override
                public void run() {
                    value.set(Thread.currentThread());
                    cdl.countDown();
                }
            });

            cdl.await();

            Thread t = value.get();
            assertNotNull(t);
            assertTrue(expectedThreadName.equals(t.getName()));
        } catch (Exception e) {
            fail();
        } finally {
            w.dispose();
        }
    }

    @Test
    public void createComputationScheduler() {
        final String name = "ComputationSchedulerTest";
        ThreadFactory factory = new ThreadFactory() {
            @Override
            public Thread newThread(Runnable r) {
                return new Thread(r, name);
            }
        };

        final Scheduler customScheduler = RxJavaPlugins.createComputationScheduler(factory);
        RxJavaPlugins.setComputationSchedulerHandler(new Function<Scheduler, Scheduler>() {
            @Override
            public Scheduler apply(Scheduler scheduler) throws Exception {
                return customScheduler;
            }
        });

        try {
            verifyThread(Schedulers.computation(), name);
        } finally {
            customScheduler.shutdown();
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void createIoScheduler() {
        final String name = "IoSchedulerTest";
        ThreadFactory factory = new ThreadFactory() {
            @Override
            public Thread newThread(Runnable r) {
                return new Thread(r, name);
            }
        };

        final Scheduler customScheduler = RxJavaPlugins.createIoScheduler(factory);
        RxJavaPlugins.setIoSchedulerHandler(new Function<Scheduler, Scheduler>() {
            @Override
            public Scheduler apply(Scheduler scheduler) throws Exception {
                return customScheduler;
            }
        });

        try {
            verifyThread(Schedulers.io(), name);
        } finally {
            customScheduler.shutdown();
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void createNewThreadScheduler() {
        final String name = "NewThreadSchedulerTest";
        ThreadFactory factory = new ThreadFactory() {
            @Override
            public Thread newThread(Runnable r) {
                return new Thread(r, name);
            }
        };

        final Scheduler customScheduler = RxJavaPlugins.createNewThreadScheduler(factory);
        RxJavaPlugins.setNewThreadSchedulerHandler(new Function<Scheduler, Scheduler>() {
            @Override
            public Scheduler apply(Scheduler scheduler) throws Exception {
                return customScheduler;
            }
        });

        try {
            verifyThread(Schedulers.newThread(), name);
        } finally {
            customScheduler.shutdown();
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void createSingleScheduler() {
        final String name = "SingleSchedulerTest";
        ThreadFactory factory = new ThreadFactory() {
            @Override
            public Thread newThread(Runnable r) {
                return new Thread(r, name);
            }
        };

        final Scheduler customScheduler = RxJavaPlugins.createSingleScheduler(factory);

        RxJavaPlugins.setSingleSchedulerHandler(new Function<Scheduler, Scheduler>() {
            @Override
            public Scheduler apply(Scheduler scheduler) throws Exception {
                return customScheduler;
            }
        });

        try {
            verifyThread(Schedulers.single(), name);
        } finally {
            customScheduler.shutdown();
            RxJavaPlugins.reset();
        }
    }

    @Test
    public void onBeforeBlocking() {
        try {
            RxJavaPlugins.setOnBeforeBlocking(new BooleanSupplier() {
                @Override
                public boolean getAsBoolean() throws Exception {
                    throw new IllegalArgumentException();
                }
            });

            try {
                RxJavaPlugins.onBeforeBlocking();
                fail("Should have thrown");
            } catch (IllegalArgumentException ex) {
                            }
        } finally {
            RxJavaPlugins.reset();
        }
    }

    @SuppressWarnings("rawtypes")
    @Test
    public void onParallelAssembly() {
        try {
            RxJavaPlugins.setOnParallelAssembly(new Function<ParallelFlowable, ParallelFlowable>() {
                @Override
                public ParallelFlowable apply(ParallelFlowable pf) throws Exception {
                    return new ParallelFromPublisher<Integer>(Flowable.just(2), 2, 2);
                }
            });

            Flowable.just(1)
            .parallel()
            .sequential()
            .test()
            .assertResult(2);
        } finally {
            RxJavaPlugins.reset();
        }

        Flowable.just(1)
        .parallel()
        .sequential()
        .test()
        .assertResult(1);
    }

    @Test
    public void isBug() {
        assertFalse(RxJavaPlugins.isBug(new RuntimeException()));
        assertFalse(RxJavaPlugins.isBug(new IOException()));
        assertFalse(RxJavaPlugins.isBug(new InterruptedException()));
        assertFalse(RxJavaPlugins.isBug(new InterruptedIOException()));

        assertTrue(RxJavaPlugins.isBug(new NullPointerException()));
        assertTrue(RxJavaPlugins.isBug(new IllegalArgumentException()));
        assertTrue(RxJavaPlugins.isBug(new IllegalStateException()));
        assertTrue(RxJavaPlugins.isBug(new MissingBackpressureException()));
        assertTrue(RxJavaPlugins.isBug(new ProtocolViolationException("")));
        assertTrue(RxJavaPlugins.isBug(new UndeliverableException(new TestException())));
        assertTrue(RxJavaPlugins.isBug(new CompositeException(new TestException())));
        assertTrue(RxJavaPlugins.isBug(new OnErrorNotImplementedException(new TestException())));
    }
}

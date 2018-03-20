

package com.google.common.eventbus;

import static com.google.common.base.Preconditions.checkNotNull;

import com.google.common.annotations.Beta;
import com.google.common.base.MoreObjects;
import com.google.common.util.concurrent.MoreExecutors;
import java.lang.reflect.Method;
import java.util.Iterator;
import java.util.Locale;
import java.util.concurrent.Executor;
import java.util.logging.Level;
import java.util.logging.Logger;


@Beta
public class EventBus {

  private static final Logger logger = Logger.getLogger(EventBus.class.getName());

  private final String identifier;
  private final Executor executor;
  private final SubscriberExceptionHandler exceptionHandler;

  private final SubscriberRegistry subscribers = new SubscriberRegistry(this);
  private final Dispatcher dispatcher;

  
  public EventBus() {
    this("default");
  }

  
  public EventBus(String identifier) {
    this(
        identifier,
        MoreExecutors.directExecutor(),
        Dispatcher.perThreadDispatchQueue(),
        LoggingHandler.INSTANCE);
  }

  
  public EventBus(SubscriberExceptionHandler exceptionHandler) {
    this(
        "default",
        MoreExecutors.directExecutor(),
        Dispatcher.perThreadDispatchQueue(),
        exceptionHandler);
  }

  EventBus(
      String identifier,
      Executor executor,
      Dispatcher dispatcher,
      SubscriberExceptionHandler exceptionHandler) {
    this.identifier = checkNotNull(identifier);
    this.executor = checkNotNull(executor);
    this.dispatcher = checkNotNull(dispatcher);
    this.exceptionHandler = checkNotNull(exceptionHandler);
  }

  
  public final String identifier() {
    return identifier;
  }

  
  final Executor executor() {
    return executor;
  }

  
  void handleSubscriberException(Throwable e, SubscriberExceptionContext context) {
    checkNotNull(e);
    checkNotNull(context);
    try {
      exceptionHandler.handleException(e, context);
    } catch (Throwable e2) {
            logger.log(
          Level.SEVERE,
          String.format(Locale.ROOT, "Exception %s thrown while handling exception: %s", e2, e),
          e2);
    }
  }

  
  public void register(Object object) {
    subscribers.register(object);
  }

  
  public void unregister(Object object) {
    subscribers.unregister(object);
  }

  
  public void post(Object event) {
    Iterator<Subscriber> eventSubscribers = subscribers.getSubscribers(event);
    if (eventSubscribers.hasNext()) {
      dispatcher.dispatch(event, eventSubscribers);
    } else if (!(event instanceof DeadEvent)) {
            post(new DeadEvent(this, event));
    }
  }

  @Override
  public String toString() {
    return MoreObjects.toStringHelper(this).addValue(identifier).toString();
  }

  
  static final class LoggingHandler implements SubscriberExceptionHandler {
    static final LoggingHandler INSTANCE = new LoggingHandler();

    @Override
    public void handleException(Throwable exception, SubscriberExceptionContext context) {
      Logger logger = logger(context);
      if (logger.isLoggable(Level.SEVERE)) {
        logger.log(Level.SEVERE, message(context), exception);
      }
    }

    private static Logger logger(SubscriberExceptionContext context) {
      return Logger.getLogger(EventBus.class.getName() + "." + context.getEventBus().identifier());
    }

    private static String message(SubscriberExceptionContext context) {
      Method method = context.getSubscriberMethod();
      return "Exception thrown by subscriber method "
          + method.getName()
          + '('
          + method.getParameterTypes()[0].getName()
          + ')'
          + " on subscriber "
          + context.getSubscriber()
          + " when dispatching event: "
          + context.getEvent();
    }
  }
}

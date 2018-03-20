
package com.iluwatar.async.method.invocation;

import org.junit.jupiter.api.Test;
import org.mockito.ArgumentCaptor;
import org.mockito.Matchers;

import java.util.Optional;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;

import static java.time.Duration.ofMillis;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertTimeout;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.junit.jupiter.api.Assertions.fail;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.internal.verification.VerificationModeFactory.times;


public class ThreadAsyncExecutorTest {

  
  @Test
  public void testSuccessfulTaskWithoutCallback() throws Exception {
    assertTimeout(ofMillis(3000), () -> {
            final ThreadAsyncExecutor executor = new ThreadAsyncExecutor();

      final Object result = new Object();
      final Callable<Object> task = mock(Callable.class);
      when(task.call()).thenReturn(result);

      final AsyncResult<Object> asyncResult = executor.startProcess(task);
      assertNotNull(asyncResult);
      asyncResult.await();       assertTrue(asyncResult.isCompleted());

            verify(task, times(1)).call();

            assertSame(result, asyncResult.getValue());
    });
  }

  
  @Test
  public void testSuccessfulTaskWithCallback() throws Exception {
    assertTimeout(ofMillis(3000), () -> {
            final ThreadAsyncExecutor executor = new ThreadAsyncExecutor();

      final Object result = new Object();
      final Callable<Object> task = mock(Callable.class);
      when(task.call()).thenReturn(result);

      final AsyncCallback callback = mock(AsyncCallback.class);
      final AsyncResult<Object> asyncResult = executor.startProcess(task, callback);
      assertNotNull(asyncResult);
      asyncResult.await();       assertTrue(asyncResult.isCompleted());

            verify(task, times(1)).call();

            final ArgumentCaptor<Optional<Exception>> optionalCaptor = ArgumentCaptor.forClass((Class) Optional.class);
      verify(callback, times(1)).onComplete(eq(result), optionalCaptor.capture());

      final Optional<Exception> optionalException = optionalCaptor.getValue();
      assertNotNull(optionalException);
      assertFalse(optionalException.isPresent());

            assertSame(result, asyncResult.getValue());
    });
  }

  
  @Test
  public void testLongRunningTaskWithoutCallback() throws Exception {
    assertTimeout(ofMillis(5000), () -> {
            final ThreadAsyncExecutor executor = new ThreadAsyncExecutor();

      final Object result = new Object();
      final Callable<Object> task = mock(Callable.class);
      when(task.call()).thenAnswer(i -> {
        Thread.sleep(1500);
        return result;
      });

      final AsyncResult<Object> asyncResult = executor.startProcess(task);
      assertNotNull(asyncResult);
      assertFalse(asyncResult.isCompleted());

      try {
        asyncResult.getValue();
        fail("Expected IllegalStateException when calling AsyncResult#getValue on a non-completed task");
      } catch (IllegalStateException e) {
        assertNotNull(e.getMessage());
      }

            verify(task, timeout(3000).times(1)).call();

            asyncResult.await();
      assertTrue(asyncResult.isCompleted());
      verifyNoMoreInteractions(task);

            assertSame(result, asyncResult.getValue());
    });
  }

  
  @Test
  public void testLongRunningTaskWithCallback() throws Exception {
    assertTimeout(ofMillis(5000), () -> {
            final ThreadAsyncExecutor executor = new ThreadAsyncExecutor();

      final Object result = new Object();
      final Callable<Object> task = mock(Callable.class);
      when(task.call()).thenAnswer(i -> {
        Thread.sleep(1500);
        return result;
      });

      final AsyncCallback<Object> callback = mock(AsyncCallback.class);
      final AsyncResult<Object> asyncResult = executor.startProcess(task, callback);
      assertNotNull(asyncResult);
      assertFalse(asyncResult.isCompleted());

      verifyZeroInteractions(callback);

      try {
        asyncResult.getValue();
        fail("Expected IllegalStateException when calling AsyncResult#getValue on a non-completed task");
      } catch (IllegalStateException e) {
        assertNotNull(e.getMessage());
      }

            verify(task, timeout(3000).times(1)).call();

      final ArgumentCaptor<Optional<Exception>> optionalCaptor = ArgumentCaptor.forClass((Class) Optional.class);
      verify(callback, timeout(3000).times(1)).onComplete(eq(result), optionalCaptor.capture());

      final Optional<Exception> optionalException = optionalCaptor.getValue();
      assertNotNull(optionalException);
      assertFalse(optionalException.isPresent());

            asyncResult.await();
      assertTrue(asyncResult.isCompleted());
      verifyNoMoreInteractions(task, callback);

            assertSame(result, asyncResult.getValue());
    });
  }

  
  @Test
  public void testEndProcess() throws Exception {
    assertTimeout(ofMillis(5000), () -> {
            final ThreadAsyncExecutor executor = new ThreadAsyncExecutor();

      final Object result = new Object();
      final Callable<Object> task = mock(Callable.class);
      when(task.call()).thenAnswer(i -> {
        Thread.sleep(1500);
        return result;
      });

      final AsyncResult<Object> asyncResult = executor.startProcess(task);
      assertNotNull(asyncResult);
      assertFalse(asyncResult.isCompleted());

      try {
        asyncResult.getValue();
        fail("Expected IllegalStateException when calling AsyncResult#getValue on a non-completed task");
      } catch (IllegalStateException e) {
        assertNotNull(e.getMessage());
      }

      assertSame(result, executor.endProcess(asyncResult));
      verify(task, times(1)).call();
      assertTrue(asyncResult.isCompleted());

            assertSame(result, executor.endProcess(asyncResult));
      verifyNoMoreInteractions(task);
    });
  }

  
  @Test
  public void testNullTask() throws Exception {
    assertTimeout(ofMillis(3000), () -> {
            final ThreadAsyncExecutor executor = new ThreadAsyncExecutor();
      final AsyncResult<Object> asyncResult = executor.startProcess(null);

      assertNotNull(asyncResult, "The AsyncResult should not be 'null', even though the task was 'null'.");
      asyncResult.await();       assertTrue(asyncResult.isCompleted());

      try {
        asyncResult.getValue();
        fail("Expected ExecutionException with NPE as cause");
      } catch (final ExecutionException e) {
        assertNotNull(e.getMessage());
        assertNotNull(e.getCause());
        assertEquals(NullPointerException.class, e.getCause().getClass());
      }
    });

  }

  
  @Test
  public void testNullTaskWithCallback() throws Exception {
    assertTimeout(ofMillis(3000), () -> {
            final ThreadAsyncExecutor executor = new ThreadAsyncExecutor();
      final AsyncCallback<Object> callback = mock(AsyncCallback.class);
      final AsyncResult<Object> asyncResult = executor.startProcess(null, callback);

      assertNotNull(asyncResult, "The AsyncResult should not be 'null', even though the task was 'null'.");
      asyncResult.await();       assertTrue(asyncResult.isCompleted());

      final ArgumentCaptor<Optional<Exception>> optionalCaptor = ArgumentCaptor.forClass((Class) Optional.class);
      verify(callback, times(1)).onComplete(Matchers.isNull(), optionalCaptor.capture());

      final Optional<Exception> optionalException = optionalCaptor.getValue();
      assertNotNull(optionalException);
      assertTrue(optionalException.isPresent());

      final Exception exception = optionalException.get();
      assertNotNull(exception);
      assertEquals(NullPointerException.class, exception.getClass());

      try {
        asyncResult.getValue();
        fail("Expected ExecutionException with NPE as cause");
      } catch (final ExecutionException e) {
        assertNotNull(e.getMessage());
        assertNotNull(e.getCause());
        assertEquals(NullPointerException.class, e.getCause().getClass());
      }
    });

  }

  
  @Test
  public void testNullTaskWithNullCallback() throws Exception {
    assertTimeout(ofMillis(3000), () -> {
            final ThreadAsyncExecutor executor = new ThreadAsyncExecutor();
      final AsyncResult<Object> asyncResult = executor.startProcess(null, null);

      assertNotNull(
          asyncResult,
          "The AsyncResult should not be 'null', even though the task and callback were 'null'."
      );
      asyncResult.await();       assertTrue(asyncResult.isCompleted());

      try {
        asyncResult.getValue();
        fail("Expected ExecutionException with NPE as cause");
      } catch (final ExecutionException e) {
        assertNotNull(e.getMessage());
        assertNotNull(e.getCause());
        assertEquals(NullPointerException.class, e.getCause().getClass());
      }
    });

  }

}
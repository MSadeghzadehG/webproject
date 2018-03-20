

package com.badlogic.gdx.utils.async;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

import com.badlogic.gdx.utils.Disposable;
import com.badlogic.gdx.utils.GdxRuntimeException;


public class AsyncExecutor implements Disposable {
	private final ExecutorService executor;

	
	public AsyncExecutor (int maxConcurrent) {
		executor = Executors.newFixedThreadPool(maxConcurrent, new ThreadFactory() {
			@Override
			public Thread newThread (Runnable r) {
				Thread thread = new Thread(r, "AsynchExecutor-Thread");
				thread.setDaemon(true);
				return thread;
			}
		});
	}

	
	public <T> AsyncResult<T> submit (final AsyncTask<T> task) {
		if (executor.isShutdown()) {
			throw new GdxRuntimeException("Cannot run tasks on an executor that has been shutdown (disposed)");
		}
		return new AsyncResult(executor.submit(new Callable<T>() {
			@Override
			public T call () throws Exception {
				return task.call();
			}
		}));
	}

	
	@Override
	public void dispose () {
		executor.shutdown();
		try {
			executor.awaitTermination(Long.MAX_VALUE, TimeUnit.SECONDS);
		} catch (InterruptedException e) {
			throw new GdxRuntimeException("Couldn't shutdown loading thread", e);
		}
	}
}

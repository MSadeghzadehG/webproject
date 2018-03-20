
package com.fernandocejas.android10.sample.data.executor;

import android.support.annotation.NonNull;
import com.fernandocejas.android10.sample.domain.executor.ThreadExecutor;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import javax.inject.Inject;
import javax.inject.Singleton;


@Singleton
public class JobExecutor implements ThreadExecutor {
  private final ThreadPoolExecutor threadPoolExecutor;

  @Inject
  JobExecutor() {
    this.threadPoolExecutor = new ThreadPoolExecutor(3, 5, 10, TimeUnit.SECONDS,
        new LinkedBlockingQueue<>(), new JobThreadFactory());
  }

  @Override public void execute(@NonNull Runnable runnable) {
    this.threadPoolExecutor.execute(runnable);
  }

  private static class JobThreadFactory implements ThreadFactory {
    private int counter = 0;

    @Override public Thread newThread(@NonNull Runnable runnable) {
      return new Thread(runnable, "android_" + counter++);
    }
  }
}


package com.iluwatar.halfsynchalfasync;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.FutureTask;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;


public class AsynchronousService {

  
  private ExecutorService service;

  
  public AsynchronousService(BlockingQueue<Runnable> workQueue) {
    service = new ThreadPoolExecutor(10, 10, 10, TimeUnit.SECONDS, workQueue);
  }


  
  public <T> void execute(final AsyncTask<T> task) {
    try {
            task.onPreCall();
    } catch (Exception e) {
      task.onError(e);
      return;
    }

    service.submit(new FutureTask<T>(task) {
      @Override
      protected void done() {
        super.done();
        try {
          
          task.onPostCall(get());
        } catch (InterruptedException e) {
                  } catch (ExecutionException e) {
          task.onError(e.getCause());
        }
      }
    });
  }
}

package hudson.model.queue;

import hudson.model.Queue.Executable;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;


public interface QueueTaskFuture<R extends Executable> extends Future<R> {
    
    Future<R> getStartCondition();

    
    R waitForStart() throws InterruptedException, ExecutionException;

}

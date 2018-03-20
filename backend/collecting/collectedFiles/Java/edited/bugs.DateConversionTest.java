
package hudson.bugs;

import com.thoughtworks.xstream.converters.basic.DateConverter;
import org.junit.Test;
import org.jvnet.hudson.test.Email;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;


@Email("http:public class DateConversionTest {
    
    @Test
    public void test() throws Exception {
        final DateConverter dc =new DateConverter();
        ExecutorService es = Executors.newFixedThreadPool(10);

        List<Future> futures = new ArrayList<Future>();
        for(int i=0;i<10;i++) {
            futures.add(es.submit(new Callable<Object>() {
                public Object call() throws Exception {
                    for( int i=0; i<10000; i++ )
                        dc.fromString("2008-08-26 15:40:14.568 GMT-03:00");
                    return null;
                }
            }));
        }

        for (Future f : futures) {
            f.get();
        }
        es.shutdown();
    }
}

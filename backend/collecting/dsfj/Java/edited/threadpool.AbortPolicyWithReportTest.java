
package com.alibaba.dubbo.common.threadpool;

import com.alibaba.dubbo.common.URL;
import com.alibaba.dubbo.common.threadpool.support.AbortPolicyWithReport;
import org.junit.Test;

import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.ThreadPoolExecutor;

public class AbortPolicyWithReportTest {
    @Test
    public void jStackDumpTest() throws InterruptedException {
        URL url = URL.valueOf("dubbo:        AbortPolicyWithReport abortPolicyWithReport = new AbortPolicyWithReport("Test", url);

        try {
            abortPolicyWithReport.rejectedExecution(new Runnable() {
                @Override
                public void run() {
                    System.out.println("hello");
                }
            }, (ThreadPoolExecutor) Executors.newFixedThreadPool(1));
        }catch (RejectedExecutionException rj){

        }

        Thread.currentThread().sleep(1000);

    }
}
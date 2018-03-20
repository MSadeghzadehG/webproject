
package com.alibaba.dubbo.common.utils;

import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.spi.LoggingEvent;

import java.util.ArrayList;
import java.util.List;

public class DubboAppender extends ConsoleAppender {

    public static boolean available = false;

    public static List<Log> logList = new ArrayList<Log>();

    public static void doStart() {
        available = true;
    }

    public static void doStop() {
        available = false;
    }

    public static void clear() {
        logList.clear();
    }

    public void append(LoggingEvent event) {
        super.append(event);
        if (available) {
            Log temp = parseLog(event);
            logList.add(temp);
        }
    }

    private Log parseLog(LoggingEvent event) {
        Log log = new Log();
        log.setLogName(event.getLogger().getName());
        log.setLogLevel(event.getLevel());
        log.setLogThread(event.getThreadName());
        log.setLogMessage(event.getMessage().toString());
        return log;
    }

}

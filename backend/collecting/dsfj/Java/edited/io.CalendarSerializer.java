

package com.alibaba.com.caucho.hessian.io;

import java.io.IOException;
import java.util.Calendar;


public class CalendarSerializer extends AbstractSerializer {
    private static CalendarSerializer SERIALIZER = new CalendarSerializer();

    public static CalendarSerializer create() {
        return SERIALIZER;
    }

    public void writeObject(Object obj, AbstractHessianOutput out)
            throws IOException {
        if (obj == null)
            out.writeNull();
        else {
            Calendar cal = (Calendar) obj;

            out.writeObject(new CalendarHandle(cal.getClass(),
                    cal.getTimeInMillis()));
        }
    }
}

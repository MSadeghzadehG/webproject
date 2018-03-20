

package com.alibaba.com.caucho.hessian.io;

import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;


public class CalendarHandle implements java.io.Serializable, HessianHandle {
    private Class type;
    private Date date;

    public CalendarHandle() {
    }

    public CalendarHandle(Class type, long time) {
        if (!GregorianCalendar.class.equals(type))
            this.type = type;

        this.date = new Date(time);
    }

    private Object readResolve() {
        try {
            Calendar cal;

            if (this.type != null)
                cal = (Calendar) this.type.newInstance();
            else
                cal = new GregorianCalendar();

            cal.setTimeInMillis(this.date.getTime());

            return cal;
        } catch (RuntimeException e) {
            throw e;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
}

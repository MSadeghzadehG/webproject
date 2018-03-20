

package io.reactivex.schedulers;

import java.util.concurrent.TimeUnit;

import io.reactivex.annotations.NonNull;
import io.reactivex.internal.functions.ObjectHelper;


public final class Timed<T> {
    final T value;
    final long time;
    final TimeUnit unit;

    
    public Timed(@NonNull T value, long time, @NonNull TimeUnit unit) {
        this.value = value;
        this.time = time;
        this.unit = ObjectHelper.requireNonNull(unit, "unit is null");
    }

    
    @NonNull
    public T value() {
        return value;
    }

    
    @NonNull
    public TimeUnit unit() {
        return unit;
    }

    
    public long time() {
        return time;
    }

    
    public long time(@NonNull TimeUnit unit) {
        return unit.convert(time, this.unit);
    }

    @Override
    public boolean equals(Object other) {
        if (other instanceof Timed) {
            Timed<?> o = (Timed<?>) other;
            return ObjectHelper.equals(value, o.value)
                    && time == o.time
                    && ObjectHelper.equals(unit, o.unit);
        }
        return false;
    }

    @Override
    public int hashCode() {
         int h = value != null ? value.hashCode() : 0;
         h = h * 31 + (int)((time >>> 31) ^ time);
         h = h * 31 + unit.hashCode();
         return h;
    }

    @Override
    public String toString() {
        return "Timed[time=" + time + ", unit=" + unit + ", value=" + value + "]";
    }
}

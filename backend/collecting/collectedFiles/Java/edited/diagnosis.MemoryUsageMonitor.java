
package hudson.diagnosis;

import java.util.concurrent.TimeUnit;
import hudson.util.ColorPalette;
import hudson.Extension;
import hudson.model.PeriodicWork;
import hudson.model.MultiStageTimeSeries;
import hudson.model.MultiStageTimeSeries.TrendChart;
import hudson.model.MultiStageTimeSeries.TimeScale;

import java.lang.management.MemoryPoolMXBean;
import java.lang.management.MemoryType;
import java.lang.management.MemoryUsage;
import java.lang.management.ManagementFactory;
import java.util.List;
import java.util.ArrayList;
import java.io.IOException;

import org.jenkinsci.Symbol;
import org.kohsuke.stapler.QueryParameter;


@Extension @Symbol("memoryUsage")
public final class MemoryUsageMonitor extends PeriodicWork {
    
    public final class MemoryGroup {
        private final List<MemoryPoolMXBean> pools = new ArrayList<MemoryPoolMXBean>();

        
        public final MultiStageTimeSeries used = new MultiStageTimeSeries(Messages._MemoryUsageMonitor_USED(), ColorPalette.RED, 0,0);
        
        public final MultiStageTimeSeries max = new MultiStageTimeSeries(Messages._MemoryUsageMonitor_TOTAL(), ColorPalette.BLUE, 0,0);

        private MemoryGroup(List<MemoryPoolMXBean> pools, MemoryType type) {
            for (MemoryPoolMXBean pool : pools) {
                if (pool.getType() == type)
                    this.pools.add(pool);
            }
        }

        private void update() {
            long used = 0;
            long max = 0;
            for (MemoryPoolMXBean pool : pools) {
                MemoryUsage usage = pool.getCollectionUsage();
                if(usage==null) continue;                   used += usage.getUsed();
                max  += usage.getMax();

            }

                        used /= 1024;
            max /= 1024;

            this.used.update(used);
            this.max.update(max);
        }

        
        public TrendChart doGraph(@QueryParameter String type) throws IOException {
            return MultiStageTimeSeries.createTrendChart(TimeScale.parse(type),used,max);
        }
    }

    public final MemoryGroup heap;
    public final MemoryGroup nonHeap;

    public MemoryUsageMonitor() {
        List<MemoryPoolMXBean> pools = ManagementFactory.getMemoryPoolMXBeans();
        heap = new MemoryGroup(pools, MemoryType.HEAP);
        nonHeap = new MemoryGroup(pools, MemoryType.NON_HEAP);
    }

    public long getRecurrencePeriod() {
        return TimeUnit.SECONDS.toMillis(10);
    }

    protected void doRun() {
        heap.update();
        nonHeap.update();
    }
}



package org.elasticsearch.cloud.gce;

import com.google.api.services.compute.model.Instance;
import org.elasticsearch.common.component.LifecycleComponent;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Setting.Property;
import org.elasticsearch.common.unit.TimeValue;

import java.io.Closeable;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.function.Function;

public interface GceInstancesService extends Closeable {

    
    String VERSION = "Elasticsearch/GceCloud/1.0";

    
    
    Setting<String> PROJECT_SETTING = Setting.simpleString("cloud.gce.project_id", Property.NodeScope);

    
    Setting<List<String>> ZONE_SETTING =
        Setting.listSetting("cloud.gce.zone", Collections.emptyList(), Function.identity(), Property.NodeScope);

    
    Setting<TimeValue> REFRESH_SETTING =
        Setting.timeSetting("cloud.gce.refresh_interval", TimeValue.timeValueSeconds(0), Property.NodeScope);

    
    Setting<Boolean> RETRY_SETTING = Setting.boolSetting("cloud.gce.retry", true, Property.NodeScope);

    
    Setting<TimeValue> MAX_WAIT_SETTING =
        Setting.timeSetting("cloud.gce.max_wait", TimeValue.timeValueSeconds(-1), Property.NodeScope);

    
    Collection<Instance> instances();
}

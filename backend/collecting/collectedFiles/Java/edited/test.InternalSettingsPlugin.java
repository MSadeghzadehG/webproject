
package org.elasticsearch.test;

import org.elasticsearch.cluster.metadata.IndexMetaData;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Setting.Property;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.index.IndexService;
import org.elasticsearch.plugins.Plugin;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.TimeUnit;

public final class InternalSettingsPlugin extends Plugin {

    public static final Setting<Integer> VERSION_CREATED =
        Setting.intSetting("index.version.created", 0, Property.IndexScope, Property.NodeScope);
    public static final Setting<String> PROVIDED_NAME_SETTING =
        Setting.simpleString("index.provided_name",Property.IndexScope, Property.NodeScope);
    public static final Setting<Boolean> MERGE_ENABLED =
        Setting.boolSetting("index.merge.enabled", true, Property.IndexScope, Property.NodeScope);
    public static final Setting<Long> INDEX_CREATION_DATE_SETTING =
        Setting.longSetting(IndexMetaData.SETTING_CREATION_DATE, -1, -1, Property.IndexScope, Property.NodeScope);
    public static final Setting<TimeValue> TRANSLOG_RETENTION_CHECK_INTERVAL_SETTING =
        Setting.timeSetting("index.translog.retention.check_interval", new TimeValue(10, TimeUnit.MINUTES),
            new TimeValue(-1, TimeUnit.MILLISECONDS), Property.Dynamic, Property.IndexScope);

    @Override
    public List<Setting<?>> getSettings() {
        return Arrays.asList(
                VERSION_CREATED,
                MERGE_ENABLED,
                INDEX_CREATION_DATE_SETTING,
                PROVIDED_NAME_SETTING,
                TRANSLOG_RETENTION_CHECK_INTERVAL_SETTING,
                IndexService.GLOBAL_CHECKPOINT_SYNC_INTERVAL_SETTING);
    }
}

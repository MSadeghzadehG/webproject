

package org.elasticsearch.client.documentation;

import org.elasticsearch.action.ActionListener;
import org.elasticsearch.action.LatchedActionListener;
import org.elasticsearch.action.admin.cluster.settings.ClusterUpdateSettingsRequest;
import org.elasticsearch.action.admin.cluster.settings.ClusterUpdateSettingsResponse;
import org.elasticsearch.client.ESRestHighLevelClientTestCase;
import org.elasticsearch.client.RestHighLevelClient;
import org.elasticsearch.cluster.routing.allocation.decider.EnableAllocationDecider;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.unit.ByteSizeUnit;
import org.elasticsearch.common.unit.TimeValue;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.indices.recovery.RecoverySettings;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import static org.hamcrest.Matchers.equalTo;


public class ClusterClientDocumentationIT extends ESRestHighLevelClientTestCase {

    public void testClusterPutSettings() throws IOException {
        RestHighLevelClient client = highLevelClient();

                ClusterUpdateSettingsRequest request = new ClusterUpdateSettingsRequest();
        
                String transientSettingKey = 
                RecoverySettings.INDICES_RECOVERY_MAX_BYTES_PER_SEC_SETTING.getKey();
        int transientSettingValue = 10;
        Settings transientSettings = 
                Settings.builder()
                .put(transientSettingKey, transientSettingValue, ByteSizeUnit.BYTES)
                .build(); 
        String persistentSettingKey = 
                EnableAllocationDecider.CLUSTER_ROUTING_ALLOCATION_ENABLE_SETTING.getKey();
        String persistentSettingValue = 
                EnableAllocationDecider.Allocation.NONE.name();
        Settings persistentSettings = 
                Settings.builder()
                .put(persistentSettingKey, persistentSettingValue)
                .build();         
                request.transientSettings(transientSettings);         request.persistentSettings(persistentSettings);         
        {
                        Settings.Builder transientSettingsBuilder = 
                    Settings.builder()
                    .put(transientSettingKey, transientSettingValue, ByteSizeUnit.BYTES); 
            request.transientSettings(transientSettingsBuilder);                     }
        {
                        Map<String, Object> map = new HashMap<>();
            map.put(transientSettingKey
                    , transientSettingValue + ByteSizeUnit.BYTES.getSuffix());
            request.transientSettings(map);                     }
        {
                        request.transientSettings(
                    "{\"indices.recovery.max_bytes_per_sec\": \"10b\"}"
                    , XContentType.JSON);                     }

                request.timeout(TimeValue.timeValueMinutes(2));         request.timeout("2m");                         request.masterNodeTimeout(TimeValue.timeValueMinutes(1));         request.masterNodeTimeout("1m");         
                request.flatSettings(true);         
                ClusterUpdateSettingsResponse response = client.cluster().putSettings(request);
        
                boolean acknowledged = response.isAcknowledged();         Settings transientSettingsResponse = response.getTransientSettings();         Settings persistentSettingsResponse = response.getPersistentSettings();                 assertTrue(acknowledged);
        assertThat(transientSettingsResponse.get(transientSettingKey), equalTo(transientSettingValue + ByteSizeUnit.BYTES.getSuffix()));
        assertThat(persistentSettingsResponse.get(persistentSettingKey), equalTo(persistentSettingValue));

                request.transientSettings(Settings.builder().putNull(transientSettingKey).build());                 request.persistentSettings(Settings.builder().putNull(persistentSettingKey));
        ClusterUpdateSettingsResponse resetResponse = client.cluster().putSettings(request);

        assertTrue(resetResponse.isAcknowledged());
    }

    public void testClusterUpdateSettingsAsync() throws Exception {
        RestHighLevelClient client = highLevelClient();
        {
            ClusterUpdateSettingsRequest request = new ClusterUpdateSettingsRequest();

                        ActionListener<ClusterUpdateSettingsResponse> listener = 
                    new ActionListener<ClusterUpdateSettingsResponse>() {
                @Override
                public void onResponse(ClusterUpdateSettingsResponse response) {
                                    }

                @Override
                public void onFailure(Exception e) {
                                    }
            };
            
                        final CountDownLatch latch = new CountDownLatch(1);
            listener = new LatchedActionListener<>(listener, latch);

                        client.cluster().putSettingsAsync(request, listener);             
            assertTrue(latch.await(30L, TimeUnit.SECONDS));
        }
    }
}

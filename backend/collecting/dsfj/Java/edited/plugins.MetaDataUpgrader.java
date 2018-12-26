

package org.elasticsearch.plugins;

import org.elasticsearch.cluster.metadata.IndexTemplateMetaData;
import org.elasticsearch.cluster.metadata.MetaData;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.function.UnaryOperator;


public class MetaDataUpgrader {
    public final UnaryOperator<Map<String, MetaData.Custom>> customMetaDataUpgraders;

    public final UnaryOperator<Map<String, IndexTemplateMetaData>> indexTemplateMetaDataUpgraders;

    public MetaDataUpgrader(Collection<UnaryOperator<Map<String, MetaData.Custom>>> customMetaDataUpgraders,
                            Collection<UnaryOperator<Map<String, IndexTemplateMetaData>>> indexTemplateMetaDataUpgraders) {
        this.customMetaDataUpgraders = customs -> {
            Map<String, MetaData.Custom> upgradedCustoms = new HashMap<>(customs);
            for (UnaryOperator<Map<String, MetaData.Custom>> customMetaDataUpgrader : customMetaDataUpgraders) {
                upgradedCustoms = customMetaDataUpgrader.apply(upgradedCustoms);
            }
            return upgradedCustoms;
        };

        this.indexTemplateMetaDataUpgraders = templates -> {
            Map<String, IndexTemplateMetaData> upgradedTemplates = new HashMap<>(templates);
            for (UnaryOperator<Map<String, IndexTemplateMetaData>> upgrader : indexTemplateMetaDataUpgraders) {
                upgradedTemplates = upgrader.apply(upgradedTemplates);
            }
            return upgradedTemplates;
        };
    }
}

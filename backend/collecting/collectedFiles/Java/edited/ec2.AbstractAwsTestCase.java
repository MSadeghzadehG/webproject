

package org.elasticsearch.discovery.ec2;

import org.elasticsearch.common.Strings;
import org.elasticsearch.common.io.PathUtils;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.settings.SettingsException;
import org.elasticsearch.env.Environment;
import org.elasticsearch.plugins.Plugin;
import org.elasticsearch.test.ESIntegTestCase;
import org.elasticsearch.test.ESIntegTestCase.ThirdParty;

import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;


@ThirdParty
public abstract class AbstractAwsTestCase extends ESIntegTestCase {

    @Override
    protected Settings nodeSettings(int nodeOrdinal) {
                Settings.Builder settings = Settings.builder()
                .put(super.nodeSettings(nodeOrdinal))
                .put(Environment.PATH_HOME_SETTING.getKey(), createTempDir());

                try {
            if (Strings.hasText(System.getProperty("tests.config"))) {
                try {
                    settings.loadFromPath(PathUtils.get(System.getProperty("tests.config")));
                } catch (IOException e) {
                    throw new IllegalArgumentException("could not load aws tests config", e);
                }
            } else {
                throw new IllegalStateException("to run integration tests, you need to set -Dtests.thirdparty=true and -Dtests.config=/path/to/elasticsearch.yml");
            }
        } catch (SettingsException exception) {
            throw new IllegalStateException("your test configuration file is incorrect: " + System.getProperty("tests.config"), exception);
        }
        return settings.build();
    }

    @Override
    protected Collection<Class<? extends Plugin>> nodePlugins() {
        return Arrays.asList(Ec2DiscoveryPlugin.class);
    }

    @Override
    protected boolean addTestZenDiscovery() {
        return false;
    }
}

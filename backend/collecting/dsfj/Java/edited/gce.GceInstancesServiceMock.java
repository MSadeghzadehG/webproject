

package org.elasticsearch.discovery.gce;

import com.google.api.client.http.HttpTransport;
import org.elasticsearch.cloud.gce.GceInstancesServiceImpl;
import org.elasticsearch.common.settings.Settings;

import java.io.IOException;
import java.security.GeneralSecurityException;

public class GceInstancesServiceMock extends GceInstancesServiceImpl {

    protected HttpTransport mockHttpTransport;

    public GceInstancesServiceMock(Settings settings) {
        super(settings);
        this.mockHttpTransport = GceMockUtils.configureMock();
    }

    @Override
    protected HttpTransport getGceHttpTransport() throws GeneralSecurityException, IOException {
        return this.mockHttpTransport;
    }
}

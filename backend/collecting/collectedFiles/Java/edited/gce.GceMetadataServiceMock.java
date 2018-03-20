

package org.elasticsearch.discovery.gce;

import com.google.api.client.http.HttpTransport;
import org.elasticsearch.cloud.gce.GceMetadataService;
import org.elasticsearch.common.settings.Settings;

import java.io.IOException;
import java.security.GeneralSecurityException;


public class GceMetadataServiceMock extends GceMetadataService {

    protected HttpTransport mockHttpTransport;

    public GceMetadataServiceMock(Settings settings) {
        super(settings);
        this.mockHttpTransport = GceMockUtils.configureMock();
    }

    @Override
    protected HttpTransport getGceHttpTransport() throws GeneralSecurityException, IOException {
        return this.mockHttpTransport;
    }
}

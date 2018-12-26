

package org.elasticsearch.cloud.gce;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.security.AccessController;
import java.security.GeneralSecurityException;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.util.function.Function;

import com.google.api.client.googleapis.javanet.GoogleNetHttpTransport;
import com.google.api.client.http.GenericUrl;
import com.google.api.client.http.HttpHeaders;
import com.google.api.client.http.HttpResponse;
import com.google.api.client.http.HttpTransport;
import org.elasticsearch.SpecialPermission;
import org.elasticsearch.cloud.gce.util.Access;
import org.elasticsearch.common.component.AbstractLifecycleComponent;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Settings;

public class GceMetadataService extends AbstractLifecycleComponent {

                    public static final Setting<String> GCE_HOST =
        new Setting<>("cloud.gce.host", "http:
    
    private HttpTransport gceHttpTransport;

    public GceMetadataService(Settings settings) {
        super(settings);
    }

    protected synchronized HttpTransport getGceHttpTransport() throws GeneralSecurityException, IOException {
        if (gceHttpTransport == null) {
            gceHttpTransport = GoogleNetHttpTransport.newTrustedTransport();
        }
        return gceHttpTransport;
    }

    public String metadata(String metadataPath) throws IOException, URISyntaxException {
                                final URI urlMetadataNetwork = new URI(GCE_HOST.get(settings)).resolve("/computeMetadata/v1/instance/").resolve(metadataPath);
        logger.debug("get metadata from [{}]", urlMetadataNetwork);
        HttpHeaders headers;
        try {
                                    headers = Access.doPrivileged(HttpHeaders::new);
            GenericUrl genericUrl = Access.doPrivileged(() -> new GenericUrl(urlMetadataNetwork));

                        headers.put("Metadata-Flavor", "Google");
            HttpResponse response = Access.doPrivilegedIOException(() ->
                getGceHttpTransport().createRequestFactory()
                    .buildGetRequest(genericUrl)
                    .setHeaders(headers)
                    .execute());
            String metadata = response.parseAsString();
            logger.debug("metadata found [{}]", metadata);
            return metadata;
        } catch (Exception e) {
            throw new IOException("failed to fetch metadata from [" + urlMetadataNetwork + "]", e);
        }
    }

    @Override
    protected void doStart() {

    }

    @Override
    protected void doStop() {
        if (gceHttpTransport != null) {
            try {
                gceHttpTransport.shutdown();
            } catch (IOException e) {
                logger.warn("unable to shutdown GCE Http Transport", e);
            }
            gceHttpTransport = null;
        }
    }

    @Override
    protected void doClose() {

    }
}

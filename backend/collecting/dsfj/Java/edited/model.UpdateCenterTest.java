
package hudson.model;

import com.trilead.ssh2.crypto.Base64;
import java.util.concurrent.TimeUnit;
import net.sf.json.JSONObject;

import java.io.ByteArrayInputStream;
import java.net.URL;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.Date;
import static org.junit.Assert.*;
import static org.junit.Assume.*;
import org.junit.Test;


public class UpdateCenterTest {
    @Test public void data() throws Exception {
        try {
            doData("https:            doData("https:        } catch (Exception x) {
                        assumeNoException("Might be no Internet connectivity, or might start failing due to expiring certificate through no fault of code changes", x);
        }
    }
    private void doData(String location) throws Exception {
        URL url = new URL(location);
        String jsonp = DownloadService.loadJSON(url);
        JSONObject json = JSONObject.fromObject(jsonp);

        UpdateSite us = new UpdateSite("default", url.toExternalForm());
        UpdateSite.Data data = us.new Data(json);
        assertTrue(data.core.url.startsWith("http:        assertTrue(data.plugins.containsKey("rake"));
        System.out.println(data.core.url);

                CertificateFactory cf = CertificateFactory.getInstance("X509");
        JSONObject signature = json.getJSONObject("signature");
        for (Object cert : signature.getJSONArray("certificates")) {
            X509Certificate c = (X509Certificate) cf.generateCertificate(new ByteArrayInputStream(Base64.decode(cert.toString().toCharArray())));
            c.checkValidity(new Date(System.currentTimeMillis() + TimeUnit.DAYS.toMillis(30)));
        }
    }
}

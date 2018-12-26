

package org.elasticsearch.discovery.ec2;

import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.component.AbstractComponent;
import org.elasticsearch.common.network.NetworkService.CustomNameResolver;
import org.elasticsearch.common.settings.Settings;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.URL;
import java.net.URLConnection;
import java.nio.charset.StandardCharsets;


class Ec2NameResolver extends AbstractComponent implements CustomNameResolver {

    
    private enum Ec2HostnameType {

        PRIVATE_IPv4("ec2:privateIpv4", "local-ipv4"),
        PRIVATE_DNS("ec2:privateDns", "local-hostname"),
        PUBLIC_IPv4("ec2:publicIpv4", "public-ipv4"),
        PUBLIC_DNS("ec2:publicDns", "public-hostname"),

                PUBLIC_IP("ec2:publicIp", PUBLIC_IPv4.ec2Name),
        PRIVATE_IP("ec2:privateIp", PRIVATE_IPv4.ec2Name),
        EC2("ec2", PRIVATE_IPv4.ec2Name);

        final String configName;
        final String ec2Name;

        Ec2HostnameType(String configName, String ec2Name) {
            this.configName = configName;
            this.ec2Name = ec2Name;
        }
    }

    
    Ec2NameResolver(Settings settings) {
        super(settings);
    }

    
    @SuppressForbidden(reason = "We call getInputStream in doPrivileged and provide SocketPermission")
    public InetAddress[] resolve(Ec2HostnameType type) throws IOException {
        InputStream in = null;
        String metadataUrl = AwsEc2ServiceImpl.EC2_METADATA_URL + type.ec2Name;
        try {
            URL url = new URL(metadataUrl);
            logger.debug("obtaining ec2 hostname from ec2 meta-data url {}", url);
            URLConnection urlConnection = SocketAccess.doPrivilegedIOException(url::openConnection);
            urlConnection.setConnectTimeout(2000);
            in = SocketAccess.doPrivilegedIOException(urlConnection::getInputStream);
            BufferedReader urlReader = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8));

            String metadataResult = urlReader.readLine();
            if (metadataResult == null || metadataResult.length() == 0) {
                throw new IOException("no gce metadata returned from [" + url + "] for [" + type.configName + "]");
            }
                        return new InetAddress[] { InetAddress.getByName(metadataResult) };
        } catch (IOException e) {
            throw new IOException("IOException caught when fetching InetAddress from [" + metadataUrl + "]", e);
        } finally {
            IOUtils.closeWhileHandlingException(in);
        }
    }

    @Override
    public InetAddress[] resolveDefault() {
        return null;     }

    @Override
    public InetAddress[] resolveIfPossible(String value) throws IOException {
        for (Ec2HostnameType type : Ec2HostnameType.values()) {
            if (type.configName.equals(value)) {
                return resolve(type);
            }
        }
        return null;
    }

}

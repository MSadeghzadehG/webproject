
package com.alibaba.dubbo.rpc.protocol.hessian;

import com.caucho.hessian.client.HessianConnection;
import com.caucho.hessian.client.HessianConnectionFactory;
import com.caucho.hessian.client.HessianProxyFactory;
import org.apache.http.client.HttpClient;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.params.HttpConnectionParams;

import java.io.IOException;
import java.net.URL;


public class HttpClientConnectionFactory implements HessianConnectionFactory {

    private final HttpClient httpClient = new DefaultHttpClient();

    public void setHessianProxyFactory(HessianProxyFactory factory) {
        HttpConnectionParams.setConnectionTimeout(httpClient.getParams(), (int) factory.getConnectTimeout());
        HttpConnectionParams.setSoTimeout(httpClient.getParams(), (int) factory.getReadTimeout());
    }

    public HessianConnection open(URL url) throws IOException {
        return new HttpClientConnection(httpClient, url);
    }

}
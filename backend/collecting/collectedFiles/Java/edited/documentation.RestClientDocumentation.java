

package org.elasticsearch.client.documentation;

import org.apache.http.Header;
import org.apache.http.HttpEntity;
import org.apache.http.HttpHost;
import org.apache.http.RequestLine;
import org.apache.http.auth.AuthScope;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.client.CredentialsProvider;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.entity.ContentType;
import org.apache.http.impl.client.BasicCredentialsProvider;
import org.apache.http.impl.nio.client.HttpAsyncClientBuilder;
import org.apache.http.impl.nio.reactor.IOReactorConfig;
import org.apache.http.message.BasicHeader;
import org.apache.http.nio.entity.NStringEntity;
import org.apache.http.ssl.SSLContextBuilder;
import org.apache.http.ssl.SSLContexts;
import org.apache.http.util.EntityUtils;
import org.elasticsearch.client.HttpAsyncResponseConsumerFactory;
import org.elasticsearch.client.Response;
import org.elasticsearch.client.ResponseListener;
import org.elasticsearch.client.RestClient;
import org.elasticsearch.client.RestClientBuilder;

import javax.net.ssl.SSLContext;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.KeyStore;
import java.util.Collections;
import java.util.Map;
import java.util.concurrent.CountDownLatch;


@SuppressWarnings("unused")
public class RestClientDocumentation {

    @SuppressWarnings("unused")
    public void testUsage() throws IOException, InterruptedException {

                RestClient restClient = RestClient.builder(
                new HttpHost("localhost", 9200, "http"),
                new HttpHost("localhost", 9201, "http")).build();
        
                restClient.close();
        
        {
                        RestClientBuilder builder = RestClient.builder(new HttpHost("localhost", 9200, "http"));
            Header[] defaultHeaders = new Header[]{new BasicHeader("header", "value")};
            builder.setDefaultHeaders(defaultHeaders);                     }
        {
                        RestClientBuilder builder = RestClient.builder(new HttpHost("localhost", 9200, "http"));
            builder.setMaxRetryTimeoutMillis(10000);                     }
        {
                        RestClientBuilder builder = RestClient.builder(new HttpHost("localhost", 9200, "http"));
            builder.setFailureListener(new RestClient.FailureListener() {
                @Override
                public void onFailure(HttpHost host) {
                                    }
            });
                    }
        {
                        RestClientBuilder builder = RestClient.builder(new HttpHost("localhost", 9200, "http"));
            builder.setRequestConfigCallback(new RestClientBuilder.RequestConfigCallback() {
                @Override
                public RequestConfig.Builder customizeRequestConfig(RequestConfig.Builder requestConfigBuilder) {
                    return requestConfigBuilder.setSocketTimeout(10000);                 }
            });
                    }
        {
                        RestClientBuilder builder = RestClient.builder(new HttpHost("localhost", 9200, "http"));
            builder.setHttpClientConfigCallback(new RestClientBuilder.HttpClientConfigCallback() {
                @Override
                public HttpAsyncClientBuilder customizeHttpClient(HttpAsyncClientBuilder httpClientBuilder) {
                    return httpClientBuilder.setProxy(new HttpHost("proxy", 9000, "http"));                  }
            });
                    }

        {
                        Response response = restClient.performRequest("GET", "/");                     }
        {
                        Response response = restClient.performRequest("GET", "/", new BasicHeader("header", "value"));
                    }
        {
                        Map<String, String> params = Collections.singletonMap("pretty", "true");
            Response response = restClient.performRequest("GET", "/", params);                     }
        {
                        Map<String, String> params = Collections.emptyMap();
            String jsonString = "{" +
                        "\"user\":\"kimchy\"," +
                        "\"postDate\":\"2013-01-30\"," +
                        "\"message\":\"trying out Elasticsearch\"" +
                    "}";
            HttpEntity entity = new NStringEntity(jsonString, ContentType.APPLICATION_JSON);
            Response response = restClient.performRequest("PUT", "/posts/doc/1", params, entity);                     }
        {
                        Map<String, String> params = Collections.emptyMap();
            HttpAsyncResponseConsumerFactory.HeapBufferedResponseConsumerFactory consumerFactory =
                    new HttpAsyncResponseConsumerFactory.HeapBufferedResponseConsumerFactory(30 * 1024 * 1024);
            Response response = restClient.performRequest("GET", "/posts/_search", params, null, consumerFactory);                     }
        {
                        ResponseListener responseListener = new ResponseListener() {
                @Override
                public void onSuccess(Response response) {
                                    }

                @Override
                public void onFailure(Exception exception) {
                                    }
            };
            restClient.performRequestAsync("GET", "/", responseListener);             
                        Header[] headers = {
                    new BasicHeader("header1", "value1"),
                    new BasicHeader("header2", "value2")
            };
            restClient.performRequestAsync("GET", "/", responseListener, headers);
            
                        Map<String, String> params = Collections.singletonMap("pretty", "true");
            restClient.performRequestAsync("GET", "/", params, responseListener);             
                        String jsonString = "{" +
                    "\"user\":\"kimchy\"," +
                    "\"postDate\":\"2013-01-30\"," +
                    "\"message\":\"trying out Elasticsearch\"" +
                    "}";
            HttpEntity entity = new NStringEntity(jsonString, ContentType.APPLICATION_JSON);
            restClient.performRequestAsync("PUT", "/posts/doc/1", params, entity, responseListener);             
                        HttpAsyncResponseConsumerFactory.HeapBufferedResponseConsumerFactory consumerFactory =
                    new HttpAsyncResponseConsumerFactory.HeapBufferedResponseConsumerFactory(30 * 1024 * 1024);
            restClient.performRequestAsync("GET", "/posts/_search", params, null, consumerFactory, responseListener);                     }
        {
                        Response response = restClient.performRequest("GET", "/");
            RequestLine requestLine = response.getRequestLine();             HttpHost host = response.getHost();             int statusCode = response.getStatusLine().getStatusCode();             Header[] headers = response.getHeaders();             String responseBody = EntityUtils.toString(response.getEntity());                     }
        {
            HttpEntity[] documents = new HttpEntity[10];
                        final CountDownLatch latch = new CountDownLatch(documents.length);
            for (int i = 0; i < documents.length; i++) {
                restClient.performRequestAsync(
                        "PUT",
                        "/posts/doc/" + i,
                        Collections.<String, String>emptyMap(),
                                                documents[i],
                        new ResponseListener() {
                            @Override
                            public void onSuccess(Response response) {
                                                                latch.countDown();
                            }

                            @Override
                            public void onFailure(Exception exception) {
                                                                latch.countDown();
                            }
                        }
                );
            }
            latch.await();
                    }

    }

    @SuppressWarnings("unused")
    public void testCommonConfiguration() throws Exception {
        {
                        RestClientBuilder builder = RestClient.builder(new HttpHost("localhost", 9200))
                    .setRequestConfigCallback(new RestClientBuilder.RequestConfigCallback() {
                        @Override
                        public RequestConfig.Builder customizeRequestConfig(RequestConfig.Builder requestConfigBuilder) {
                            return requestConfigBuilder.setConnectTimeout(5000)
                                    .setSocketTimeout(60000);
                        }
                    })
                    .setMaxRetryTimeoutMillis(60000);
                    }
        {
                        RestClientBuilder builder = RestClient.builder(new HttpHost("localhost", 9200))
                    .setHttpClientConfigCallback(new RestClientBuilder.HttpClientConfigCallback() {
                        @Override
                        public HttpAsyncClientBuilder customizeHttpClient(HttpAsyncClientBuilder httpClientBuilder) {
                            return httpClientBuilder.setDefaultIOReactorConfig(
                                    IOReactorConfig.custom().setIoThreadCount(1).build());
                        }
                    });
                    }
        {
                        final CredentialsProvider credentialsProvider = new BasicCredentialsProvider();
            credentialsProvider.setCredentials(AuthScope.ANY,
                    new UsernamePasswordCredentials("user", "password"));

            RestClientBuilder builder = RestClient.builder(new HttpHost("localhost", 9200))
                    .setHttpClientConfigCallback(new RestClientBuilder.HttpClientConfigCallback() {
                        @Override
                        public HttpAsyncClientBuilder customizeHttpClient(HttpAsyncClientBuilder httpClientBuilder) {
                            return httpClientBuilder.setDefaultCredentialsProvider(credentialsProvider);
                        }
                    });
                    }
        {
                        final CredentialsProvider credentialsProvider = new BasicCredentialsProvider();
            credentialsProvider.setCredentials(AuthScope.ANY,
                    new UsernamePasswordCredentials("user", "password"));

            RestClientBuilder builder = RestClient.builder(new HttpHost("localhost", 9200))
                    .setHttpClientConfigCallback(new RestClientBuilder.HttpClientConfigCallback() {
                        @Override
                        public HttpAsyncClientBuilder customizeHttpClient(HttpAsyncClientBuilder httpClientBuilder) {
                            httpClientBuilder.disableAuthCaching();                             return httpClientBuilder.setDefaultCredentialsProvider(credentialsProvider);
                        }
                    });
                    }
        {
            Path keyStorePath = Paths.get("");
            String keyStorePass = "";
                        KeyStore truststore = KeyStore.getInstance("jks");
            try (InputStream is = Files.newInputStream(keyStorePath)) {
                truststore.load(is, keyStorePass.toCharArray());
            }
            SSLContextBuilder sslBuilder = SSLContexts.custom().loadTrustMaterial(truststore, null);
            final SSLContext sslContext = sslBuilder.build();
            RestClientBuilder builder = RestClient.builder(new HttpHost("localhost", 9200, "https"))
                    .setHttpClientConfigCallback(new RestClientBuilder.HttpClientConfigCallback() {
                        @Override
                        public HttpAsyncClientBuilder customizeHttpClient(HttpAsyncClientBuilder httpClientBuilder) {
                            return httpClientBuilder.setSSLContext(sslContext);
                        }
                    });
                    }
    }
}



package org.elasticsearch.test.rest.yaml;

import org.apache.http.HttpEntity;
import org.apache.http.HttpHost;
import org.elasticsearch.Version;
import org.elasticsearch.client.Response;
import org.elasticsearch.client.ResponseException;
import org.elasticsearch.client.RestClient;
import org.elasticsearch.test.rest.yaml.restspec.ClientYamlSuiteRestSpec;

import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;


public final class ClientYamlDocsTestClient extends ClientYamlTestClient {

    public ClientYamlDocsTestClient(ClientYamlSuiteRestSpec restSpec, RestClient restClient, List<HttpHost> hosts, Version esVersion)
            throws IOException {
        super(restSpec, restClient, hosts, esVersion);
    }

    public ClientYamlTestResponse callApi(String apiName, Map<String, String> params, HttpEntity entity, Map<String, String> headers)
            throws IOException {

        if ("raw".equals(apiName)) {
                        Map<String, String> queryStringParams = new HashMap<>(params);
            String method = Objects.requireNonNull(queryStringParams.remove("method"), "Method must be set to use raw request");
            String path = "/" + Objects.requireNonNull(queryStringParams.remove("path"), "Path must be set to use raw request");
                        try {
                Response response = restClient.performRequest(method, path, queryStringParams, entity);
                return new ClientYamlTestResponse(response);
            } catch (ResponseException e) {
                throw new ClientYamlTestResponseException(e);
            }
        }
        return super.callApi(apiName, params, entity, headers);
    }
}

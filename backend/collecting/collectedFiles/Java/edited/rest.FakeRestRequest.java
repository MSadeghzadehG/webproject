

package org.elasticsearch.test.rest;

import org.elasticsearch.common.bytes.BytesReference;
import org.elasticsearch.common.xcontent.NamedXContentRegistry;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.rest.RestRequest;

import java.net.SocketAddress;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class FakeRestRequest extends RestRequest {

    private final BytesReference content;
    private final Method method;
    private final SocketAddress remoteAddress;

    public FakeRestRequest() {
        this(NamedXContentRegistry.EMPTY, new HashMap<>(), new HashMap<>(), null, Method.GET, "/", null);
    }

    private FakeRestRequest(NamedXContentRegistry xContentRegistry, Map<String, List<String>> headers, Map<String, String> params,
                            BytesReference content, Method method, String path, SocketAddress remoteAddress) {
        super(xContentRegistry, params, path, headers);
        this.content = content;
        this.method = method;
        this.remoteAddress = remoteAddress;
    }

    @Override
    public Method method() {
        return method;
    }

    @Override
    public String uri() {
        return rawPath();
    }

    @Override
    public boolean hasContent() {
        return content != null;
    }

    @Override
    public BytesReference content() {
        return content;
    }

    @Override
    public SocketAddress getRemoteAddress() {
        return remoteAddress;
    }

    public static class Builder {
        private final NamedXContentRegistry xContentRegistry;

        private Map<String, List<String>> headers = new HashMap<>();

        private Map<String, String> params = new HashMap<>();

        private BytesReference content;

        private String path = "/";

        private Method method = Method.GET;

        private SocketAddress address = null;

        public Builder(NamedXContentRegistry xContentRegistry) {
            this.xContentRegistry = xContentRegistry;
        }

        public Builder withHeaders(Map<String, List<String>> headers) {
            this.headers = headers;
            return this;
        }

        public Builder withParams(Map<String, String> params) {
            this.params = params;
            return this;
        }

        public Builder withContent(BytesReference content, XContentType xContentType) {
            this.content = content;
            if (xContentType != null) {
                headers.put("Content-Type", Collections.singletonList(xContentType.mediaType()));
            }
            return this;
        }

        public Builder withPath(String path) {
            this.path = path;
            return this;
        }

        public Builder withMethod(Method method) {
            this.method = method;
            return this;
        }

        public Builder withRemoteAddress(SocketAddress address) {
            this.address = address;
            return this;
        }

        public FakeRestRequest build() {
            return new FakeRestRequest(xContentRegistry, headers, params, content, method, path, address);
        }

    }

}

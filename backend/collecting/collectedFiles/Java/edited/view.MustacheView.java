

package org.springframework.boot.web.reactive.result.view;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.Writer;
import java.nio.charset.Charset;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;

import com.samskivert.mustache.Mustache.Compiler;
import com.samskivert.mustache.Template;
import reactor.core.publisher.Flux;
import reactor.core.publisher.Mono;

import org.springframework.core.io.Resource;
import org.springframework.core.io.buffer.DataBuffer;
import org.springframework.core.io.buffer.DataBufferUtils;
import org.springframework.http.MediaType;
import org.springframework.web.reactive.result.view.AbstractUrlBasedView;
import org.springframework.web.reactive.result.view.View;
import org.springframework.web.server.ServerWebExchange;


public class MustacheView extends AbstractUrlBasedView {

	private Compiler compiler;

	private String charset;

	
	public void setCompiler(Compiler compiler) {
		this.compiler = compiler;
	}

	
	public void setCharset(String charset) {
		this.charset = charset;
	}

	@Override
	public boolean checkResourceExists(Locale locale) throws Exception {
		return resolveResource() != null;
	}

	@Override
	protected Mono<Void> renderInternal(Map<String, Object> model, MediaType contentType,
			ServerWebExchange exchange) {
		Resource resource = resolveResource();
		if (resource == null) {
			return Mono.error(new IllegalStateException(
					"Could not find Mustache template with URL [" + getUrl() + "]"));
		}
		DataBuffer dataBuffer = exchange.getResponse().bufferFactory().allocateBuffer();
		try (Reader reader = getReader(resource)) {
			Template template = this.compiler.compile(reader);
			Charset charset = getCharset(contentType).orElse(getDefaultCharset());
			try (Writer writer = new OutputStreamWriter(dataBuffer.asOutputStream(),
					charset)) {
				template.execute(model, writer);
				writer.flush();
			}
		}
		catch (Exception ex) {
			DataBufferUtils.release(dataBuffer);
			return Mono.error(ex);
		}
		return exchange.getResponse().writeWith(Flux.just(dataBuffer));
	}

	private Resource resolveResource() {
		Resource resource = getApplicationContext().getResource(getUrl());
		if (resource == null || !resource.exists()) {
			return null;
		}
		return resource;
	}

	private Reader getReader(Resource resource) throws IOException {
		if (this.charset != null) {
			return new InputStreamReader(resource.getInputStream(), this.charset);
		}
		return new InputStreamReader(resource.getInputStream());
	}

	private Optional<Charset> getCharset(MediaType mediaType) {
		return Optional.ofNullable(mediaType != null ? mediaType.getCharset() : null);
	}

}

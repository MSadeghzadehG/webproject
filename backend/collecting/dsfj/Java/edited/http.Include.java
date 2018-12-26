

package org.springframework.boot.actuate.trace.http;

import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.Set;


public enum Include {

	
	REQUEST_HEADERS,

	
	RESPONSE_HEADERS,

	
	COOKIE_HEADERS,

	
	AUTHORIZATION_HEADER,

	
	PRINCIPAL,

	
	REMOTE_ADDRESS,

	
	SESSION_ID,

	
	TIME_TAKEN;

	private static final Set<Include> DEFAULT_INCLUDES;

	static {
		Set<Include> defaultIncludes = new LinkedHashSet<>();
		defaultIncludes.add(Include.REQUEST_HEADERS);
		defaultIncludes.add(Include.RESPONSE_HEADERS);
		defaultIncludes.add(Include.COOKIE_HEADERS);
		defaultIncludes.add(Include.TIME_TAKEN);
		DEFAULT_INCLUDES = Collections.unmodifiableSet(defaultIncludes);
	}

	
	public static Set<Include> defaultIncludes() {
		return DEFAULT_INCLUDES;
	}

}

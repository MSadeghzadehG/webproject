

package org.springframework.boot.devtools.livereload;

import java.io.IOException;


class ConnectionClosedException extends IOException {

	ConnectionClosedException() {
		super("Connection closed");
	}

}

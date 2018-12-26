

package org.springframework.boot.test.autoconfigure.data.redis;

import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

import org.springframework.data.redis.connection.RedisConnection;
import org.springframework.data.redis.core.RedisOperations;
import org.springframework.stereotype.Service;


@Service
public class ExampleService {

	private static final Charset CHARSET = StandardCharsets.UTF_8;

	private RedisOperations<Object, Object> operations;

	public ExampleService(RedisOperations<Object, Object> operations) {
		this.operations = operations;
	}

	public boolean hasRecord(PersonHash personHash) {
		return this.operations.execute((RedisConnection connection) -> connection
				.exists(("persons:" + personHash.getId()).getBytes(CHARSET)));
	}

}

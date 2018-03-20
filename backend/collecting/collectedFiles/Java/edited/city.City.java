

package org.springframework.boot.autoconfigure.data.couchbase.city;

import com.couchbase.client.java.repository.annotation.Field;
import com.couchbase.client.java.repository.annotation.Id;

import org.springframework.data.couchbase.core.mapping.Document;

@Document
public class City {

	@Id
	private String id;

	@Field
	private String name;

}

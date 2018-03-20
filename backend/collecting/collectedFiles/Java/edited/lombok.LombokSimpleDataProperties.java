

package org.springframework.boot.configurationsample.lombok;

import java.util.ArrayList;
import java.util.List;

import lombok.Data;

import org.springframework.boot.configurationsample.ConfigurationProperties;


@Data
@ConfigurationProperties(prefix = "data")
@SuppressWarnings("unused")
public class LombokSimpleDataProperties {

	private final String id = "super-id";

	
	private String name;

	private String description;

	private Integer counter;

	@Deprecated
	private Integer number = 0;

	private final List<String> items = new ArrayList<>();

	private final String ignored = "foo";

}

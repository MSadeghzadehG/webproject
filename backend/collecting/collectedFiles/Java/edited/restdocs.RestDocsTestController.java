

package org.springframework.boot.test.autoconfigure.restdocs;

import java.util.HashMap;
import java.util.Map;

import org.springframework.hateoas.MediaTypes;
import org.springframework.hateoas.mvc.ControllerLinkBuilder;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class RestDocsTestController {

	@ResponseBody
	@RequestMapping(path = "/", produces = MediaTypes.HAL_JSON_VALUE)
	public Map<String, Object> index() {
		Map<String, Object> response = new HashMap<>();
		Map<String, String> links = new HashMap<>();
		links.put("self", ControllerLinkBuilder.linkTo(getClass()).toUri().toString());
		response.put("_links", links);
		return response;
	}

}

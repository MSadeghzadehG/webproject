
package com.alibaba.json.bvt.support.jaxrs.mock.service;

import java.util.List;

import javax.ws.rs.Consumes;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.Produces;
import javax.ws.rs.core.MediaType;

import com.alibaba.fastjson.JSONObject;
import com.alibaba.json.bvt.support.spring.mock.entity.FastJsonParentTestVO;
import com.alibaba.json.bvt.support.spring.mock.entity.FastJsonTestVO;


@Path("fastjson")
public interface FastJsonRestfulServiceTest {

	@POST
	@Path("/test1")
	@Produces(MediaType.APPLICATION_JSON)
	@Consumes(MediaType.APPLICATION_JSON)
	public JSONObject test1(FastJsonTestVO vo);
	
	@POST
	@Path("/test2")
	@Produces(MediaType.APPLICATION_JSON)
	@Consumes(MediaType.APPLICATION_JSON)
	public JSONObject test2(List<FastJsonParentTestVO> vos);
}

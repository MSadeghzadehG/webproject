

package org.elasticsearch.client.documentation;

import org.elasticsearch.client.Client;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.test.ESIntegTestCase;


public class IndicesDocumentationIT extends ESIntegTestCase {

    
     public void testPutMappingDocumentation() throws Exception {
        Client client = client();

                client.admin().indices().prepareCreate("twitter")                   .addMapping("\"tweet\": {\n" +                                      "  \"properties\": {\n" +
                        "    \"message\": {\n" +
                        "      \"type\": \"text\"\n" +
                        "    }\n" +
                        "  }\n" +
                        "}")
                .get();
        
                client.admin().indices().prepareDelete("twitter").get();
        client.admin().indices().prepareCreate("twitter").get();

                client.admin().indices().preparePutMapping("twitter")           .setType("user")                                                .setSource("{\n" +                                                      "  \"properties\": {\n" +
                "    \"name\": {\n" +
                "      \"type\": \"text\"\n" +
                "    }\n" +
                "  }\n" +
                "}", XContentType.JSON)
        .get();

                client.admin().indices().preparePutMapping("twitter")
        .setType("user")
        .setSource("{\n" +
                "    \"user\":{\n" +                                            "        \"properties\": {\n" +
                "            \"name\": {\n" +
                "                \"type\": \"text\"\n" +
                "            }\n" +
                "        }\n" +
                "    }\n" +
                "}", XContentType.JSON)
        .get();
        
                client.admin().indices().preparePutMapping("twitter")           .setType("user")                                                .setSource("{\n" +                                                      "  \"properties\": {\n" +
                "    \"user_name\": {\n" +
                "      \"type\": \"text\"\n" +
                "    }\n" +
                "  }\n" +
                "}", XContentType.JSON)
        .get();
            }

}

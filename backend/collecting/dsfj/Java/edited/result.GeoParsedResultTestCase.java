

package com.google.zxing.client.result;

import java.util.Locale;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.Result;
import org.junit.Assert;
import org.junit.Test;


public final class GeoParsedResultTestCase extends Assert {

  private static final double EPSILON = 1.0E-10;

  @Test
  public void testGeo() {
    doTest("geo:1,2", 1.0, 2.0, 0.0, null, "geo:1.0,2.0");
    doTest("geo:80.33,-32.3344,3.35", 80.33, -32.3344, 3.35, null, null);
    doTest("geo:-20.33,132.3344,0.01", -20.33, 132.3344, 0.01, null, null);
    doTest("geo:-20.33,132.3344,0.01?q=foobar", -20.33, 132.3344, 0.01, "q=foobar", null);
    doTest("GEO:-20.33,132.3344,0.01?q=foobar", -20.33, 132.3344, 0.01, "q=foobar", null);
  }

  private static void doTest(String contents,
                             double latitude,
                             double longitude,
                             double altitude,
                             String query,
                             String uri) {
    Result fakeResult = new Result(contents, null, null, BarcodeFormat.QR_CODE);
    ParsedResult result = ResultParser.parseResult(fakeResult);
    assertSame(ParsedResultType.GEO, result.getType());
    GeoParsedResult geoResult = (GeoParsedResult) result;
    assertEquals(latitude, geoResult.getLatitude(), EPSILON);
    assertEquals(longitude, geoResult.getLongitude(), EPSILON);
    assertEquals(altitude, geoResult.getAltitude(), EPSILON);
    assertEquals(query, geoResult.getQuery());
    assertEquals(uri == null ? contents.toLowerCase(Locale.ENGLISH) : uri, geoResult.getGeoURI());
  }

}

package hudson.util.jna;

import static com.sun.jna.Library.*;
import com.sun.jna.win32.W32APITypeMapper;
import com.sun.jna.win32.W32APIFunctionMapper;

import java.util.Map;
import java.util.HashMap;


public interface Options {
  Map<String, Object> UNICODE_OPTIONS = new HashMap<String, Object>() {
    {
      put(OPTION_TYPE_MAPPER, W32APITypeMapper.UNICODE);
      put(OPTION_FUNCTION_MAPPER, W32APIFunctionMapper.UNICODE);
    }
  };
}
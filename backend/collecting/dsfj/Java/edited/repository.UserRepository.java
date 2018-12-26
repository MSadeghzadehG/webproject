
package com.fernandocejas.android10.sample.domain.repository;

import com.fernandocejas.android10.sample.domain.User;
import io.reactivex.Observable;
import java.util.List;


public interface UserRepository {
  
  Observable<List<User>> users();

  
  Observable<User> user(final int userId);
}

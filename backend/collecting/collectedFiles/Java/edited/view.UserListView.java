
package com.fernandocejas.android10.sample.presentation.view;

import com.fernandocejas.android10.sample.presentation.model.UserModel;
import java.util.Collection;


public interface UserListView extends LoadDataView {
  
  void renderUserList(Collection<UserModel> userModelCollection);

  
  void viewUser(UserModel userModel);
}

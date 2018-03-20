

package sample.hateoas.domain;

import java.util.List;

public interface CustomerRepository {

	List<Customer> findAll();

	Customer findOne(Long id);

}

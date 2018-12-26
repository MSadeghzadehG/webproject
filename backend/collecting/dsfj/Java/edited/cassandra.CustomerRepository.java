

package sample.data.cassandra;

import java.util.List;

import org.springframework.data.cassandra.repository.Query;
import org.springframework.data.repository.CrudRepository;

public interface CustomerRepository extends CrudRepository<Customer, String> {

	@Query("Select * from customer where firstname=?0")
	public Customer findByFirstName(String firstName);

	@Query("Select * from customer where lastname=?0")
	public List<Customer> findByLastName(String lastName);

}

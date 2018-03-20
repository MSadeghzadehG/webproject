

package sample.hateoas.domain;

import java.util.ArrayList;
import java.util.List;

import org.springframework.stereotype.Repository;
import org.springframework.util.ObjectUtils;

@Repository
public class InMemoryCustomerRepository implements CustomerRepository {

	private final List<Customer> customers = new ArrayList<>();

	public InMemoryCustomerRepository() {
		this.customers.add(new Customer(1L, "Oliver", "Gierke"));
		this.customers.add(new Customer(2L, "Andy", "Wilkinson"));
		this.customers.add(new Customer(2L, "Dave", "Syer"));
	}

	@Override
	public List<Customer> findAll() {
		return this.customers;
	}

	@Override
	public Customer findOne(Long id) {
		for (Customer customer : this.customers) {
			if (ObjectUtils.nullSafeEquals(customer.getId(), id)) {
				return customer;
			}
		}
		return null;
	}

}

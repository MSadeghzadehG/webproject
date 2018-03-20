
package com.iluwatar.cqrs.queries;

import java.math.BigInteger;
import java.util.List;

import com.iluwatar.cqrs.dto.Author;
import com.iluwatar.cqrs.dto.Book;


public interface IQueryService {

  Author getAuthorByUsername(String username);

  Book getBook(String title);

  List<Book> getAuthorBooks(String username);

  BigInteger getAuthorBooksCount(String username);

  BigInteger getAuthorsCount();

}

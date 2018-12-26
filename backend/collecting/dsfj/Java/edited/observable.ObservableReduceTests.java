

package io.reactivex.observable;

import static org.junit.Assert.*;

import org.junit.Test;

import io.reactivex.*;
import io.reactivex.functions.BiFunction;
import io.reactivex.observable.ObservableCovarianceTest.*;

public class ObservableReduceTests {

    @Test
    public void reduceIntsObservable() {
        Observable<Integer> o = Observable.just(1, 2, 3);
        int value = o.reduce(new BiFunction<Integer, Integer, Integer>() {
            @Override
            public Integer apply(Integer t1, Integer t2) {
                return t1 + t2;
            }
        }).toObservable().blockingSingle();

        assertEquals(6, value);
    }

    @SuppressWarnings("unused")
    @Test
    public void reduceWithObjectsObservable() {
        Observable<Movie> horrorMovies = Observable.<Movie> just(new HorrorMovie());

        Observable<Movie> reduceResult = horrorMovies.scan(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        }).takeLast(1);

        Observable<Movie> reduceResult2 = horrorMovies.reduce(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        }).toObservable();

        assertNotNull(reduceResult2);
    }

    
    @Test
    public void reduceWithCovariantObjectsObservable() {
        Observable<Movie> horrorMovies = Observable.<Movie> just(new HorrorMovie());

        Observable<Movie> reduceResult2 = horrorMovies.reduce(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        }).toObservable();

        assertNotNull(reduceResult2);
    }


    @Test
    public void reduceInts() {
        Observable<Integer> o = Observable.just(1, 2, 3);
        int value = o.reduce(new BiFunction<Integer, Integer, Integer>() {
            @Override
            public Integer apply(Integer t1, Integer t2) {
                return t1 + t2;
            }
        }).blockingGet();

        assertEquals(6, value);
    }

    @SuppressWarnings("unused")
    @Test
    public void reduceWithObjects() {
        Observable<Movie> horrorMovies = Observable.<Movie> just(new HorrorMovie());

        Observable<Movie> reduceResult = horrorMovies.scan(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        }).takeLast(1);

        Maybe<Movie> reduceResult2 = horrorMovies.reduce(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        });

        assertNotNull(reduceResult2);
    }

    
    @Test
    public void reduceWithCovariantObjects() {
        Observable<Movie> horrorMovies = Observable.<Movie> just(new HorrorMovie());

        Maybe<Movie> reduceResult2 = horrorMovies.reduce(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        });

        assertNotNull(reduceResult2);
    }

    
    @Test
    public void reduceCovariance() {
                Observable<Movie> horrorMovies = Observable.<Movie> just(new HorrorMovie());
        libraryFunctionActingOnMovieObservables(horrorMovies);
    }

    
    public void libraryFunctionActingOnMovieObservables(Observable<Movie> obs) {

        obs.reduce(new BiFunction<Movie, Movie, Movie>() {
            @Override
            public Movie apply(Movie t1, Movie t2) {
                return t2;
            }
        });
    }

}

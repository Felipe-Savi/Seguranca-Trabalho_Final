#include <stdio.h>
#include <random>
#include <tuple>
#include <iostream>
#include <assert.h>
#include <iomanip>

#include <gmp.h>
#include <gmpxx.h>

#include "basic.h"
#include "gen_random_prime.h"

//mpz_class random_number(mpz_class lower, mpz_class higher) {
//    std::random_device dev;
//    std::mt19937 rng(dev());
//    std::uniform_int_distribution<mpz_class> dist(lower, higher);
//
//    return dist(rng);
//}

inline mpz_class mod(mpz_class const& a, mpz_class const& n) {
    mpz_class out;
    mpz_mod(out.get_mpz_t(), a.get_mpz_t(), n.get_mpz_t());
    return out;
}

inline mpz_class gcd(mpz_class const& a, mpz_class const& b) {
    mpz_class out;
    mpz_gcd(out.get_mpz_t(), a.get_mpz_t(), b.get_mpz_t());
    return out;
}

std::tuple<mpz_class, mpz_class, bool> pollard_rho(mpz_class n, int64_t* iteration_count = nullptr) {
    mpz_class i = 1;
    mpz_class k = 2;
    mpz_class x = rand_big_int(0, n-1);
    mpz_class y = x;
    while (true) {
        if (iteration_count) (*iteration_count)++;

        i += 1;
        
        mpz_class a = x * x - 1;
        x = mod(a, n);

        mpz_class y_minus_x = mod(y - x, n);

        mpz_class d = gcd(y_minus_x, n);

        enum Status { FAILED, IN_PROGRESS, SUCCESS, };

        Status status = IN_PROGRESS;
        if      (d == 1) status = IN_PROGRESS;
        else if (d == n) status = FAILED;
        else             status = SUCCESS;
        
        if (status == FAILED) {
            return {-1, -1, false};
        }

        if (status == SUCCESS) {
            mpz_class p = d;
            mpz_class q = n / p;
            return {p, q, true};
        }

        if (i == k) {
            y = x;
            k = 2 * k;
        }
    }
}

int main() {
    setbuf(stdout, NULL);

    std::vector<int> prime_bit_counts = {10, 20, 30, 40, 50, 60};

    uint64_t seed = 4;

    using std::cout;

    cout << std::setprecision(1) << std::fixed;

    for (int prime_bit_count : prime_bit_counts) {
        cout << "\n";
        cout << "######################\n";
        cout << "### Key size = " << std::setfill('0') << std::setw(3) << 2*prime_bit_count << " ###\n";
        cout << "######################\n";
        cout << "\n";

        std::vector<double> iteration_counts;
        std::vector<double> times;

        int test_count = 100;
        if (prime_bit_count * 2 >= 120) test_count = 10;

        for (int test_i = 1; test_i <= test_count; test_i++) {
            mpz_class n = gen_random_prime(prime_bit_count, &seed) * gen_random_prime(prime_bit_count, &seed);

            auto run_pollard_rho_until_success = [&]() -> std::tuple<mpz_class, mpz_class, int64_t> {
                while (true) {
                    int64_t iteration_count = 0;
                    auto [p, q, success] = pollard_rho(n, &iteration_count);

                    if (success) {
                        return {p, q, iteration_count};
                    }

                    cout << "Failed after such iterations: " << iteration_count << "\n";
                }
            };

            auto start = get_time();
                auto [p, q, iteration_count] = run_pollard_rho_until_success();
            auto end = get_time();

            double time_diff = compute_time_diff(start, end);

            iteration_counts.push_back(iteration_count);
            times.push_back(time_diff);

            cout << "Test " << test_i << ":\n";
            cout << n << " = " << p << " * " << q << "\n";
            cout << "Time: " << time_diff << "us" << "\n";
            cout << "Iterations: " << iteration_count << "\n";
            cout << "\n";
        }

        auto compute_mean_and_std_dev = [&](auto const& values) -> std::tuple<double, double>{
            double sum = 0.0;
            for (double value : values) {
                sum += value;
            }
            double mean = sum / values.size();

            double err_sq = 0.0;
            for (double value : values) {
                err_sq += (value - mean) * (value - mean);
            }
            double variance = err_sq / (values.size() - 1);

            double std_dev = sqrt(variance);

            return {mean, std_dev};
        };

        cout << "Summary:\n";

        cout << "Time: ";
        {
            auto [mean, std_dev] = compute_mean_and_std_dev(times);
            cout << "mean=" << mean << "us ; std_dev=" << std_dev << "us\n";

            //double sum = 0.0;
            //for (double time : times) {
            //    sum += time;
            //}
            //double mean = sum / times.size();

            //double err_sq = 0.0;
            //for (double time : times) {
            //    err_sq += (time - mean) * (time - mean);
            //}
            //double variance = err_sq / (times.size() - 1);

            //double std_dev = sqrt(variance);

            //double z = 1.96;
            //double margin_of_error = z * (std_dev / sqrt(times.size()));

            //cout << mean << " +- " << margin_of_error << "\n";
        }

        cout << "Iterations: ";
        {
            auto [mean, std_dev] = compute_mean_and_std_dev(iteration_counts);
            cout << "mean=" << mean << " ; std_dev=" << std_dev << "\n";

            //double sum = 0.0;
            //for (double iteration_count : iteration_counts) {
            //    sum += iteration_count;
            //}
            //double mean = sum / iteration_counts.size();

            //double err_sq = 0.0;
            //for (double iteration_count : iteration_counts) {
            //    err_sq += (iteration_count - mean) * (iteration_count - mean);
            //}
            //double variance = err_sq / (iteration_counts.size() - 1);

            //double std_dev = sqrt(variance);

            //double z = 1.96;
            //double margin_of_error = z * (std_dev / sqrt(iteration_counts.size()));

            //cout << mean << " +- " << margin_of_error << "\n";
        }

        cout << "\n";
    }
}


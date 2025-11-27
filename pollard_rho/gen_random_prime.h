gmp_randclass global_randclass(gmp_randinit_default);

mpz_class mod_pow(mpz_class a, mpz_class b, mpz_class n) {
    mpz_class a_pow2 = a;
    mpz_class res = 1;

    for (; b > 0; b /= 2) {
        if (b % 2 != 0) {
            res *= a_pow2;
            res %= n;
        }
        a_pow2 *= a_pow2;
        a_pow2 %= n;
    }

    return res;
}

using ui = unsigned long;

ui to_ui(mpz_class&& n) {
    return n.get_ui();
}

mpz_class rand_big_int(mpz_class const& min_v, mpz_class const& max_v) {
    assert(min_v <= max_v);

    mpz_class range = (max_v - min_v) + 1;
    mpz_class rand = global_randclass.get_z_range(range);
    return rand + min_v;
}

bool is_prime_miller_rabin(mpz_class const& n, int iterations_count) {
    if (n < 2)         return false;
    if (n == 2)        return true;
    if (n == 3)        return true;
    if (!to_ui(n % 2)) return false;

    mpz_class k;
    mpz_class m;
    {
        k = 0;
        m = n - 1;

        while (true) {
            mpz_class r = m % 2;

            if (r != 0) break;

            k++;
            m /= 2;
        }
    }

    for (int iter_i = 0; iter_i < iterations_count; iter_i++) {
        mpz_class a_min = 2;
        mpz_class a_max = n - 2;

        mpz_class a = rand_big_int(a_min, a_max);

        //auto start = get_time();
        mpz_class a_to_m = mod_pow(a, m, n);
        //log_time(start, "mod_pow");

        bool check1 = false;
        /* check1 */ {
            check1 = (a_to_m == 1);
        }

        bool check2 = false;
        /* check2 */ {
            mpz_class x = a_to_m;
            for (mpz_class i = 0; i < k; i++) {
                if (x == n - 1) {
                    check2 = true;
                    break;
                }
                x *= x;
                x %= n;
            }
        }

        bool is_composite = !check1 && !check2;

        if (is_composite)   return false;
    }

    return true;
}

inline uint64_t prng_blum_blum_shub(uint64_t m, uint64_t* x) {
    using u128 = __uint128_t;
    using u64  = uint64_t;

    u128 v1  = *x;
    u128 v2  = v1 * v1;
    u128 v3  = v2 % m;
    u128 res = v3;

    *x = (u64)res;

    return *x;
}

struct Blum_Blum_Shub_Config {
    uint64_t m;
};

std::vector<bool> generate_bits_using_blum_blum_shub(int bit_count, uint64_t* x, Blum_Blum_Shub_Config const& config) {
    std::vector<bool> bits(bit_count);
    uint64_t m = config.m;

    // Discard the first few bits.
    for (int i = 0; i < 10; i++) {
        prng_blum_blum_shub(m, x);
    }

    for (int i = 0; i < bit_count; i++) {
        prng_blum_blum_shub(m, x);
        bits[i] = __builtin_popcountl(*x) % 2;
    }

    return bits;
}

Blum_Blum_Shub_Config get_default_blum_blum_shub_config() {
    uint32_t p1 = 0;
    uint32_t p2 = 0;
    for (uint32_t x = 1'000'000'000; ; x++) {
        if (x >= 2'000'000'000) break;

        int miller_rabin_iterations = 15;

        /* check_conditions */ {
            bool check1 = (x % 4 == 3);
            if (!check1) continue;

            bool check2 = is_prime_miller_rabin(x, miller_rabin_iterations);
            if (!check2) continue;

            uint64_t x_in_u64 = (uint64_t)x;
            bool check3 = is_prime_miller_rabin(2 * x_in_u64 + 1, miller_rabin_iterations);
            if (!check3) continue;
        }

        if      (!p1) p1 = x;
        else if (!p2) p2 = x;
        
        if (p2) break;
    }

    assert(p2);

    uint64_t m = (uint64_t)p1 * (uint64_t)p2;

    return Blum_Blum_Shub_Config{m};
}

mpz_class gen_random_prime(int bit_count, uint64_t* seed) {
    Blum_Blum_Shub_Config config = get_default_blum_blum_shub_config();

    std::string s(bit_count, '0');

    mpz_class prime_value = {};

    int iterations_before_finding_prime = 0;

    using std::vector;

    vector<bool> last_bits = {};
    for (; ; iterations_before_finding_prime++) {
        vector<bool> bits = generate_bits_using_blum_blum_shub(bit_count, seed, config);

        for (int i = 0; i < (int)bits.size(); i++) {
            s[s.size()-1-i] = bits[i] ? '1' : '0';
        }

        mpz_class value(s, 2);

        {
            bool is_composite = false;
            int max_check = 100;
            if (value > max_check) {
                for (int x = 2; x <= max_check; x++) {
                    if (value % x == 0) {
                        is_composite = true;
                        break;
                    }
                }
            }
            if (is_composite) continue;
        }

        bool is_prime = is_prime_miller_rabin(value, 20);

        if (is_prime) {
            prime_value = value;
            break;
        }
    }

    return prime_value;
}

#include <iostream>
#include <fstream>
#include <cmath>
#include <omp.h>

#define uint64 unsigned long long
#define MAX 1'000'000'000ULL
#define THREAD_NUM 16  // Use only 2^n threads. If not, it doesn't work well.

using namespace std;

string filename = "./prime.txt";
uint64 count = 0, *cache_primes;
bool *sieve;
uint64 sieve_size = MAX/3 + 3;


bool is_prime(uint64 n)
{
    uint64 end = (uint64)sqrt(n);

    for (uint64 i = 0; cache_primes[i] <= end; i++)
    {
        if (n % cache_primes[i] == 0)
            return false;   
    }

    return true;
}


int main(int argc, char **argv)
{
    double start, end;

    sieve = new bool[sieve_size];

    // init sieve
    for (uint64 i = 0; i < sieve_size; i++)
    {
        sieve[i] = true;
    }

    // calculate some primes
    uint64 size = (uint64)(sqrt(MAX) / log(sqrt(MAX)) * 1.1) + 1'000ULL;
    cache_primes = new uint64[size];
    
    cache_primes[count++] = 2;    

    for (uint64 i = 3; i <= sqrt(MAX); i += 2)
    {
        if (is_prime(i))
        {
            cache_primes[count++] = i;
        }
    }

    // calcuate using eratosthenes' sieve
    cout << "Enter Calculation\n";
    start = omp_get_wtime();

    omp_set_num_threads(THREAD_NUM);
    
    #pragma omp parallel for schedule(dynamic)
    for (uint64 i = 2; i < count; i++)
    {
        uint64 p = cache_primes[i];

        for (uint64 j = 0;; j++)
        {
            if (p * (6 * j + 5) > MAX) break;

            //if (p * (6 * j + 5) % 6 == 1)  // 6k + 1 => 6(k - 1) + 7
                sieve[2 * (p * (6 * j + 5) / 6 - 1) + 1] = false;
            
            //if (p * (6 * j + 5) % 6 == 5)  // 6k + 5
                sieve[2 * (p * (6 * j + 5) / 6)] = false;

            if (p * (6 * j + 7) > MAX) break;

            //if (p * (6 * j + 7) % 6 == 1)  // 6k + 1 => 6(k - 1) + 7
                sieve[2 * (p * (6 * j + 7) / 6 - 1) + 1] = false;
            
            //if (p * (6 * j + 7) % 6 == 5)  // 6k + 5
                sieve[2 * (p * (6 * j + 7) / 6)] = false;
        }
    }

    end = omp_get_wtime();
    cout << "Finished Calculation: " << (end - start) * 1000 << "ms" << "\n\n";

    // save primes
    cout << "Enter Save\n";
    start = omp_get_wtime();

    ofstream file(filename.data());
    if (!file.is_open())
    {
        cout << "Error: Cannot write file." << "\n";
        return -1;
    }

    file << 2 << "\n";
    file << 3 << "\n";
    
    for (uint64 i = 0; 6 * i / 2 + 5 <= MAX; i += 2)
    {
        if (sieve[i])
            file << to_string(6 * (i / 2) + 5) << "\n";

        if (sieve[i + 1])
            file << to_string(6 * (i / 2) + 7) << "\n";
    }

    file.close();

    end = omp_get_wtime();
    cout << "Finished Save: " << (end - start) * 1000 << "ms" << "\n\n";

    return 0;
}

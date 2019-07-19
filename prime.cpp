#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <omp.h>

#define uint64 unsigned long long
#define MAX_NUM 1'000'000'000'000ULL
#define MAX_MEM       500'000'000ULL
#define THREAD_NUM 16

using namespace std;

string filename = "./save/prime.txt";
uint64 count = 0, *cache_primes;
bool **sieve;

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

void init(int type)
{
    for (uint64 j = 0; j < MAX_MEM / 2; j++)    
        sieve[type][j] = true;
}

void check(int type, uint64 p, uint64 now_idx)
{
    for (uint64 i = (now_idx + p - 1) / p / 2 * 2 + 1; p * i <= min(now_idx + MAX_MEM, MAX_NUM); i += 2)
    {
        sieve[type][(i * p - now_idx) / 2] = false;
    }
}

int main(int argc, char **argv)
{
    double start, end;

    sieve = new bool*[2];
    sieve[0] = new bool[MAX_MEM / 2];
    sieve[1] = new bool[MAX_MEM / 2];

    // calculate some primes
    cache_primes = new uint64[(uint64)(sqrt(MAX_NUM) / log(sqrt(MAX_NUM)) * 1.2) + 5'000ULL];
    
    cache_primes[count++] = 2;    

    for (uint64 i = 3; i <= sqrt(MAX_NUM); i += 2)
    {
        if (is_prime(i))
        {
            cache_primes[count++] = i;
        }
    }

    // calcuate using eratosthenes' sieve and save
    cout << "Enter Calculation & Save\n";
    start = omp_get_wtime();

    omp_set_num_threads(THREAD_NUM);
    
    ofstream file(filename, ios::out);
    if (!file.is_open())
    {
        cout << "Error: Cannot write file." << "\n";
        return -1;
    }

    for (uint64 i = 0; i < count; i++)
    {
        file << cache_primes[i] << "\n";
    }

    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < (MAX_NUM + MAX_MEM - 1) / MAX_MEM; i++)
            {
                int type = i % 2;
                uint64 now_idx = ((uint64)sqrt(MAX_NUM) + 1) / 2 * 2 + i * MAX_MEM;
                init(type);

                cout << "Calculation: [" << now_idx << ", " << min(now_idx + MAX_MEM, MAX_NUM) << "]\n";

                for (uint64 j = 1; j < count; j += 5)
                    #pragma omp task
                    for (uint64 k = 0; k < min(5ULL, count - j); k++)
                        check(type, cache_primes[j + k], now_idx);

                #pragma omp taskwait

                cout << "Calculation Clear!\n";

                #pragma omp task
                for (uint64 j = 0; j < min(MAX_MEM, (MAX_NUM - now_idx)) / 2; j++)
                    if (sieve[type][j])
                    {
                        file << 2 * j + 1 + now_idx;
                        file.put('\n');
                    }
            }
        }
    }

    file.close();

    end = omp_get_wtime();
    cout << "Finished Calculation & Save: " << (end - start) * 1000 << "ms" << "\n\n";

    return 0;
}

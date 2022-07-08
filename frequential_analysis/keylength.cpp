#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>
#include <set>

#define MIN_COUNT 10 // Replace this value
#define MAX_COUNT 10 // Replace this value

using namespace std;

typedef map<string, unsigned> SubstringMap;

/* Saves the given repeated substring with its distance.
 *
 * str - is the string to save.
 * offset - is the offset where string was located in its file.
 * strs - is a map associating repeated substrings with a distance between two
 * occurences.
 * distances - is a set of all distances between repeated substrings. 
 */
static void getDistance(const char *str, unsigned offset, SubstringMap &strs,
                 set<unsigned> &distances)
{
    string substring{str};
    auto result = strs.find(substring);
    if (result == strs.end())
    {
        strs.insert(pair<string, unsigned>(substring, offset));
    }
    else
    {
        unsigned last_offset = result->second;
        distances.insert(offset - last_offset);
    }
}

/* Finds all the repeated substrings in the file described by src of a size
 * between MIN_COUNT and MAX_COUNT. 
 *
 * src - is the file descriptor of the file to search in.
 * strs - is a map associating a repeated substring with its distance to the
 * next one.
 * distances - is a set of distances between encountered repeated substrings.  
 */
void findRepeatedSubstrings(int src, SubstringMap &strs, set<unsigned> &distances)
{
    char buffer[MAX_COUNT];
    off_t end_offset = lseek(src, 0, SEEK_END);
    off_t offset = lseek(src, 0, SEEK_SET);
    unsigned count = MIN_COUNT;
    while (offset + count <= end_offset)
    {
        while (offset + count <= end_offset && count <= MAX_COUNT)
        {
            read(src, buffer, count);
            buffer[count] = '\0';
            getDistance(buffer, offset, strs, distances);
            lseek(src, offset + 1, SEEK_SET);
            count++;
        }
        offset++;
        count = MIN_COUNT;
    }
}

/* Counts all the divisors of the given distance.
 *
 * distance - is the distance to find the divisors for.
 * divisors - is a map associating a divisors with its occurences. It contains
 * pairs for all the previously encountered divisors.
 */
void countDistanceDivisors(unsigned distance, map<unsigned, unsigned> &divisors)
{
    for (int divisor = 2; divisor <= distance; divisor++)
    {
        if (distance % divisor == 0)
        {
            divisors[divisor]++;
        }
    }
}

/* Finds the distances divisors frquencies.
 * 
 * divisors - is a map associating a divisor to its frequency.
 * distances - is a set of all encoutered distances between substring.
 */
static void findDistancesDivisorsFrequencies(map<unsigned, unsigned> &divisors,
                                             const set<unsigned> &distances)
{
    for (const auto &distance : distances)
    {
        countDistanceDivisors(distance, divisors);
    }
}

/* Finds the most frequent distance divisor.
 *
 * divisors - is a map associating a divisor to its frequency.
 */
static unsigned findMostFrequentDivisor(std::map<unsigned, unsigned> &divisors)
{
    unsigned mostFrequentDivisor{2};
    for (const auto &p : divisors)
    {
        if (divisors[mostFrequentDivisor] < p.second)
        {
            mostFrequentDivisor = p.first;
        }
    }
    return mostFrequentDivisor;
}

/* Finds the length of the key used to cipher a text with Vigènere cipher.
 *
 * src is the file to find the key length for.
 */
unsigned findKeyLength(int src)
{
    map<string, unsigned> substrings;
    map<unsigned, unsigned> divisors;
    set<unsigned> distances;
    findRepeatedSubstrings(src, substrings, distances);
    findDistancesDivisorsFrequencies(divisors, distances);
    unsigned key_length = findMostFrequentDivisor(divisors);
    return key_length;
}

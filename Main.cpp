#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

#include "cipher_tool/cipher.h"
#include "frequential_analysis/kasiski.h"
#include "frequential_analysis/keylength.h"

#define MIN_ARGC 4
#define MAX_ARGC 6

static int requireValidFileDescriptor(const char *path, int flags)
{
    int fd = open(path, flags);
    if (fd == -1)
    {
        perror(strerror(errno));
        exit(1);
    }
    return fd;
}

static void requireAKey(int argc)
{
    if (argc < MAX_ARGC)
    {
        std::cout << "No key given! If you don't know the key, you could try a";
        std::cout << " frequential analysis attack.\n";
        exit(2);
    }
}

// convierte los numeros dados a letras del alfabeto correspondiente.
static char toLetter(unsigned index)
{
    char letters[26] = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q',
        'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
    return letters[index];
}

// Imprime los vectores de la llave como strings.
static void printKey(std::vector<unsigned> keys)
{
    std::cout << "The key is \"";
    for (const auto &key : keys)
        std::cout << toLetter(key);
    std::cout << "\"" << std::endl;
}

/* argv contains (in the following order):
 * - argv[1]: 
 *  - "cipher" Para cifrar el contenido del archivo original con una llave.
 *  - "unipher" Para descifrar el contenido del archivo original con una llave.
 *  - "attack" Para descifrar el contenido sin una llave.
 * - argv[2] Contiene el directorio de donde estaran los resultados.
 * - argv[3] Contiene la llave para cuando sea necesaria,
 */
int main(int argc, const char *argv[])
{
    if (argc < MIN_ARGC)
    {
        std::cout << "usage: ./sec <cipher | uncipher | attack>";
        std::cout << " <-vigenere | -caesar> <source>";
        std::cout << " <destination> [key]\n";
        exit(3);
    }

    const char *action_type = argv[1];
    const char *cipher_type = argv[2];
    const char *source_path = argv[3];
    const char *destination_path = argv[4];

    int src = requireValidFileDescriptor(source_path, O_RDWR);
    int dest = open(destination_path, O_WRONLY | O_CREAT, 0666);

    if (strcmp("attack", action_type) == 0)
    {
        std::cout << "Attacking " << source_path << "...\n";
        if (strcmp("-vigenere", cipher_type) == 0)
        {
            std::vector<unsigned> keys = findKey(src, dest);
            printKey(keys);
            unsigned *k = &keys[0];
            uncipher(src, dest, k, keys.size());
        }
        else if (strcmp("-caesar", cipher_type) == 0)
        {
            caesarFrequentialAnalysisAttack(src, dest);
        }
        else
        {
            std::cout << "Unkown type of cipher! Here are your options:";
            std::cout << " -vigenere or -caesar.\n";
            exit(1);
        }
        std::cout << destination_path << " contains the unciphered text.\n";
    }
    else if (strcmp("uncipher", action_type) == 0 || strcmp("cipher", action_type) == 0)
    {
        requireAKey(argc);
        unsigned keys[strlen(argv[5])];
        keyToValues(argv[5], keys);
        if (strcmp("cipher", argv[1]) == 0)
        {
            cipher(src, dest, keys, strlen(argv[5]));
            std::cout << "Ciphered text of " << source_path << " is in " << destination_path << ".\n";
        }
        else if (strcmp("uncipher", argv[1]) == 0)
        {
            uncipher(src, dest, keys, strlen(argv[5]));
            std::cout << "Unciphered text of " << source_path << " is in " << destination_path << ".\n";
        }
    }
    else
    {
        std::cout << "Unkown type of action! Here are your options: cipher";
        std::cout << " uncipher or attack.";
        exit(1);
    }
    close(src);
    close(dest);
    exit(0);
}

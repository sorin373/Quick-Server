#include <iostream>
#include <fstream>
#include <cstring>

char v[10][101];

char a[10][101];
char b[10][101];

int main()
{
    std::ifstream file("src/server/config.txt");

    if (!file)
    {
        std::cerr << "failed to open file!\n";
        return EXIT_FAILURE;
    }

    unsigned int n = 0, m = 0, q = 0;

    while (file.getline(v[n], 101))
        n++;

    for (unsigned int i = 0; i < n; i++)
    {
        char *p = strtok(v[i], " ");

        while (p != NULL)
        {
            if (p[0] == '.')
            {
                strcpy(a[m], p);
                m++;
            }

            if (p[0] == '/')
            {
                strcpy(b[q], p);
                q++;
            }

            p = strtok(NULL, " ");
        }
    }

    for (unsigned int i = 0; i < q; i++)
        std::cout << b[i] << std::endl;

    

    return EXIT_SUCCESS;
}
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

const int NR_PROCESE = 10;
const int DIM_INTERVAL = 1000;

struct Interval {
    int inceput;
    int sfarsit;
};

bool estePrim(int n) {
    if (n < 2)
        return false;
    for (int d = 2; d * d <= n; d++)
        if (n % d == 0)
            return false;
    return true;
}

void ruleazaCopil(int citire, int scriere) {
    Interval interval;
    if (read(citire, &interval, sizeof(interval)) != sizeof(interval))
        _exit(1);
    close(citire);

    for (int n = interval.inceput; n <= interval.sfarsit; n++)
        if (estePrim(n))
            write(scriere, &n, sizeof(n));

    close(scriere);
    _exit(0);
}

int main() {
    pid_t copii[NR_PROCESE];
    int rezultate[NR_PROCESE];

    std::cout << "Parinte (PID " << getpid() << "): creez " << NR_PROCESE
              << " procese\n" << std::flush;

    for (int i = 0; i < NR_PROCESE; i++) {
        int catreCopil[2];
        int catreParinte[2];
        if (pipe(catreCopil) < 0 || pipe(catreParinte) < 0) {
            perror("pipe");
            return 1;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            close(catreCopil[1]);
            close(catreParinte[0]);
            for (int j = 0; j < i; j++)
                close(rezultate[j]);
            ruleazaCopil(catreCopil[0], catreParinte[1]);
        }

        close(catreCopil[0]);
        close(catreParinte[1]);

        Interval interval = {i * DIM_INTERVAL + 1, (i + 1) * DIM_INTERVAL};
        write(catreCopil[1], &interval, sizeof(interval));
        close(catreCopil[1]);

        copii[i] = pid;
        rezultate[i] = catreParinte[0];
    }

    int total = 0;
    for (int i = 0; i < NR_PROCESE; i++) {
        std::cout << "\nProcesul " << i + 1 << " (PID " << copii[i] << "), interval ["
                  << i * DIM_INTERVAL + 1 << ", " << (i + 1) * DIM_INTERVAL << "]:\n";

        int prim;
        while (read(rezultate[i], &prim, sizeof(prim)) == sizeof(prim)) {
            std::cout << prim << " ";
            total++;
        }
        std::cout << "\n";
        close(rezultate[i]);
    }

    for (int i = 0; i < NR_PROCESE; i++)
        waitpid(copii[i], nullptr, 0);

    std::cout << "\nTotal numere prime gasite: " << total << "\n";
    return 0;
}

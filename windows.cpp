#include <windows.h>
#include <iostream>
#include <string>

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

int ruleazaCopil() {
    HANDLE citire = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE scriere = GetStdHandle(STD_OUTPUT_HANDLE);

    Interval interval;
    DWORD octeti = 0;
    if (!ReadFile(citire, &interval, sizeof(interval), &octeti, NULL) || octeti != sizeof(interval))
        return 1;

    for (int n = interval.inceput; n <= interval.sfarsit; n++)
        if (estePrim(n))
            WriteFile(scriere, &n, sizeof(n), &octeti, NULL);

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "copil")
        return ruleazaCopil();

    char exe[MAX_PATH];
    GetModuleFileNameA(NULL, exe, MAX_PATH);

    HANDLE procese[NR_PROCESE];
    HANDLE threaduri[NR_PROCESE];
    HANDLE rezultate[NR_PROCESE];
    DWORD pid[NR_PROCESE];

    std::cout << "Parinte (PID " << GetCurrentProcessId() << "): creez " << NR_PROCESE
              << " procese\n" << std::flush;

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    for (int i = 0; i < NR_PROCESE; i++) {
        HANDLE copilCitire, parinteScriere;
        HANDLE parinteCitire, copilScriere;
        if (!CreatePipe(&copilCitire, &parinteScriere, &sa, 0) ||
            !CreatePipe(&parinteCitire, &copilScriere, &sa, 0)) {
            std::cerr << "CreatePipe a esuat: " << GetLastError() << "\n";
            return 1;
        }
        SetHandleInformation(parinteScriere, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(parinteCitire, HANDLE_FLAG_INHERIT, 0);

        std::string cmd = std::string("\"") + exe + "\" copil";

        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = copilCitire;
        si.hStdOutput = copilScriere;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        PROCESS_INFORMATION pi = {};

        if (!CreateProcessA(NULL, &cmd[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            std::cerr << "CreateProcess a esuat: " << GetLastError() << "\n";
            return 1;
        }

        CloseHandle(copilCitire);
        CloseHandle(copilScriere);

        Interval interval = {i * DIM_INTERVAL + 1, (i + 1) * DIM_INTERVAL};
        DWORD octeti = 0;
        WriteFile(parinteScriere, &interval, sizeof(interval), &octeti, NULL);
        CloseHandle(parinteScriere);

        procese[i] = pi.hProcess;
        threaduri[i] = pi.hThread;
        pid[i] = pi.dwProcessId;
        rezultate[i] = parinteCitire;
    }

    int total = 0;
    for (int i = 0; i < NR_PROCESE; i++) {
        std::cout << "\nProcesul " << i + 1 << " (PID " << pid[i] << "), interval ["
                  << i * DIM_INTERVAL + 1 << ", " << (i + 1) * DIM_INTERVAL << "]:\n";

        int prim;
        DWORD octeti = 0;
        while (ReadFile(rezultate[i], &prim, sizeof(prim), &octeti, NULL) && octeti == sizeof(prim)) {
            std::cout << prim << " ";
            total++;
        }
        std::cout << "\n";
        CloseHandle(rezultate[i]);
    }

    WaitForMultipleObjects(NR_PROCESE, procese, TRUE, INFINITE);

    for (int i = 0; i < NR_PROCESE; i++) {
        CloseHandle(procese[i]);
        CloseHandle(threaduri[i]);
    }

    std::cout << "\nTotal numere prime gasite: " << total << "\n";
    return 0;
}

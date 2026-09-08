extern "C" void launchKernel(void);

int main() {
    // board-level init (takt, GPIO) — za sada nista
    launchKernel();   // ne vraca se
    while (true) {}
}

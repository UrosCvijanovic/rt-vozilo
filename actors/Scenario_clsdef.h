// actors/Scenario_clsdef.h -- rucni dodatak klasi Actors::Scenario (model/sistem.room).
// Generisani Scenario.h ga ukljucuje UNUTAR tela klase, posle sekcije private.
// Rucni fajl: pri obnovi generisanog koda se ne brise (17.3).
public:
    // 1. Roditelj pravi deo sa CETIRI argumenta, npr. u Simulator_base.cpp:
    //        new (actors) Plovilo(this, host, plovilo_core, 0)
    //    a generisana klasa ima konstruktor sa tri. Cetvrti argument je redni
    //    broj dela u roditelju (0, 1, ...), potvrdjeno u generisanom kodu A3;
    //    ovde se ne koristi. Ako ga buduca verzija alata doda, kompajler javlja duplikat.
    Scenario (Actor* owner, coreid_t host, coreid_t target, int /*redniBrojDela*/)
        : Scenario(owner, host, target) {}

    // 2. Telemetrija taska aktera. DEFINE_PERIODIC_TASK pravi staticki TaskConfig cfg,
    //    koji createTask() (u build()) prosledjuje jezgru. Zato se poziva PRE
    //    root->build(). cfg je zajednicki za sve instance klase (17.4).
    static void setTaskTelemetry (Kernel::TaskTelemetry* t) { cfg.telemetry = t; }

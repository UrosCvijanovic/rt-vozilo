// actors/Proizvodjac_clsdef.h -- rucni dodatak klasi Actors::Proizvodjac.
// Generisani Proizvodjac.h ga ukljucuje UNUTAR tela klase.
public:
    // 1. Generisani Proba_base.cpp pravi deo sa CETIRI argumenta
    //        new (actors) Proizvodjac(this, host, proizvodjac_core, 0)
    //    a generisana klasa ima samo konstruktor sa tri. Dopuna: konstruktor sa
    //    cetvrtim argumentom (po svemu sudeci redni broj dela), koji ga zanemaruje.
    Proizvodjac (Actor* owner, coreid_t host, coreid_t target, int /*redniBrojDela*/)
        : Proizvodjac(owner, host, target) {}
 
    // 2. Telemetrija za task aktera. DEFINE_PERIODIC_TASK pravi staticki
    //    TaskConfig cfg, koji createTask() (u build()) prosledjuje jezgru, a
    //    jezgro iz cfg.telemetry uzima pokazivac na strukturu. Zato se poziva
    //    PRE root->build(). cfg je zajednicki za sve instance klase.
    static void setTaskTelemetry (Kernel::TaskTelemetry* t) { cfg.telemetry = t; }
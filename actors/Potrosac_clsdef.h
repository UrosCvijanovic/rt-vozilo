// actors/Potrosac_clsdef.h -- rucni dodatak klasi Actors::Potrosac.
// Generisani Potrosac.h ga ukljucuje UNUTAR tela klase.
public:
    // 1. Konstruktor sa cetiri argumenta -- isti razlog kao u Proizvodjac_clsdef.h.
    Potrosac (Actor* owner, coreid_t host, coreid_t target, int /*redniBrojDela*/)
        : Potrosac(owner, host, target) {}

    // 2. Telemetrija za task aktera -- isti mehanizam kao u Proizvodjac_clsdef.h
    //    (DEFINE_SPORADIC_TASK takodje pravi staticki cfg). Poziva se PRE build().
    static void setTaskTelemetry (Kernel::TaskTelemetry* t) { cfg.telemetry = t; }
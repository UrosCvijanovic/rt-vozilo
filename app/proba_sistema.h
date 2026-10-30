#pragma once
// app/proba_sistema.h -: hijerarhija, relejni portovi i poruke sa podacima
// (model/sistem.room; tabela-aktera.md, 2.1).
// Rezultati se citaju u debageru posle zaustavljanja u probeDone():
//     p mPlovilo   p mRegulacija   p mScenario   p mMisija
// Deljene globalne promenljive su dozvoljene SAMO u probi.
//
// Pokretanja se razlikuju SAMO u prioritetima (manji broj = visi, 15.2):
//   A: Plovilo pre Regulacije -> komanda koju Plovilo cita stara je do ~20 ms
//   B: Regulacija pre Plovila -> do ~10 ms (otvoreno pitanje 4: redosled u lancu)
#include <cstdint>

constexpr uint8_t PRIO_MISIJA = 2; // sporadicni, najvisi (D < T)
constexpr uint8_t PRIO_PLOVILO = 3;
constexpr uint8_t PRIO_REGULACIJA = 4;
constexpr uint8_t PRIO_SCENARIO = 5;
constexpr uint8_t PRIO_SIMGNSS = 6;
constexpr uint8_t PRIO_SIMIMU = 7;
constexpr uint8_t PRIO_FUZIJA = 8;
constexpr uint8_t PRIO_VODJENJE = 9;
constexpr uint8_t PRIO_TELEMETRIJA = 10;

// Scenario: aktivacija a, t = a x 100 ms
constexpr uint32_t A_RUTA_START = 1;    // ruta pa start, u istoj aktivaciji
constexpr uint32_t A_STOP = 5;
constexpr uint32_t A_KRAJ = 10;   // t = 1000 ms = hiperperioda probe: probeDone()

// Ruta probe: RUTA_N tacaka od 45,0 N, 20,0 E (jedinice 1e-7 stepena)
constexpr uint8_t RUTA_N = 5;
constexpr int32_t RUTA_LAT0 = 450000000;
constexpr int32_t RUTA_LON0 = 200000000;

void probeDone();

struct MerenjePlovila {
    uint32_t aktivacija; // ukupno aktivacija: slanje ne budi periodicni task (4.1)
    uint32_t prvaUs; // TIM2 u prvoj aktivaciji (faza, tacka 10)
    uint32_t drugaUs; // TIM2 u drugoj
    uint32_t bezKomande; // aktivacije pre prve komande: msg() == nullptr
    uint32_t nove; // komanda sa novim vremenom slanja
    uint32_t ponovljene; // ista komanda kao u prethodnoj aktivaciji (zadrzavanje)
    uint32_t neispravne; // tL + tR != 1: podatak ostecen na putu
    uint32_t maxStarostUs; // najstarija komanda koju je Plovilo procitalo
};

struct MerenjeRegulacije {
    uint32_t aktivacija;
    uint32_t prvaUs;         // faza, tacka 10
    uint32_t poslato;
    uint32_t maxSendUs;      // cena send() ka periodicnom primaocu, kroz 2 relejna porta (tacka 7)
};

struct MerenjeScenarija {
    uint32_t aktivacija;
    uint32_t prvaUs;         // faza, tacka 10
    uint32_t poslato;
};

struct MerenjeMisije {
    uint32_t aktivacija;       // pozivi behavior(); ostale aktivacije su prazne (zastavica, 15.8)
    uint32_t ruta, start, stop;
    uint32_t neispravnaRuta;   // n ili tacke ne odgovaraju poslatim
    uint32_t maxRUs;           // od slanja do obrade, po poruci (tacke 7 i 9)
    uint32_t maxPoAktivaciji;  // najvise poruka obradjenih u jednoj aktivaciji
};

extern volatile MerenjePlovila    mPlovilo;
extern volatile MerenjeRegulacije mRegulacija;
extern volatile MerenjeScenarija  mScenario;
extern volatile MerenjeMisije     mMisija;

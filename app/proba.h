#pragma once
#include <cstdint>
 
// Proba A1: komunikacija kroz ROOM alat (model/proba.room).
// Pokretanja se razlikuju SAMO u ovim konstantama (i ULAZ_CAP u actors/actorConfig.h):
//   A : potrosac viseg prioriteta, 1 poruka po poslu      -> R po poruci, 1 poruka = 1 aktivacija
//   B : proizvodjac viseg prioriteta, 3 (5) poruka/posao  -> zastavica dogadjaja, cap 1
//   C1: kao B, ULAZ_CAP = 4, bez praznjenja reda          -> zaglavljene poruke
//   C2: kao C1, sa praznjenjem reda u behavior()          -> ispravka
constexpr uint8_t PRIO_S = 3; // Potrosac (sporadicni)    A: 3  B: 3  C: 3
constexpr uint8_t PRIO_P = 5; // Proizvodjac (periodicni) A: 5  B: 2  C: 2
constexpr uint32_t PINGS_PER_JOB = 1; // A: 1  B: 3  C: 3
constexpr bool DRAIN = false; // praznjenje reda C1: false  C2: true
 
void probeDone();
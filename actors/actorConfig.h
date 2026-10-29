#pragma once
// actorConfig.h -- rucni fajl koji generisani kod ukljucuje (actors/*_base.h).
// Ovde idu konfiguracioni parametri aktera: visestrukosti portova, delova i
// atributa, kapaciteti bafera portova (simbolicke konstante iz [[cap = ...]]).
#include <cstddef>
#include "poruke.h"
 
// Generisani kod uvek pise "using namespace Protocol;", a taj prostor imena
// definisu generisani fajlovi protokola. Dok model nema nijedan protokol,
// deklaracija sprecava gresku; kad protokoli postoje, samo se spaja sa njima.
namespace Protocol {}
 
// Kapacitet reda porta Misija.operaterIn (dogadjaji).
// Privremeno: Scenario moze poslati rutu i start u istoj aktivaciji, pa je
// potrebno bar 2; konacna vrednost iz T_min u tabeli procesa.
constexpr std::size_t OPERATER_CAP = 4;
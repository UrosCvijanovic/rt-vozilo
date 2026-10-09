#pragma once
// actorConfig.h -- rucni fajl koji generisani kod ukljucuje (actors/*_base.h).
// Ovde idu konfiguracioni parametri aktera: visestrukosti portova, delova i
// atributa, kapaciteti bafera portova (simbolicke konstante iz [[cap = ...]]).
#include <cstddef>
 
// Generisani kod uvek pise "using namespace Protocol;", a taj prostor imena
// definisu generisani fajlovi protokola. Dok model nema nijedan protokol,
// deklaracija sprecava gresku; kad protokoli postoje, samo se spaja sa njima.
namespace Protocol {}
 
// Kapacitet reda porta Potrosac.ulaz (model/proba.room: [[cap = ULAZ_CAP]]).
// Pun red ne blokira posiljaoca: nova poruka prepisuje najstariju (FIFO).
constexpr std::size_t ULAZ_CAP = 4;
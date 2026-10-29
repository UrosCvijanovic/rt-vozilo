#pragma once
// poruke.h -- tipovi podataka koje nose signali protokola (model/sistem.room).
// Ukljucuje ga actors/actorConfig.h, PRE generisanih headera protokola (17.4).
// Pravila (tabela-aktera.md):
//  - prosti agregati: bez podrazumevanih vrednosti polja, konstruktora, pokazivaca
//    i kontejnera (podatak je clan union-a u poruci i kopira se u red primaoca);
//  - svaki signal nosi podatak, makar vreme slanja t (us, TIM2->CNT).
#include <cstdint>
#include <cstddef>
 

// Opseg komande pogona (normalizovano). Odluka o motorima je otvorena:
// POGON_MIN = 0 samo napred (tezi slucaj za regulaciju), -1 i unazad.
constexpr float POGON_MIN = 0.0f;
constexpr float POGON_MAX = 1.0f;
constexpr std::size_t RUTA_MAX = 16;

struct PodaciPogona { 
    float tL; 
    float tR; 
    uint32_t t; 
};

struct Vreme { 
    uint32_t t; 
};

struct Waypoint { 
    int32_t lat; 
    int32_t lon; 
};

struct PodaciRute { 
    Waypoint wp[RUTA_MAX]; 
    uint8_t n; 
    uint32_t t; 
};


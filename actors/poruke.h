#pragma once
// poruke.h -- tipovi podataka koje nose signali protokola (model/sistem.room).
// Ukljucuje ga actors/actorConfig.h, PRE generisanih headera protokola
// Pravila:
//  - prosti agregati: bez podrazumevanih vrednosti polja, konstruktora, pokazivaca i kontejnera (podatak je clan union-a u poruci i kopira se u red primaoca)
//  - svaki signal nosi podatak, makar vreme slanja t (us, TIM2->CNT)
#include <cstdint>
#include <cstddef>
 

// Opseg komande pogona (normalizovano). Odluka o motorima je otvorena:
// POGON_MIN = 0 samo napred (tezi slucaj za regulaciju), -1 i unazad.
constexpr float POGON_NULA = 0.0f;
constexpr float POGON_MIN = 0.0f;
constexpr float POGON_MAX = 1.0f;
constexpr std::size_t RUTA_MAX = 16;

enum class RezimRada : uint8_t {
    cekanje, // 0
    autonomno, // 1
    povratak, // 2
    stop, // 3
    bezbedno // 4
};

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
    uint32_t t;
    uint8_t n; 
};

struct PodaciPlovila {
    float x;
    float y;
    float psi;
    float u;
    float r;
    float ax;
    uint32_t t;
};

struct PodaciGnss {
    int32_t lat;
    int32_t lon;
    float sog;
    float cog;
    uint32_t t;
    uint32_t tMerenja;
    uint8_t ispravno;
};

struct PodaciImu
{
    float ugao;
    float r;
    float ax;
    uint32_t indeks;
    uint32_t t;
    uint32_t tMerenja;
};


struct PodaciProcene {
    float x;
    float y;
    float psi;
    float u;
    float r;
    int32_t lat0;
    int32_t lon0;
    uint32_t t;
    uint8_t ishodistePostavljeno; // 0 ili 1
};

struct PodaciReference
{
    float psiRef;
    float uRef;
    uint32_t t;
    uint8_t vazi; // 0 ili 1
};

struct PodaciNapretka
{
    uint32_t rutaT;
    uint32_t t;
    uint8_t indeks;
};

struct PodaciRezima {
    uint32_t t;
    RezimRada rezim;
    uint8_t pogonDozvoljen; // 0 ili 1
};

struct PodaciIzvestaja {
    int32_t lat;
    int32_t lon;
    float psi;
    float u;
    uint32_t t;
    uint32_t tProcene;
    RezimRada rezim;
    uint8_t pozicijaVazi;
};
#ifndef __PROXIMITY_SENSOR__
#define __PROXIMITY_SENSOR__

/*
Interfaccia di un sensore di prossimità (implementata da Sonar)
*/

class ProximitySensor
{
public:
    // Campiona il sensore
    virtual void sample() = 0;

    // Ritorna true se il sensore ha rilevato un ostacolo
    virtual bool isDetected() = 0;
};

#endif
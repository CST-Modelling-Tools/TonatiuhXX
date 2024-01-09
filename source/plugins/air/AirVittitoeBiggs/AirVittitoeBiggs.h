#pragma once

#include <Inventor/fields/SoSFEnum.h>

#include "kernel/air/AirAbstract.h"


class AirVittitoeBiggs: public AirAbstract
{
    SO_NODE_HEADER(AirVittitoeBiggs);

public:
    static void initClass();
    AirVittitoeBiggs();

    double transmission(double distance) const;

    enum Visibility {ClearDay, HazyDay};
    SoSFEnum visibility;

    NAME_ICON_FUNCTIONS("Vittitoe-Biggs", ":/AirVittitoeBiggs.png")
};



#include "kernel/air/AirFactory.h"

class AirVittitoeBiggsFactory:
    public QObject, public AirFactoryT<AirVittitoeBiggs>
{
    Q_OBJECT
    Q_INTERFACES(AirFactory)
    Q_PLUGIN_METADATA(IID "tonatiuh.AirFactory")
};

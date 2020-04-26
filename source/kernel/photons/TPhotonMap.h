#pragma once

#include "Photon.h"

class PhotonMapExport;

class TONATIUH_KERNEL TPhotonMap
{
public:
    TPhotonMap();
    ~TPhotonMap();

    void EndStore(double wPhoton);
    std::vector<Photon*> GetAllPhotons() const;
    PhotonMapExport* GetExportMode() const;
    void SetBufferSize(ulong nPhotons);
    void SetConcentratorToWorld(Transform concentratorToWorld);
    bool SetExportMode(PhotonMapExport* pExportPhotonMap);
    void StoreRays(std::vector<Photon>& ray);

private:
    ulong m_bufferSize;
    Transform m_concentratorToWorld;
    PhotonMapExport* m_pExportPhotonMap;
    const SceneModel* m_pSceneModel;
    unsigned long m_storedPhotonsInBuffer;
    unsigned long m_storedAllPhotons;
    std::vector<Photon*> m_photonsInMemory;
};

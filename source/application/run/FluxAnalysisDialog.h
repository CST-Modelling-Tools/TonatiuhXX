#pragma once

#include <QDialog>
#include <cstdint>

namespace Ui {
class FluxAnalysisDialog;
}

#include "kernel/photons/PhotonsBuffer.h"
#include "libraries/math/2D/Matrix2D.h"
#include "libraries/math/2D/vec2i.h"
#include "libraries/math/2D/Box2D.h"

class InstanceNode;
class SceneTreeModel;
class QIntValidator;
class TSceneKit;
class FluxAnalysis;
class QCPItemLine;

class FluxAnalysisDialog: public QDialog
{
    Q_OBJECT

public:
    FluxAnalysisDialog(TSceneKit* sceneKit, SceneTreeModel* sceneModel, int sunWidthDivisions, int sunHeightDivisions,
                       std::uint64_t masterSeed, QWidget* parent = 0);
    ~FluxAnalysisDialog();

private slots:
    void SurfaceSelected();
    void SurfaceChanged();
    void SideChanged();
    void UpdateAnalysis();
    void run();

    void UnitsChanged();
    void UpdateSectorPlotSlot();
    void on_pushButton_clicked();

private:
    void ClearAnalysis();

    void UpdateStatistics(double powerTotal, double fluxMin, double fluxAverage, double fluxMax,
                          double fluxMaxU, double fluxMaxV, double error, double uniformity, double gravityX, double gravityY);
    void UpdateFluxMapPlot(const Matrix2D<double>& photonCounts, const Box2D& box);
    void CreateSectorPlots(double xMin, double yMin, double xMax, double yMax);
    void UpdateSectorPlots(const Matrix2D<double>& flux, const Box2D& box);

    double findAreaCell(double uStep, double vStep);

private:
    Ui::FluxAnalysisDialog* ui;
    FluxAnalysis* m_fluxAnalysis;
    SceneTreeModel* m_sceneModel;
    QString m_fluxSurfaceURL;
    QString m_fluxLabel;
    QString m_path;
    QCPItemLine* m_lineV;
    QCPItemLine* m_lineH;
};

#pragma once

#include <QDialog>
class SceneTreeModel;
class PhotonsFactory;
struct PhotonsSettings;

namespace Ui {
class RayTracingDialog;
}


class RayTracingDialog: public QDialog
{
    Q_OBJECT

public:
    RayTracingDialog(QWidget* parent = 0);
    ~RayTracingDialog();

    void setParameters(
        int raysNumber, int raysScreen,
        int raysGridWidth = 200, int raysGridHeight = 200,
        int photonBufferSize = 1'000'000, bool photonBufferAppend = false);

    int raysNumber() const;
    int raysScreen() const;
    int raysGridWidth() const;
    int raysGridHeight() const;

    int photonBufferSize() const;
    bool photonBufferAppend() const;

    void setPhotonSettings(SceneTreeModel* scene, QVector<PhotonsFactory*> factories, PhotonsSettings* ps);
    PhotonsSettings getPhotonSettings() const;

private slots:
    void outputChanged();
    void surfaceAdd();
    void surfaceDelete();

private:
    Ui::RayTracingDialog* ui;
    SceneTreeModel* m_scene;
    QStringList m_surfaces;
};

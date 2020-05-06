#include <iostream>
#include <QVariant>

#include <Inventor/actions/SoBoxHighlightRenderAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>

#include "GraphicRoot.h"
#include "GraphicView.h"
#include "tree/SoPathVariant.h"

/**
 * Creates a new GraphicView with a \a parerent for the model data 3D representation.
 *
 * Use setModel() to set the model.
 */
GraphicView::GraphicView(QWidget* parent):
    QAbstractItemView(parent),
    m_sceneGraphRoot(0),
    m_viewer(0)
{

}

GraphicView::~GraphicView()
{
    delete m_viewer;
}

//#include <Inventor/nodes/SoPerspectiveCamera.h>
void GraphicView::SetSceneGraph(GraphicRoot* sceneGraphRoot)
{
    m_sceneGraphRoot = sceneGraphRoot;
    m_viewer = new SoQtExaminerViewer(this);

    SoBoxHighlightRenderAction* highlighter = new SoBoxHighlightRenderAction();
    highlighter->setColor(SbColor(100/255., 180/255., 120/255.));
    highlighter->setLineWidth(2.);
    m_viewer->setGLRenderAction(highlighter);

    m_viewer->setTransparencyType(SoGLRenderAction::SORTED_OBJECT_BLEND);
    m_viewer->setSceneGraph(m_sceneGraphRoot->GetNode() );

//    SoPerspectiveCamera* camera = dynamic_cast<SoPerspectiveCamera*>(m_myRenderArea->getCamera());
//    camera->scaleHeight(-1.); // left-handed to right-handed

    ViewCoordinateSystem(true);
    m_viewer->setHeadlight(false);
    m_viewer->setAntialiasing(true, 1); // disable if slow
}

SbViewportRegion GraphicView::GetViewportRegion() const
{
    return m_viewer->getViewportRegion();
}

SoCamera* GraphicView::GetCamera() const
{
    return m_viewer->getCamera();
}

QModelIndex GraphicView::indexAt(const QPoint& /*point*/) const
{
    return QModelIndex();
}

void GraphicView::scrollTo(const QModelIndex& /*index*/, ScrollHint /*hint*/)
{

}

QRect GraphicView::visualRect (const QModelIndex& /*index*/) const
{
    return QRect();
}

void GraphicView::ViewCoordinateSystem(bool view)
{
    m_viewer->setFeedbackVisibility(view);
}

void GraphicView::ViewDecoration(bool view)
{
    m_viewer->setDecoration(view);
}

void GraphicView::dataChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/)
{

}

void GraphicView::rowsInserted(const QModelIndex& /*parent*/, int /*start*/, int /*end*/)
{

}

void GraphicView::rowsAboutToBeRemoved(const QModelIndex& /*parent*/, int /*start*/, int /*end*/)
{

}

void GraphicView::setSelection(const QRect& /*rect*/, QItemSelectionModel::SelectionFlags /*flags*/)
{

}

int GraphicView::horizontalOffset() const
{
    return 0;
}

int GraphicView::verticalOffset() const
{
    return 0;
}

bool GraphicView::isIndexHidden(const QModelIndex& /*index*/) const
{
    return false;
}

QModelIndex GraphicView::moveCursor(CursorAction /*cursorAction*/, Qt::KeyboardModifiers /*modifiers*/)
{
    return QModelIndex();
}

QRegion GraphicView::visualRegionForSelection(const QItemSelection& /*selection*/) const
{
    return QRegion();
}

void GraphicView::currentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    if (m_sceneGraphRoot)
    {
        m_sceneGraphRoot->DeselectAll();

        SoFullPath* path;
        QVariant variant = current.data(Qt::UserRole);

        if (variant.canConvert<SoPathVariant>() )
        {
            path = static_cast< SoFullPath*>(variant.value< SoPathVariant >().GetPath() );
            m_sceneGraphRoot->Select(path);
        }

        //m_sceneGraphRoot->touch();
    }
}

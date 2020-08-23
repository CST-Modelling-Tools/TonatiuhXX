#include "SceneModel.h"

#include <QIcon>
#include <QMessageBox>

#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/nodekits/SoNodeKitListPart.h>
#include <Inventor/nodekits/SoSceneKit.h>
#include <Inventor/nodes/SoSelection.h>
#include <Inventor/nodes/SoMaterial.h>

#include "application/view/GraphicRoot.h"
#include "kernel/profiles/ProfileRT.h"
#include "kernel/material/MaterialRT.h"
#include "kernel/run/InstanceNode.h"
#include "kernel/scene/TSceneKit.h"
#include "kernel/scene/TSeparatorKit.h"
#include "kernel/scene/TShapeKit.h"
#include "kernel/scene/WorldKit.h"
#include "kernel/scene/LocationNode.h"
#include "kernel/scene/TerrainKit.h"
#include "kernel/shape/ShapeRT.h"
#include "kernel/air/AirKit.h"
#include "kernel/sun/SunKit.h"
#include "kernel/sun/SunKit.h"
#include "kernel/trackers/Tracker.h"
#include "libraries/math/gcf.h"
#include "tree/SoPathVariant.h"
#include "main/Document.h"

/*!
 * Creates an empty model.
 */
SceneModel::SceneModel(QObject* parent):
    QAbstractItemModel(parent),
    m_nodeRoot(0),
    m_nodeScene(0),
    m_instanceScene(0)
{

}

SceneModel::~SceneModel()
{
    clear();
}

/*!
 * Clears the model scene
 *
 * Clears nodes from the model.
 */
void SceneModel::clear()
{
    deleteInstanceTree(m_instanceScene);

    delete m_instanceScene;
    m_instanceScene = 0;
}

/*!
 * Sets the model scene to the given \a coinScene.
 *
 * Creates nodes for the model to the scene subnodes.
 */
void SceneModel::setDocument(Document* document)
{
    beginResetModel();

    m_nodeRoot = document->getSceneKit()->m_graphicRoot->getRoot();
    m_nodeScene = document->getSceneKit();
    m_mapCoinQt.clear();
    if (m_instanceScene) clear();

    m_instanceScene = new InstanceNode(m_nodeScene);
    m_mapCoinQt[m_nodeScene].append(m_instanceScene);

    SoNode* worldKit = m_nodeScene->getPart("world", true);
    InstanceNode* instanceW = addInstanceNode(m_instanceScene, worldKit);

    SoNode* location = m_nodeScene->getPart("world.location", true);
    addInstanceNode(instanceW, location);

    SoNode* sunKitW = m_nodeScene->getPart("world.sun", true);
    addInstanceNode(instanceW, sunKitW);

    SoNode* airKit = m_nodeScene->getPart("world.air", true);
    addInstanceNode(instanceW, airKit);

    SoNode* terrainKit = m_nodeScene->getPart("world.terrain", true);
    addInstanceNode(instanceW, terrainKit);

    TSeparatorKit* nodeLayout = (TSeparatorKit*) m_nodeScene->getLayout();
    m_instanceLayout = addInstanceNode(m_instanceScene, nodeLayout);
    generateInstanceTree(m_instanceLayout);

    endResetModel();
}

InstanceNode* SceneModel::addInstanceNode(InstanceNode* parent, SoNode* node)
{
    InstanceNode* instance = new InstanceNode(node);
    parent->addChild(instance);
    m_mapCoinQt[node].append(instance);
    return instance;
}

void SceneModel::generateInstanceTree(InstanceNode* instance)
{
    SoNode* node = instance->getNode();

    if (TShapeKit* shapeKit = dynamic_cast<TShapeKit*>(node))
    {
//        SoNode* shape = shapeKit->shapeRT.getValue();
//        if (shape)
//            addInstanceNode(instance, shape);

//        SoNode* aperture = shapeKit->profileRT.getValue();
//        if (aperture)
//            addInstanceNode(instance, aperture);

//        SoNode* materialRT = shapeKit->materialRT.getValue();
//        if (materialRT)
//            addInstanceNode(instance, materialRT);

//        SoNode* material = shapeKit->getPart("material", false);
//        if (material)
//            addInstanceNode(instance, material);
    }
    else if (TSeparatorKit* separatorKit = dynamic_cast<TSeparatorKit*>(node))
    {
        SoNode* tracker = separatorKit->getPart("tracker", false);
        if (tracker)
            addInstanceNode(instance, tracker);

        SoGroup* group = (SoGroup*) separatorKit->getPart("group", false);
        if (!group) return;
        for (int n = 0; n < group->getNumChildren(); ++n)
        {
            SoBaseKit* coinChild = (SoBaseKit*) group->getChild(n);
            InstanceNode* instanceChild = addInstanceNode(instance, coinChild);
            generateInstanceTree(instanceChild);
        }
    }
}

void SceneModel::deleteInstanceTree(InstanceNode* instance)
{
    while (instance->children.count() > 0)
    {
        InstanceNode* child = instance->children[instance->children.count() - 1];
        deleteInstanceTree(child);
        delete child;
    }

//    for (InstanceNode* child : instance.children)
//    {
//        DeleteInstanceTree(*child);
//        delete child;
//    }

    QList<InstanceNode*>& instanceList = m_mapCoinQt[instance->getNode()];
    int q = instanceList.indexOf(instance);
    instanceList.removeAt(q);

    InstanceNode* instanceParent = instance->getParent();
    if (instanceParent)
    {
        int row = instanceParent->children.indexOf(instance);
        instanceParent->children.remove(row);
    }
}

QModelIndex SceneModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!m_instanceScene) return QModelIndex();
    InstanceNode* instance = getInstance(parent);
    return createIndex(row, column, instance->children[row]);
}

int SceneModel::rowCount(const QModelIndex& index) const
{
    InstanceNode* instance = getInstance(index);
    return instance ? instance->children.count() : 0;
}

int SceneModel::columnCount(const QModelIndex& /*index*/) const
{
    return 1;
}

QModelIndex SceneModel::parent(const QModelIndex& index) const
{
    InstanceNode* instance = getInstance(index);
    if (!instance) return QModelIndex();

    InstanceNode* instanceParent = instance->getParent();
    if (!instanceParent) return QModelIndex();

    InstanceNode* instanceGrandparent = instanceParent->getParent();
    if (!instanceGrandparent) return QModelIndex();

    int row = instanceGrandparent->children.indexOf(instanceParent);
    return createIndex(row, 0, instanceParent);
}

QVariant SceneModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::UserRole && role != Qt::DecorationRole) return QVariant();

    InstanceNode* instance = getInstance(index);
    if (!instance) return QVariant();

    SoNode* node = instance->getNode();
    if (!node) return QVariant();

    if (index.column() != 0) return QVariant();

    if (role == Qt::DisplayRole)
    {
        QString name;
        if (node->getName() == SbName())
            name = node->getTypeId().getName().getString();
        else
            name = node->getName().getString();

        SoSearchAction action;
        action.setNode(node);
        action.setInterest(SoSearchAction::ALL);
        action.apply(m_nodeScene);

        int count = action.getPaths().getLength();
//        count = node->getRefCount();
        if (count > 1)
            name = QString("%1 [%2]").arg(name).arg(count);

        return name;
    }
    if (role == Qt::UserRole)
    {
        SoNodeKitPath* path = pathFromIndex(index);
        SoPathVariant pathWrapper(path);
        return QVariant::fromValue(pathWrapper);
    }
    if (role == Qt::DecorationRole)
    {
        SoType type = node->getTypeId();
        if (type.isDerivedFrom(TSeparatorKit::getClassTypeId()))
        {
            return QIcon(":/images/scene/nodeGroup.png");
        }
        else if (type.isDerivedFrom(TShapeKit::getClassTypeId()))
        {
            return QIcon(":/images/scene/nodeShape.png");
        }
        else if (type.isDerivedFrom(ShapeRT::getClassTypeId()))
        {
            ShapeRT* shape = static_cast<ShapeRT*>(node);
            return QIcon(shape->getTypeIcon());
        }
        else if (type.isDerivedFrom(ProfileRT::getClassTypeId()))
        {
            ProfileRT* aperture = static_cast<ProfileRT*>(node);
            return QIcon(aperture->getTypeIcon());
        }
        else if (type.isDerivedFrom(MaterialRT::getClassTypeId()))
        {
            MaterialRT* material = static_cast<MaterialRT*>(node);
            return QIcon(material->getTypeIcon());
        }
        else if (type.isDerivedFrom(SoMaterial::getClassTypeId()))
        {
            return QIcon(":/images/scene/nodeMaterialGL.png");
        }
        else if (type.isDerivedFrom(Tracker::getClassTypeId()))
        {
            Tracker* tracker = static_cast<Tracker*>(node);
            return QIcon(tracker->getTypeIcon());
        }
        else if (type == SunKit::getClassTypeId())
            return QIcon(":/images/scene/nodeSun.png");
        else if (type == WorldKit::getClassTypeId())
            return QIcon(":/images/scene/nodeFolder.png");
        else if (type == LocationNode::getClassTypeId())
            return QIcon(":/images/scene/nodeLocation.png");
        else if (type == AirKit::getClassTypeId())
            return QIcon(":/images/scene/nodeAir.png");
        else if (type == TerrainKit::getClassTypeId())
            return QIcon(":/images/scene/nodeTerrain.png");
    }

    return QVariant();
}

QVariant SceneModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (section == 0) return "Node";
        }
    }
    return QVariant();
}

Qt::ItemFlags SceneModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid()) return defaultFlags;
    if (index.column() > 0) return defaultFlags;
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | defaultFlags;
}

Qt::DropActions SceneModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions SceneModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

InstanceNode* SceneModel::getInstance(const QModelIndex& index) const
{
    if (!index.isValid()) return m_instanceScene;
    return (InstanceNode*) index.internalPointer();
}

bool SceneModel::setNodeName(SoNode* node, QString name)
{
    SbName sbname(name.toStdString().c_str());

    for (InstanceNode* instance : m_mapCoinQt[node])
    {
        for (InstanceNode* sibling : instance->getParent()->children)
        {
            if (sibling == instance) continue;
            if (sibling->getNode()->getName() == sbname)
                return false;
        }
    }

    node->setName(sbname);
    emit layoutChanged();
    return true;
}

/**
 * Returns the index of item with the \a nodeUrl.
 *
 * If \a nodeUrl is not a valid node url, the function returns root node index.
 *
 *
 * \sa IndexFromNodeUrl, NodeFromIndex, PathFromIndex.
**/
QModelIndex SceneModel::indexFromUrl(QString url) const
{
    QStringList path = url.split("/", QString::SkipEmptyParts);

    if (path.size() == 0) return QModelIndex();
    if (path.size() == 1 && path[0] == "Sun") return index(0, 0);

    QString nodeName = path.last();
    path.removeLast();
    QString parentURL = QString("//") + path.join("/");
    QModelIndex parentIndex = indexFromUrl(parentURL);
    InstanceNode* instanceParent = getInstance(parentIndex);

    int row = 0;
    for (InstanceNode* child : instanceParent->children)
    {
        if (child->getNode()->getName() == nodeName.toStdString().c_str())
            return index(row, 0, parentIndex);
        row++;
    }

    return QModelIndex();
}

/**
 * Returns the index of item with the \a coinNodePath.
 *
 * If \a coinNodePath is not a valid node path, the function returns root node index.
 *
 *
 * \sa IndexFromNodeUrl, NodeFromIndex, PathFromIndex.
**/
QModelIndex SceneModel::indexFromPath(const SoNodeKitPath& path) const
{
    SoBaseKit* coinNode = static_cast<SoBaseKit*>(path.getTail());
    if (!coinNode) gcf::SevereError("IndexFromPath Null coinNode.");
    if (coinNode->getTypeId().getName() == "SunKit") return index(0, 0);

    if (path.getLength() <= 1) return QModelIndex();
    SoBaseKit* coinParent = static_cast<SoBaseKit*>(path.getNodeFromTail(1));
    if (!coinParent) gcf::SevereError("IndexFromPath Null coinParent.");
    if (coinParent->getTypeId().getName() == "SunKit") return index(0, 0);
    if (coinParent->getTypeId().isDerivedFrom(SoSceneKit::getClassTypeId()))
    {
        int row = 0;
        for (InstanceNode* child : m_instanceScene->children)
        {
            if (child->getNode() == coinNode)
                return index(row, 0);
            row++;
        }
        return QModelIndex();
    }

    SoGroup* coinPartList = static_cast<SoGroup*>(coinParent->getPart("group", true));
    if (!coinPartList) gcf::SevereError("IndexFromPath Null coinPartList.");
    int row = coinPartList->findChild(coinNode);
    if (coinParent->getTypeId().isDerivedFrom(TSeparatorKit::getClassTypeId()))
        if (coinParent->getPart("tracker", false)) row++;

    SoNodeKitPath* pathParent = static_cast<SoNodeKitPath*>(path.copy());
    pathParent->ref();
    pathParent->truncate(pathParent->getLength() - 1);
    QModelIndex indexParent = indexFromPath(*pathParent);
    pathParent->unref();
    return index(row, 0, indexParent);
}

SoNodeKitPath* SceneModel::pathFromIndex(const QModelIndex& index) const
{
    // search for kits only
    SoNode* node = getInstance(index)->getNode();
    if (!node->getTypeId().isDerivedFrom(SoBaseKit::getClassTypeId()))
        return pathFromIndex(parent(index));

    // find first
    SoSearchAction action;
    action.setNode(node);
    action.setInterest(SoSearchAction::FIRST);
    action.apply(m_nodeRoot);

    SoNodeKitPath* path = (SoNodeKitPath*) action.getPath();
    if (!path) gcf::SevereError("PathFromIndex Null nodePath.");
    path->ref(); // otherwise destroyed with action

    //
//    SoNodeList nodeList;
//    InstanceNode* instance = getInstance(index);
//    while (instance->getParent())
//    {
//        nodeList.append(instance->getNode());
//        instance = instance->getParent();
//    }

//    for (int i = nodeList.getLength() - 1; i >= 0; --i)
//    {
//        SoBaseKit* coinNode = static_cast<SoBaseKit*>(nodeList[i]);
//        if (coinNode) path->append(coinNode);
//    }

    return path; // make unref later
}

/*!
 * Insert a light node to the model. If the model has an other light node, the previous node
 * will be deleted.
 */
void SceneModel::insertSunNode(SunKit* sunKit)
{
    m_nodeScene->setPart("world.sun", sunKit);

    InstanceNode* instance = new InstanceNode(sunKit);

    InstanceNode* instW = m_instanceScene->children[0];
    instW->children.remove(1);
    instW->insertChild(1, instance);

    emit layoutChanged();
}

int SceneModel::insertCoinNode(SoNode* node, SoBaseKit* parent)
{
    int row = -1;
    if (dynamic_cast<TSeparatorKit*>(parent))
    {
        if (dynamic_cast<Tracker*>(node))
        {
            parent->setPart("tracker", node);
            row = 0;
        }
        else // separatorKits and shapeKits
        {
            SoGroup* group = (SoGroup*) parent->getPart("group", true);

            row = group->getNumChildren();
            if (parent->getPart("tracker", false)) row++;

            if (SoBaseKit* kit = dynamic_cast<SoBaseKit*>(node))
            {
//                kit->setSearchingChildren(true);
                group->addChild(kit);
            }
        }
    }
    else // parent shapeKit
    {
        if (parent->getPart("shape", false)) row++; //?
        if (parent->getPart("material", false)) row++;
    }

    for (InstanceNode* instanceParent : m_mapCoinQt[parent])
    {
        InstanceNode* instance = new InstanceNode(node);
        instanceParent->insertChild(row, instance);
        m_mapCoinQt[node].append(instance);
        generateInstanceTree(instance);
    }

    emit layoutChanged();
    return row;
}

void SceneModel::removeCoinNode(int row, SoBaseKit* parent)
{
    if (parent->getTypeId().isDerivedFrom(TSeparatorKit::getClassTypeId()))
    {
        bool hasTracker = parent->getPart("tracker", false);
        if (row == 0 && hasTracker)
            parent->setPart("tracker", 0);
        else if (SoGroup* parts = (SoGroup*) parent->getPart("group", false))
            parts->removeChild(hasTracker ? row - 1 : row);
    }

    for (InstanceNode* instanceParent : m_mapCoinQt[parent])
    {
        InstanceNode* instance = instanceParent->children[row];
        instanceParent->children.remove(row);
        QList<InstanceNode*>& instances = m_mapCoinQt[instance->getNode()];
        instances.removeAt(instances.indexOf(instance));
    }

    emit layoutChanged();
}

void SceneModel::replaceCoinNode(TShapeKit* parent, SoNode* node)
{
    int row = 0;
    if (dynamic_cast<ShapeRT*>(node))
    {
        row = InstanceNode::IndexShapeRT;
        parent->shapeRT = node;
    }
    else if (dynamic_cast<ProfileRT*>(node))
    {
        row = InstanceNode::IndexProfileRT;
        parent->profileRT = node;
    }
    else if (dynamic_cast<MaterialRT*>(node))
    {
        row = InstanceNode::IndexMaterialRT;
        parent->materialRT = node;
    }

//    for (InstanceNode* instanceParent : m_mapCoinQt[parent])
//    {
//        // remove
//        InstanceNode* instance = instanceParent->children[row];
//        QList<InstanceNode*>& instances = m_mapCoinQt[instance->getNode()];
//        instances.removeAt(instances.indexOf(instance));

//        // insert
//        instance = new InstanceNode(node);
//        instanceParent->replaceChild(row, instance);
//        m_mapCoinQt[node].append(instance);
//    }

//    emit layoutChanged();
}

void SceneModel::replaceAir(AirKit* air)
{
    m_nodeScene->setPart("world.air", air);
    InstanceNode* instW = m_instanceScene->children[0];
    instW->children[2]->setNode(air);
    emit layoutChanged();
}

void SceneModel::replaceSun(SunKit* sun)
{
    m_nodeScene->setPart("world.sun", sun);
    InstanceNode* instW = m_instanceScene->children[0];
    instW->children[1]->setNode(sun);
    emit layoutChanged();
}

/**
 * Cuts the \a row child from \a coinParent node.
 *
 * Returns whether the cut is successfully done.
**/
bool SceneModel::Cut(SoBaseKit& parent, int row)
{
    if (row < 0) return false;

    QList<InstanceNode*> instancesParent = m_mapCoinQt[&parent];
    InstanceNode* instanceParent = instancesParent[0]; //?
    QModelIndex indexParent = createIndex(instanceParent->getParent()->children.indexOf(instanceParent), 0, instanceParent);
    beginRemoveRows(indexParent, row, row);

    SoNode* node = instanceParent->children[row]->getNode();
    if (!node->getTypeId().isDerivedFrom(SoBaseKit::getClassTypeId()))
    {
        SbString partName = parent.getPartString(node);
        if (partName.getLength() == 0) partName = "material"; //?
        parent.setPart(partName, 0);
    }
    else
    {
        if (parent.getTypeId().isDerivedFrom(TSeparatorKit::getClassTypeId()))
        {
            SoGroup* childList = (SoGroup*) parent.getPart("group", false);
            if (!childList) return false;

            int r = row;
            if (parent.getPart("tracker", false)) r--;
            node = childList->getChild(r);
            childList->removeChild(r);

            if (!node) gcf::SevereError("SceneModel::Cut Null coinChild pointer");
        }
    }

    QList<InstanceNode*> instances = m_mapCoinQt[node];

    // Test if the node is shared
    if (instances.size() == 1) //?
    {
        InstanceNode* instance = instances[0];
        deleteInstanceTree(instance);
        //delete instanceNode; //?
    }
    else
    {
        for (InstanceNode* instanceParent : m_mapCoinQt[&parent])
        {
            InstanceNode* instance = instanceParent->children[row];
            deleteInstanceTree(instance);
            //delete instanceNode;
        }
    }

    endRemoveRows();
    emit layoutChanged();

    return true;
}

/**
 * Adds a child to \a coinParent as \a row child. If \a type is tgc::Copied, the added child is a copy of \a coinNode. But the type is tgc::Shared the node is shared with previous parent.
 * Return true if the paste action is correctly done. Otherwise returns false.
**/
bool SceneModel::Paste(tgc::PasteType type, SoBaseKit& coinParent, SoNode& coinNode, int row)
{
    SoNode* child;
    SoNode* pCoinParent = &coinParent;

    if (type == tgc::Copied)
        child = static_cast<SoNode*>(coinNode.copy(true));
    else
        child = &coinNode;

    if (!child->getTypeId().isDerivedFrom(SoBaseKit::getClassTypeId()))
    { // material, tracker, shape
        if (child->getTypeId().isDerivedFrom(Tracker::getClassTypeId()))
        {
            TSeparatorKit* separatorKit = static_cast<TSeparatorKit*>(pCoinParent);
            Tracker* tracker = static_cast<Tracker*>(separatorKit->getPart("tracker", false));
            if (tracker)
            {
                QMessageBox::warning(0, "Tonatiuh warning", "This TSeparatorKit already contains a tracker");
                return false;
            }
            coinParent.setPart("tracker", child);
        }
        if (child->getTypeId().isDerivedFrom(ShapeRT::getClassTypeId()))
        {
            TShapeKit* shapeKit = static_cast<TShapeKit*>(pCoinParent);
            if (!shapeKit) return false;
            ShapeRT* shape = static_cast<ShapeRT*>(shapeKit->getPart("shape", false));
            if (shape)
            {
                QMessageBox::warning(0, "Tonatiuh warning", "This TShapeKit already contains a shape");
                return false;
            }
            coinParent.setPart("shape", child);
        }
        if (child->getTypeId().isDerivedFrom(MaterialRT::getClassTypeId()))
        {
            TShapeKit* shapeKit = static_cast<TShapeKit*>(pCoinParent);
            if (!shapeKit) return false;
            MaterialRT* material = static_cast<MaterialRT*>(shapeKit->getPart("material", false));
            if (material)
            {
                QMessageBox::warning(0, "Tonatiuh warning", "This TShapeKit already contains a material");
                return false;
            }
            coinParent.setPart("material", child);
        }
    }
    else
    {
        SoGroup* parts = static_cast<SoGroup*>(coinParent.getPart("group", true));
        if (!parts) gcf::SevereError( "SceneModel::Paste Null coinPartList pointer");
        if (coinParent.getPart("tracker", false))
            parts->insertChild(child, row - 1);
        else
            parts->insertChild(child, row);
    }

    for (InstanceNode* instanceParent : m_mapCoinQt[&coinParent])
    {
        InstanceNode* instance = new InstanceNode(child);
        instanceParent->insertChild(row, instance);
        m_mapCoinQt[child].append(instance);
        generateInstanceTree(instance);
    }

    emit layoutChanged();
    return true;
}

void SceneModel::UpdateSceneModel()
{
    emit layoutChanged();
}

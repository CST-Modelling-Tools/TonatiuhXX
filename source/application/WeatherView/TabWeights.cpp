#include "TabWeights.h"
#include "ui_TabWeights.h"

#include "TMY/ParameterModel.h"
#include "TMY/ParameterItem.h"
#include <QJSEngine>
#include <QFileDialog>
#include <QProcess>

TabWeights::TabWeights(QWidget* parent):
    QWidget(parent),
    ui(new Ui::TabWeights)
{
    ui->setupUi(this);
    m_sunSpatial = 0;

    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels({
        "Index",
        "Azimuth,\ndeg",
        "Elevation,\ndeg",
        "Weight,\nkWh/m2",
        "Weight,\n%"
     });
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    m_paramsModel = new ParameterModel(this);

    QStandardItem* itemRoot = m_paramsModel->invisibleRootItem();
    QStandardItem* item;

    item = m_paramsModel->appendParameter(itemRoot, new ParameterAbstract("Mesh"));
    m_paramsModel->appendParameter(item, new ParameterAbstract("Resolution", "1", true));
    m_paramsModel->appendParameter(item, new ParameterDouble("HasMiddle", 1, "", "{.f}"));

    item = m_paramsModel->appendParameter(itemRoot, new ParameterAbstract("Kernel"));
    m_paramsModel->appendParameter(item, new ParameterDouble("Order", 6, "", "{.f}"));

    item = m_paramsModel->appendParameter(itemRoot, new ParameterAbstract("Function"));
    m_paramsModel->appendParameter(item, new ParameterDouble("Preconditioner", 0, "", "{.f}"));

    item = m_paramsModel->appendParameter(itemRoot, new ParameterAbstract("Output"));
    m_paramsModel->appendParameter(item, new ParameterAbstract("Sampling", "sampling.csv", true));
    m_paramsModel->appendParameter(item, new ParameterAbstract("Script", "sampling.tnhpps", true));

    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeView->setModel(m_paramsModel);

    ui->treeView->expandAll();
    ui->treeView->header()->resizeSection(0, 150);
//    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->treeView, SIGNAL(pressed(QModelIndex)),
            ui->treeView, SLOT(edit(QModelIndex)));    
}

TabWeights::~TabWeights()
{
    delete ui;
    delete m_sunSpatial;
}

void TabWeights::setModelTMY(ModelTMY* model)
{
    m_modelData = model;
}

#include "SunPath/calculators/SunCalculatorMB.h"
#include "SunPath/samplers/SunSpatial.h"
#include "SunPath/samplers/SunTemporal.h"
#include "SunPath/data/FormatTMY.h"
#include "SunPath/data/FormatWSN.h"
using namespace sp;

#include "TMY/ModelTMY.h"
#include "TMY/ParameterItem.h"


void TabWeights::on_pushButton_Generate_pressed()
{
    ReaderTMY* format = m_modelData->formatTMY();
    ParameterModel* pm = format->paramEffective();
    double latitude = pm->parameterDouble("Location.Latitude")->value();
    double longitude = pm->parameterDouble("Location.Longitude")->value();
    int offsetUTC = pm->parameterDouble("Time.Zone")->value();
    int tStep = pm->parameterDouble("Time.Step")->value();
    double origin = pm->parameterDouble("Time.Origin")->value();
    int endTMY = origin*tStep + tStep;



    SunCalculatorMB sunCalculator;
    Location location("", latitude, longitude, offsetUTC);
    sunCalculator.setLocation(location);

    // temporal
    SunTemporal sunTemporal(sunCalculator);

    QVector<QDateTime> ts;
    ts << QDateTime();
    QVector<double> ds;

    for (RecordTMY& r : format->data()) {
        ts << r.time.addSecs(-offsetUTC + endTMY);
        ds << r.DNI;
    }
    ts[0] = ts[1].addSecs(-tStep);

    TimeSampler timeSampler(&sunTemporal);
    timeSampler.sample(ts);

    sunTemporal.setData(ds);


    // spatial
    delete m_sunSpatial;
    m_sunSpatial = new SunSpatial(sunCalculator);
    SunCalculator::setObliquity(23.4*degree);
    SkySampler skySampler(m_sunSpatial);

    int order = m_paramsModel->parameterDouble("Kernel.Order")->value();
    m_sunSpatial->setKernel(new SkyKernelPolyharmonic(order));

    QString rhot = m_paramsModel->parameterAbstract("Mesh.Resolution")->text();
//    QString expression_string("3 + Math.sqrt(5) + Math.pow(3,2) + Math.log(5)");
    QJSEngine engine;
    double rho = engine.evaluate(rhot).toNumber();
    bool hasMiddle = m_paramsModel->parameterDouble("Mesh.HasMiddle")->value() > 0;
    rho *= sunCalculator.obliquity();
    // set res here
    skySampler.sample(rho, 0., hasMiddle);



    m_sunSpatial->setWeights(sunTemporal);


//    FormatWSN formatWSN(&sunSpatial);
//    formatWSN.write("SevilleWeights.csv");

    // table
    QTableWidget* tw = ui->tableWidget;
//    tw->clear();
    tw->clearContents();

    const QVector<SkyNode>& skyNodes = m_sunSpatial->skyNodes();

    int rMax = skyNodes.size();
    tw->setRowCount(rMax);
    tw->setColumnCount(5);

//    tw->setHorizontalHeaderLabels({
//        "Index",
//        "Azimuth,\ndeg",
//        "Elevation,\ndeg",
//        "Weight,\nWh/m2",
//        "Weight,\n%"
//     });
//    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    const QVector<double>& weights = m_sunSpatial->weights();
    double wTotal = 0.;
    for (double w : weights) wTotal += w;

    for (int r = 0; r < rMax; r++) {
        const SkyNode& sn = skyNodes[r];
        Horizontal hc = sunCalculator.findHorizontal(sn.v);

        QTableWidgetItem* item;

        QString text = QString::number(r + 1);
        item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignRight);
        tw->setItem(r, 0, item);

        text = QString::number(hc.azimuth()/degree, 'f', 3);
        item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignRight);
        tw->setItem(r, 1, item);

        double alpha = hc.elevation()/degree;
        text = QString::number(alpha, 'f', 3);
        item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignRight);
        tw->setItem(r, 2, item);

        double w = weights[r];
        text = QString::number(w/1000., 'f', 3);
        item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignRight);
        tw->setItem(r, 3, item);

        text = QString::number(w/wTotal*100, 'f', 3);
        item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignRight);
        tw->setItem(r, 4, item);

        if (eqz(alpha))
            for (int c = 0; c < tw->columnCount(); c++)
                  tw->item(r, c)->setBackground(QColor("#F0F0F0"));

    }
}

void TabWeights::on_pushButton_Save_pressed()
{
    if (!m_sunSpatial) return;
    QString dirName = ui->lineEdit_dir->text();

    QString fileName = dirName + "/" + m_paramsModel->parameterAbstract("Output.Sampling")->text();
    QString scriptName = dirName + "/" + m_paramsModel->parameterAbstract("Output.Script")->text();
    try {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            throw QString("File not opened: ") + fileName;
        file.write(writeSampling().toLatin1());

        QFile fileScript(scriptName);
        if (!fileScript.open(QIODevice::WriteOnly | QIODevice::Text))
            throw QString("File not opened: ") + scriptName;

        fileScript.write(writeScript().toLatin1());
    }
    catch (const QString& message) {
        qDebug() << QString("Runtime error in TabWeights\n") + message;
    }
}

#include <QTextStream>
using Qt::endl;
QString TabWeights::writeSampling()
{
    QString ans;
    QTextStream fout(&ans);

    QString temp;
    int cw = 16;

    temp += QString("Index").rightJustified(cw) + ",";
    temp += QString("Azimuth [deg]").rightJustified(cw) + ",";
    temp += QString("Elevation [deg]").rightJustified(cw) + ",";
    temp += QString("Weight [kWh/m2]").rightJustified(cw) + ",";
    temp += QString("Weight [%]").rightJustified(cw);
    fout << temp << endl;

    SunCalculator* sun = m_sunSpatial->calculator();
    const QVector<SkyNode>& skyNodes = m_sunSpatial->skyNodes();
    const QVector<double>& values = m_sunSpatial->values();
    const QVector<double>& weights = m_sunSpatial->weights();
    const QVector<double>& amplitudes = m_sunSpatial->amplitudes();

    double wTotal = 0.;
    for (double w : weights) wTotal += w;

    for (int n = 0; n < skyNodes.size(); ++n) {
        QString temp;

        const SkyNode& sn = skyNodes[n];
        Horizontal hc = sun->findHorizontal(sn.v);
//        if (params.positiveAzimuth) hc.makeAzimuthPositive();

        temp += QString::number(n + 1).rightJustified(cw) + ",";
        temp += QString::number(hc.azimuth()/degree, 'f', 3).rightJustified(cw) + ",";
        temp += QString::number(hc.elevation()/degree, 'f', 3).rightJustified(cw) + ",";

        double w = weights[n];
        temp += QString::number(w/1000., 'f', 3).rightJustified(cw) + ",";
        temp += QString::number(w/wTotal*100, 'f', 3).rightJustified(cw);

        fout << temp << endl;
    }

    return ans;
}

QString TabWeights::writeScript()
{
    QString ans = R"(// Calculation of annual efficiency
var mirrorsArea = 2.*2.*500; // total area of mirrors
var receiverName = "//Node/Script/Tower/Receiver/Shape"; // receiver object
var rays = 1e5; // number of rays
var fileName = "sampling.csv"; // sampling points

var scene = tn.getScene();
var sp = scene.getPart("world.sun.position")
var irradiance = 1000.;
sp.setParameter("irradiance", irradiance);

var file = new DataObject;
file.read(fileName);
var sum = 0.;
var sumMax = 0.;
print("");
var line = "azimuth [deg], elevation[deg], efficiency";
print(line);

var nMax = file.rows();
for (var n = 1; n < nMax; n++) {
    var azimuth = parseFloat(file.array(n)[1]);
    var elevation = parseFloat(file.array(n)[2]);
    var weight = parseFloat(file.array(n)[3]);
    sp.setParameter("azimuth", azimuth);
    sp.setParameter("elevation", elevation);

    var f = tn.FindInterception(receiverName, rays);
    var fMax = mirrorsArea*irradiance;
    var eta = f/fMax;
    sum += weight*eta;
    sumMax += weight;
    var line = "" + azimuth + ", " + elevation + ", " + eta;
    print(line);
}

print("");
eta = sum/sumMax;
print("annual efficiency: " + eta);
)";
    return ans;
}

#include <QSettings>
void TabWeights::on_pushButton_selectDir_pressed()
{
    QSettings settings("Tonatiuh", "Cyprus");
    QString dirName = settings.value("dirProjects", "").toString();

    dirName = QFileDialog::getExistingDirectory(
        this, "Open Directory", dirName
    );
    if (dirName.isEmpty()) return;
    dirName = QDir::current().absoluteFilePath(dirName);
    ui->lineEdit_dir->setText(dirName);
}

void TabWeights::on_pushButton_openDir_pressed()
{
    QString dir = ui->lineEdit_dir->text();
//    dir = QDir::current().absoluteFilePath(dir);
//    dir = QDir::toNativeSeparators(dir);
    QString cmd = QString("explorer \"%1\"").arg(dir);
//    qDebug() << cmd;
    QProcess::startDetached(cmd);
}


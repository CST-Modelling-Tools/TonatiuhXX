#include "TabData.h"
#include "ui_TabData.h"

#include "TMY/ReaderTMY.h"
#include "TMY/ModelTMY.h"
#include "plot/Plotter.h"

TabData::TabData(QWidget* parent):
    QWidget(parent),
    ui(new Ui::TabData)
{
    ui->setupUi(this);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 1);
    ui->splitter->setSizes({200, 200});

    m_plotter = new Plotter(ui->customPlot);

    ui->horizontalScrollBar->setRange(0, 0); // xA, xB
    ui->horizontalScrollBar->setPageStep(1000); // xD, xMin = xA, xMax = xB + xD
    ui->verticalScrollBar->setRange(0, 0);
    ui->verticalScrollBar->setPageStep(1000);

    connect(ui->yearFilter, SIGNAL(filterChanged()), this, SLOT(filterChanged()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheelCP(QWheelEvent*)));
    connect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoveCP(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePressCP(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseReleaseCP(QMouseEvent*)));
    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
//    connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(vertScrollBarChanged(int)));
m_moveStart = false;

}

TabData::~TabData()
{
    delete m_plotter;
    delete ui;
}

void TabData::setModelTMY(ModelTMY* model)
{
    m_modelData = model;
    ui->tableView_Data->setModel(m_modelData);
    ui->tableView_Data->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    ui->tableView_Data->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
//    ui->tableView_Data->horizontalHeader()->resizeSection(0, 150);
//    ui->tableView_Data->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

#include "TMY/ParameterItem.h"
void TabData::onOpened()
{
    sp::ReaderTMY* format = m_modelData->formatTMY();
    ui->comboBox_DNI->clear();
    ui->comboBox_DNI->addItems(format->recordsInfo().names);
    ui->comboBox_DNI->setCurrentIndex(format->recordsInfo().names.indexOf("DNI"));
    bool leap = m_modelData->formatTMY()->paramEffective()->parameterDouble("Checks.Time in February")->value() > 28*24*3600;
    ui->yearFilter->setLeapYear(leap);
    filterChanged();
}

void TabData::on_comboBox_DNI_textActivated(const QString& arg)
{
    m_modelData->formatTMY()->setDNI(arg);
    filterChanged();
}

void TabData::filterChanged()
{
    m_modelData->setDates(ui->yearFilter->dateA(), ui->yearFilter->dateB());
    m_plotter->makePlot(m_modelData);
}

void TabData::on_pushButton_Reset_clicked()
{
    m_plotter->resetPlot();
}

#include "SunPath/math/math.h"

void TabData::mouseWheelCP(QWheelEvent* event)
{
    Qt::Orientations orientation;
    if (event->modifiers() & Qt::ShiftModifier) {
        orientation = Qt::Vertical;
        ui->customPlot->axisRect()->setRangeZoom(orientation);
    } else if (event->modifiers() & Qt::ControlModifier) {
        int mw = event->angleDelta().y();

        double mx = event->position().x();
        mx = ui->customPlot->xAxis->pixelToCoord(mx);

        double tMin = ui->customPlot->xAxis->range().lower; // visible
        double tMax = ui->customPlot->xAxis->range().upper;
        double TMin = m_plotter->tMin(); // filtered
        double TMax = m_plotter->tMax();

        double u = (mx - tMin)/(tMax - tMin);
        double tSize = tMax - tMin;

        double tStep = m_plotter->tStep();
        double zoom = 1. + mw/1000.;
        if (tSize < 10*tStep && zoom < 1.) return;
        tSize *= zoom;
        tMin = mx -  u*tSize;
        tMax = tMin + tSize;

        tMin = sp::clamp(tMin, TMin, TMax);
        tMax = sp::clamp(tMax, TMin, TMax);
        setRange(tMin, tMax);
    } else {
        ui->customPlot->axisRect()->setRangeZoom(orientation);
        ui->horizontalScrollBar->setValue(ui->horizontalScrollBar->value() + event->angleDelta().y());
    }

}

void TabData::mouseMoveCP(QMouseEvent* event)
{
//    Qt::Orientations orientation;
//    if (event->modifiers() & Qt::ShiftModifier) {
//        orientation = Qt::Vertical;
//        ui->customPlot->axisRect()->setRangeDrag(orientation);
//    } else if (event->modifiers() & Qt::ControlModifier) {
//        orientation = Qt::Horizontal ;
//        ui->customPlot->axisRect()->setRangeDrag(orientation);
//    } else {
//        ui->customPlot->axisRect()->setRangeDrag(orientation);
//    }

    if (m_moveStart) {
        double tMin = ui->customPlot->xAxis->range().lower; // visible
        double tMax = ui->customPlot->xAxis->range().upper;
        double tSize = tMax - tMin;
        double mx = event->position().x();
        mx = ui->customPlot->xAxis->pixelToCoord(mx);
        double u = (mx - tMin)/tSize;

        tMin = m_xpressed - u*tSize;
        if (tMin < m_plotter->tMin()) return;
        tMax = tMin + tSize;
        if (tMax > m_plotter->tMax()) return;
        setRange(tMin, tMax);

    }
}

void TabData::mousePressCP(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ControlModifier) ) {
        m_moveStart = true;
        double mx = event->position().x();
        mx = ui->customPlot->xAxis->pixelToCoord(mx);
        m_xpressed = mx;
    }
}

void TabData::mouseReleaseCP(QMouseEvent*)
{
    m_moveStart = false;
}

void TabData::horzScrollBarChanged(int value)
{
    QCustomPlot* plot = ui->customPlot;
    double tMin = plot->xAxis->range().lower; // visible
    double dt = plot->xAxis->range().size();
    double TMin = m_plotter->tMin(); // filtered
    double s = 1000./dt;
    int xV = s*(tMin - TMin);
    if (abs(xV - value) < 10) return;

    tMin = value/s + TMin;
    plot->xAxis->setRange(tMin, dt, Qt::AlignLeft);
    plot->replot();
}

void TabData::vertScrollBarChanged(int value)
{
    QCustomPlot* plot = ui->customPlot;
    double y0 = plot->yAxis->range().center();
    double yr = plot->yAxis->range().size();
    double v = value/100.*yr;
    if (qAbs(y0 + v) < 0.01*yr) return;

    plot->yAxis->setRange(-v, yr, Qt::AlignCenter);
    plot->replot();
}

void TabData::on_checkBox_stateChanged(int /*arg1*/)
{
    m_plotter->addSun(ui->checkBox->isChecked());
}

void TabData::setRange(double tMin, double tMax)
{
    double TMin = m_plotter->tMin(); // filtered
    double TMax = m_plotter->tMax();

    ui->customPlot->xAxis->setRange(tMin, tMax);
    ui->customPlot->replot();

    double s = 1000./(tMax - tMin);
    int xB = s*(TMax - TMin) - 1000;
    int xV = s*(tMin - TMin);
    ui->horizontalScrollBar->setRange(0, xB);
    ui->horizontalScrollBar->setValue(xV);
}


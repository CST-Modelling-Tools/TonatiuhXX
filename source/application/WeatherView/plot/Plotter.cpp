#include "Plotter.h"

#include "qcustomplot/qcustomplot.h"
#include "../TMY/ParameterItem.h"

Plotter::Plotter(QCustomPlot* cp)
{
    m_customPlot = cp;
    m_customPlot->axisRect()->setupFullAxesBox(true);
    m_customPlot->axisRect()->setAutoMargins(QCP::msNone);
m_customPlot->axisRect()->setMargins(QMargins(50,30,30,50));

    m_customPlot->setInteractions(QCP::iMultiSelect); // QCP::iRangeDrag
    // zoom rect https://www.qcustomplot.com/index.php/support/forum/1616
//    m_customPlot->setSelectionRectMode(QCP::SelectionRectMode::srmSelect);

    // set some pens, brushes and backgrounds:
    QPen penBlack(Qt::black, 1);
    QPen penGray(QBrush("#10000000"), 1, Qt::SolidLine);
    QPen penGraySub(QBrush("#0A000000"), 1, Qt::SolidLine);

    QCPAxis* xAxis = m_customPlot->xAxis;
    QCPAxis* xAxis2 = m_customPlot->xAxis2;
    QCPAxis* yAxis = m_customPlot->yAxis;
    QCPAxis* yAxis2 = m_customPlot->yAxis2;

    for (QCPAxis* axis : {xAxis, xAxis2, yAxis, yAxis2}) {
        axis->setBasePen(penBlack);
        axis->setTickPen(penBlack);
        axis->setSubTickPen(penBlack);
        axis->setTickLabelColor(Qt::black);
    }

    for (QCPAxis* axis : {xAxis, yAxis}) {
        QCPGrid* grid = axis->grid();
        grid->setPen(penGray);
        grid->setSubGridPen(penGraySub);
        grid->setSubGridVisible(!true);
        grid->setZeroLinePen(Qt::NoPen);
    }

//    customPlot->setBackground(QBrush("#FFFFFF"));
//    customPlot->axisRect()->setBackground(QBrush("#FFFFFF"));

//    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
//    m_customPlot->xAxis->setTicker(timeTicker);
//    timeTicker->setTimeFormat("%h:%m\n%dd");

    QSharedPointer<QCPAxisTickerDateTime> timeTicker(new QCPAxisTickerDateTime);
    m_customPlot->xAxis->setTicker(timeTicker);
    timeTicker->setDateTimeFormat("h:mm\nMMM d");

    timeTicker->setDateTimeSpec(Qt::UTC);
    // ge(QCPAxisTickerDateTime::dateTimeToKey(QDate(2013, 11, 16)), QCPAxisTickerDateTime::dateTimeToKey(QDate(2015, 5, 2)));
    //dateTimeTicker->setDateTimeFormat("d. MMM\nyyyy")
    xAxis->setRange(0, 24*3600);
    yAxis->setRange(0, 1);
}

void Plotter::makePlot(ModelTMY* md)
{
    m_model = md;
    int a = m_model->filter().indexA;
    int b = m_model->filter().indexB;
    sp::ReaderTMY* format = m_model->formatTMY();

    ParameterModel* pm = format->paramEffective();
    m_tStep = pm->parameterDouble("Time.Step")->value();
    m_tOrigin = pm->parameterDouble("Time.Origin")->value()*m_tStep;
    double middle = m_tOrigin + m_tStep/2;

    bool leap = pm->parameterDouble("Checks.Time in February")->value() > 28*24*3600;
    m_x0 = 0.;
    if (leap) m_x0 = QDateTime(QDate(1972, 1, 1), QTime(0,0), Qt::UTC).toSecsSinceEpoch();

    QVector<double> xs;
    QVector<double> ys;
    int nMax = format->data().size();
    for (int n = a; n < b; n++) {
        int nf = n;
        if (nf >= nMax) nf -= nMax;
        xs << m_x0 + format->data()[nf].stamp + middle;
        ys << format->data()[nf].DNI;
    }

    m_customPlot->clearPlottables();
    QCPBars* bars = new QCPBars(m_customPlot->xAxis, m_customPlot->yAxis);
    bars->setWidth(0.9*m_tStep);
    bars->setData(xs, ys);
    bars->setPen(Qt::NoPen);
    bars->setBrush(QBrush("#ccbb95"));

    resetPlot();
}

void Plotter::resetPlot()
{
    double xMin = m_model->filter().stampA;
    double xMax = m_model->filter().stampB;

    sp::ReaderTMY* format = m_model->formatTMY();

    m_customPlot->xAxis->setRange(m_x0 + xMin,  m_x0 + xMax);
    double ya = format->recordsInfo().min;
    double yb = format->recordsInfo().max;
    double ab = yb - ya;
    ya -= 0.05*ab;
    yb += 0.05*ab;
    m_customPlot->yAxis->setRange(ya, yb);
    m_customPlot->replot();
}

// add axis tag
// add second axis
// https://www.qcustomplot.com/index.php/tutorials/specialcases/axistags
#include "SunPath/calculators/SunCalculatorMB.h"
void Plotter::addSun(bool on)
{
    if (!on) {
//       int gMax = m_customPlot->graphCount();
//       if (gMax > 1)
//       m_customPlot->removeGraph(m_customPlot->graph(1));
//       return;
//        m_customPlot->graph(1)->name()

        for (int g =0; g <m_customPlot->graphCount(); g++) {
         QCPGraph* gr = m_customPlot->graph(g);
         if (gr->name() == "sun") {
             m_customPlot->removeGraph(gr);
             m_customPlot->replot();
            return;
         }
        }
        return;

    }
    sp::ReaderTMY* format = m_model->formatTMY();
    ParameterModel* pm = format->paramEffective();

    sp::SunCalculator* sunCalc = new sp::SunCalculatorMB;
    double latitude = pm->parameterDouble("Location.Latitude")->value();
    double longitude = pm->parameterDouble("Location.Longitude")->value();
    int offsetUTC = pm->parameterDouble("Time.Zone")->value();
    int tStep = pm->parameterDouble("Time.Step")->value();
    int tOrigin = pm->parameterDouble("Time.Origin")->value()*tStep;
    double middle = tOrigin + tStep/2;

    sp::Location location("", latitude, longitude, offsetUTC);
    sunCalc->setLocation(location);

    int a = m_model->filter().indexA;
    int b = m_model->filter().indexB;

    bool leap = pm->parameterDouble("Checks.Time in February")->value() > 28*24*3600;
    m_x0 = 0.;
    if (leap) m_x0 = QDateTime(QDate(1972, 1, 1), QTime(0,0), Qt::UTC).toSecsSinceEpoch();

    double ya = format->recordsInfo().min;
    double yb = format->recordsInfo().max;
    double ab = (yb - ya)/(90*sp::degree);

    QVector<double> xs;
    QVector<double> ys;
    int nMax = format->data().size();
    for (int n = a; n < b; n++) {
        int nf = n;
        if (nf >= nMax) nf -= nMax;
        xs << m_x0 + format->data()[nf].stamp + middle;

        sp::Horizontal hc = sunCalc->findHorizontalV(format->data()[n].time.addSecs(-offsetUTC + middle));
        double y = hc.elevation();
        if (y < 0) y = 0;
        ys << y*ab;
    }

//    QCPGraph* graph0 = m_customPlot->graph(0);
//    graph->setData(graph0->data());

    QCPGraph* graph = m_customPlot->addGraph();
    graph->setName("sun");
    graph->setPen(QPen(Qt::red));
//    bars->setBrush(QBrush("#ccbb95"));
    graph->setData(xs, ys);
    graph->data()->squeeze();
     m_customPlot->replot();


//    QCPGraph* graph = m_customPlot->addGraph();
//    sp::ReaderTMY* format = m_model->formatTMY();


//    double xMin = m_x0 + m_model->filter().stampA;
//    double xMax = m_x0 + m_model->filter().stampB;

//    double ya = format->recordsInfo().min;
//    double yb = format->recordsInfo().max;

//    QVector<double> xs;
//    QVector<double> ys;
//    int nMax = 100;
//    double dx = (xMax - xMin)/(nMax - 1);
//    double dy = (yb - ya)/nMax;
//    for (int n = 0; n < nMax; n++) {
//        xs << xMin + n*dx;
//        ys << ya + dy*n;
//    }
    delete sunCalc;
}

double Plotter::tMin()
{
    return m_x0 + m_model->filter().stampA;
}

double Plotter::tMax()
{
    return m_x0 + m_model->filter().stampB;
}


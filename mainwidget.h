#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <QPen>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_multifit.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>
#include <QPrintDialog>
#include <QPrinter>
#include <QMessageBox>
#include "tearingmach.h"
#include <QFileDialog>
#include <qdesktopwidget.h>
#include <QSettings>

typedef
struct {
    QVector<double> x;
    QVector<double> y0;
    QVector<double> y1;
} pnts;

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT
    
public:
    MainWidget(QWidget *parent = 0);
    ~MainWidget();
    double getTek();
    double getVr();
    double getUd();
    void show();
    void setParam(double d0, double l0, double lr, QString title, bool isParti, bool isW);
    void loadSettings();
    void saveSettings();
    
private:
    Ui::MainWidget *ui;
    QwtPlot *plot;
    QwtPlotCurve *curve;
    QwtPlotCurve *curveEll;
    QwtPlotCurve *curveLine;
    QwtPlotCurve *curveLine02;
    QwtPlotCurve *curveLineUd;
    QwtPlotCurve *curvePb;
    QwtPlotMarker *marker;
    pnts data;
    QString devModbus, devTenz, devLas;
    bool isWire;
    bool tek02;

    double D0;
    double L0;
    double LR;
    TearingMach *mach;
    double pMx,pKn,pTk,lMx,lKn,lTk,vr,tek,udMx,udKn,vpp,upr,pUpr;
    void showCalcs();
    void print(QPrinter *p);

public slots:
    void addDataNG(double t, double p, double dl);
    void plotData();
    void clear();
    void calc();

private slots:
    void setUp(bool val);
    void setDown(bool val);
    void setLock(bool val);
    void setIzmEnabled(bool val);
    void save();
    void updTextMarker();
    void showError(QString mess);
    void setStart();
    void resetError();
    void addData(double t, double p, double dl);
    void printResult();

signals:
    void sigSave();
};

#endif // MAINWIDGET_H

#ifndef PLOT_H
#define PLOT_H
#include "ui_plot.h"
#include <QLabel>

enum PlotType
{
    PLOT_LINE,
    PLOT_BAR
};


class Plot : public QDialog, private Ui::Plot
{
    Q_OBJECT

public:
    explicit Plot(const double *d, int len, PlotType t =PLOT_LINE ,QWidget *parent = nullptr);
    void add(const double *d, int len);
    ~Plot();

private:
    double* m_data;
    double* m_data2;
    int m_len;
    double m_min, m_max;
    QPolygon m_poly;
    QPolygon m_poly2;
    QLabel* m_titulo;
    QLabel* m_ejex;
    QLabel* m_ejey;
    QRect   m_rctTxt;
    PlotType m_type;

    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent*);
    void regenerar();
};

#endif // PLOT_H

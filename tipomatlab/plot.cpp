#include "plot.h"
#include <QPainter>
#include <QFontMetrics>
#include <string.h>
#define W_MARGEN    10
#define H_LINE      20


Plot::Plot(const double *d, int len, PlotType t, QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    m_data=(double*)malloc(sizeof(double)*len);
    memcpy(m_data,d,sizeof(double)*len);
    m_len=len;
    m_titulo = new QLabel("titulo", this);
    m_ejex   = new QLabel("eje x" , this);
    m_ejey   = new QLabel("eje y" , this);
    m_titulo->show();
    m_ejex  ->hide();
    m_ejey  ->hide();
    m_titulo->setAlignment(Qt::AlignCenter);
    m_type = t;
    m_data2 = NULL;

    QFontMetrics fm(font());
    m_rctTxt = fm.boundingRect("-0.0e-00");

    setWindowTitle(t==PLOT_BAR? "Bar" : "Plot");
}


void Plot::add(const double *d, int len)
{
    if(m_data2) free(m_data2);
    m_data2=(double*)malloc(sizeof(double)*len);
    memcpy(m_data2,d,sizeof(double)*len);
}


Plot::~Plot()
{
    free(m_data);
    if(m_data2) free(m_data2);
}


void Plot::resizeEvent(QResizeEvent *)
{    
    regenerar();
}


void Plot::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    int x0 = W_MARGEN+H_LINE;
    int y0 = W_MARGEN+H_LINE;
    int w0 = width()-2*W_MARGEN-H_LINE;
    int h0 = height()-2*W_MARGEN-2*H_LINE;
    p.fillRect(x0, y0, w0, h0, Qt::white);

    p.translate(x0,y0);

    if(m_type==PLOT_LINE)
    {
        p.setPen(Qt::blue);
        p.drawPolyline(m_poly);
        p.setPen(Qt::red);
        p.drawPolyline(m_poly2);
    }
    else if(m_type==PLOT_BAR)
    {
        double ww = w0/m_poly.count();
        p.setPen(Qt::black);
        p.setBrush(Qt::cyan);
        for(int i=0; i<m_poly.count(); i++)
        {
            p.drawRect(m_poly.at(i).x(), m_poly.at(i).y(), ww, h0-m_poly.at(i).y());
        }
    }

    p.drawText(QRect(0,0,m_rctTxt.width(),m_rctTxt.height()),Qt::AlignLeft,QString().sprintf("%.1e",m_max));
    p.drawText(QRect(0,h0-H_LINE,m_rctTxt.width(),m_rctTxt.height()),Qt::AlignLeft,QString().sprintf("%.1e",m_min));
}

void Plot::regenerar()
{
    m_poly.clear();
    m_poly2.clear();
    m_titulo->setGeometry(W_MARGEN+H_LINE,
                          W_MARGEN,
                          width()-2*W_MARGEN-H_LINE,
                          H_LINE);
    // Min-max
    m_min = m_data[0];
    m_max = m_data[0];
    if(m_len<=0) return;

    // Espacio de dibujo
    double w = width()-2*W_MARGEN-H_LINE;
    double h = height()-2*W_MARGEN-2*H_LINE;

    for(int i=1; i<m_len; i++)
    {
        if(m_data[i]>m_max) m_max=m_data[i];
        if(m_data[i]<m_min) m_min=m_data[i];
    }

    // Limites sin dibujo
    if(m_min>m_max) return;
    else if(m_min==m_max)
    {
        m_poly << QPoint(0,h/2);
        m_poly << QPoint(w,h/2);
        return;
    }

    // Dibujar
    double rango = m_max-m_min;
    double dx = w/m_len;
    double y = h-(m_data[0]-m_min)*h/rango;

    double y2=0;
    if(m_data2) y2 = h-(m_data2[0]-m_min)*h/rango;
    m_poly << QPoint(0,y);
    m_poly2 << QPoint(0,y2);
    for(int i=1; i<m_len; i++)
    {
        y = h-(m_data[i]-m_min)*h/rango;
        m_poly << QPoint(i*dx,y);
        if(m_data2)
        {
            y2 = h-(m_data2[i]-m_min)*h/rango;
            m_poly2 << QPoint(i*dx,y2);
        }
    }
}



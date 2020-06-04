#include "imshow.h"

#include <QPainter>

Imshow::Imshow(const Matrix& img, double rango[2], QWidget *parent) :
    QDialog(parent)
{
    m_img = new QImage(img.cols(), img.rows(),QImage::Format_ARGB32_Premultiplied);
    m_img->fill(Qt::black);
    uint* d = (uint*)m_img->bits();

    const double* src = img.data();
    int w = img.cols();
    int h = img.rows();
    int n = w*h;

    // Rango
    double low=src[0];
    double high=src[0];
    if(rango != NULL)
    {
        low=rango[0];
        high=rango[1];
    }
    else
    {
        for(int i=1; i<n; i++)
        {
            if(src[i]>high) high=src[i];
            if(src[i]<low)  low=src[i];
        }
    }

    if(low==high) high = low+1;
    double r = high-low;
    double scale = 255/r;
    for(int i=0; i<n; i++)
    {
        int clr = (src[i]-low)*scale;
        d[i] = 0xff000000 | (clr<<16) | (clr<<8) | clr;
    }

    setWindowTitle("imshow");
}

Imshow::~Imshow()
{
    delete  m_img;
}


void Imshow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    float w = width();
    float h = height();
    float win = w/h;
    float im  = (float)m_img->width()/(float)m_img->height();

    if(win>im)
    {
        float scale = h/(float)m_img->height();
        p.drawImage(QRect(0,0,m_img->width()*scale,m_img->height()*scale), *m_img);
    }
    else
    {
        float scale = w/(float)m_img->width();
        p.drawImage(QRect(0,0,m_img->width()*scale,m_img->height()*scale), *m_img);
    }


}

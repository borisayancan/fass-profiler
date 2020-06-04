#ifndef IMSHOW_H
#define IMSHOW_H
#include <QDialog>
#include "maths/math_tipos.h"

class Imshow : public QDialog
{
public:
    explicit Imshow(const Matrix& img, double rango[2]=NULL, QWidget* parent=NULL);
    ~Imshow();

private:
    QImage* m_img;
    QPolygon m_poly;
    void paintEvent(QPaintEvent*);


};

#endif // IMSHOW_H

#include "fmain.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <fcntl.h>
#include <QElapsedTimer>
#include "tipomatlab/tipomatlab.h"
#include <stdio.h>


/**********************************************************************
 *
 *  Demostrador del algoritmo del profiler del FASS
 *  Version C/C++ Boris Ayancan, Junio 2020
 *
 * *******************************************************************/
FMain::FMain(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
}


/**********************************************************************
 *
 *  Abrir archivo con frames .fass y analizar
 *
 * *******************************************************************/
void FMain::on_btnFrames_clicked()
{
    // Abrir archivo
    QString init_dir = get_key("init_dir");
    QString file = QFileDialog::getOpenFileName(this,"Frames FASS",init_dir, "Archivo FASS (*.fass)");
    if(file.isEmpty()) return;
    set_key("init_dir", QFileInfo(file).path());

    // Verificar integridad y cargar en ram
    QElapsedTimer tic; tic.start();
    int num_frames;
    T_FassFile* frames = read_file(file, &num_frames);
    if(frames==NULL)
    {
        QMessageBox::critical(this,"Error","No se pudo abrir el archivo indicado");
        return;
    }

    // Iniciar profiler
    qDebug("Iniciando profiler para %d frames...", num_frames);
    bool init = fass_profiler_init(frames->width, frames->height, num_frames);
    if(!init)
    {
        QMessageBox::critical(this,"Error","No se pudo iniciar el profiler, verifique archivos de calibracion");
        return;
    }

    // Analizar en el profiler
    QApplication::setOverrideCursor(Qt::WaitCursor);
    Matrix img = zeros(frames->height, frames->width);
    u16* src = &frames->frame_start;
    int block = frames->height * frames->width;
    for(int i=8; i<num_frames; i++)
    {
        img.load(&src[block*i]);
        fass_profiler_push_image(img);
    }

    // Finalizar
    Vector layers;
    fass_profiler_calcular(layers);
    free(frames);

    qDebug("Analisis en %lld ms", tic.elapsed());
    QApplication::restoreOverrideCursor();

    // Mostrar resultado
    Vector vmed, coefs, vcalc;
    fass_profiler_results(coefs, vmed, vcalc);

    plot_hold_on(vmed,vcalc);
    display(coefs);
    bar(coefs);
}


/**********************************************************************
 *
 *  Retorna el valor del key indicado
 *
 * *******************************************************************/
QString FMain::get_key(const char *key)
{
    QFile f(key);
    QString ret;
    if(f.open(QIODevice::ReadOnly))
    {
        QTextStream ts(&f);
        ret = ts.readLine(64);
        f.close();
    }

    return ret;
}


/**********************************************************************
 *
 *  Establece el valor del key al valor val
 *
 * *******************************************************************/
void FMain::set_key(const char *key, const QString &val)
{
    QFile f(key);
    if(f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        f.write(val.toUtf8().constData());
        f.close();
    }
}


/**********************************************************************
 *
 *  Lee un set de frames FASS desde el archivo indicado y devuelve
 *  una referencia a estos.
 *  Retorna NULL en caso de falla.
 *  En caso de exito, el puntero se debe liberar con free
 *
 * *******************************************************************/
T_FassFile *FMain::read_file(const QString &path, int* num_frames)
{
    *num_frames=0;
    QFileInfo fi(path);
    if(!fi.exists())
        return NULL;

    // Cargar en RAM
    char* ram_file = (char*)malloc(fi.size());
    if(ram_file==NULL)
    {
        QMessageBox::critical(this,"Error","Memoria de sistema insuficiente");
        return NULL;
    }

    FILE* file=fopen(path.toUtf8().constData(),"r");
    if(file==NULL)
    {
        free(ram_file);
        return NULL;
    }

    fread(ram_file,1,fi.size(),file);
    fclose(file);

    // Testear
    T_FassFile* d = (T_FassFile*)ram_file;
    bool valido = d->width>0 && d->width<=4096 &&
                  d->height>0 && d->height<=4096 ;

    if(!valido)
    {
        free(ram_file);
        return NULL;
    }

    int szh = sizeof(T_FassFile)-sizeof(u16);
    int sz_fr = d->width*d->height*2;
    int nfr = (fi.size()-szh)/sz_fr;

    if(fi.size() != (szh+sz_fr*nfr))
    {
        free(ram_file);
        return NULL;
    }

    *num_frames = nfr;
    return d;
}

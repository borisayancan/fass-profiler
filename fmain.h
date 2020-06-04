#ifndef FMAIN_H
#define FMAIN_H
#include "ui_fmain.h"
#include "fass/fass.h"

class FMain : public QDialog, private Ui::FMain
{
    Q_OBJECT

public:
    explicit FMain(QWidget *parent = nullptr);

private slots:
    void on_btnFrames_clicked();

private:
    QString get_key(const char* file);
    void set_key(const char *key, const QString &val);
    T_FassFile* read_file(const QString& path, int *num_frames);
};

#endif // FMAIN_H

#include "PhotonsFileWidget.h"
#include "ui_PhotonsFileWidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "PhotonsFile.h"


PhotonsFileWidget::PhotonsFileWidget(QWidget* parent):
    PhotonsWidget(parent),
    ui(new Ui::PhotonsFileWidget)
{
    ui->setupUi(this);
    connect(ui->directoryButton, SIGNAL(clicked()), this, SLOT(selectDirectory()));
}

PhotonsFileWidget::~PhotonsFileWidget()
{
    delete ui;
}

QStringList PhotonsFileWidget::getParameterNames() const
{
    return PhotonsFile::getParameterNames();
}

QString PhotonsFileWidget::getParameterValue(QString name) const
{
    QStringList names = getParameterNames();

    if (name == names[0])
        return ui->directoryEdit->text();
    else if (name == names[1])
        return ui->fileEdit->text();
    else if (name == names[2])
    {
        if (!ui->photonsCheck->isChecked())
            return QString::number(-1);
        else
            return QString::number(ui->photonsSpin->value());
	}
	return QString();
}

void PhotonsFileWidget::selectDirectory()
{
    QSettings settings("Tonatiuh", "Cyprus");
    QString dirName = settings.value("dirProjects", "").toString();

    dirName = QFileDialog::getExistingDirectory(this, "Save Directory", dirName);
    if (dirName.isEmpty()) return;

    QDir dir(dirName);
    if (!dir.exists())
	{
        QMessageBox::information(this, "Tonatiuh", "Selected directory is not valid");
        return;
	}

    settings.setValue("dirProjects", dirName);
    ui->directoryEdit->setText(dirName);
}

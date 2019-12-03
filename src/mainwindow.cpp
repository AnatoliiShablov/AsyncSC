#include "mainwindow.h"
#include "../ui_forms/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::key_pressed(QKeyEvent *event)
{
    if(event->key() == Qt::Key::Key_Escape) {
        if(ui->multi_form->currentWidget() == ui->dialog_page) {

            ui->multi_form->setCurrentWidget(ui->sign_page);
        } else if(ui->multi_form->currentWidget()== ui->sign_page){
            this->close();
        }
    }
}


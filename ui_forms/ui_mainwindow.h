/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.13.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow {
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QStackedWidget *multi_form;
    QWidget *sign_page;
    QVBoxLayout *verticalLayout_2;
    QLabel *nice_text;
    QLineEdit *login_field;
    QLineEdit *password_field;
    QHBoxLayout *buttons_layout;
    QPushButton *signin_button;
    QPushButton *signup_button;
    QLabel *additional_info;
    QSpacerItem *spacer;
    QWidget *dialog_page;
    QVBoxLayout *verticalLayout_3;
    QListWidget *listWidget;
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEdit;
    QPushButton *pushButton;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow) {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(400, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        multi_form = new QStackedWidget(centralwidget);
        multi_form->setObjectName(QString::fromUtf8("multi_form"));
        sign_page = new QWidget();
        sign_page->setObjectName(QString::fromUtf8("sign_page"));
        verticalLayout_2 = new QVBoxLayout(sign_page);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        nice_text = new QLabel(sign_page);
        nice_text->setObjectName(QString::fromUtf8("nice_text"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(nice_text->sizePolicy().hasHeightForWidth());
        nice_text->setSizePolicy(sizePolicy);
        QFont font;
        font.setFamily(QString::fromUtf8("ae_AlMateen"));
        font.setPointSize(32);
        font.setBold(true);
        font.setItalic(true);
        font.setWeight(75);
        nice_text->setFont(font);
        nice_text->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(nice_text);

        login_field = new QLineEdit(sign_page);
        login_field->setObjectName(QString::fromUtf8("login_field"));

        verticalLayout_2->addWidget(login_field);

        password_field = new QLineEdit(sign_page);
        password_field->setObjectName(QString::fromUtf8("password_field"));
        password_field->setEchoMode(QLineEdit::Password);

        verticalLayout_2->addWidget(password_field);

        buttons_layout = new QHBoxLayout();
        buttons_layout->setObjectName(QString::fromUtf8("buttons_layout"));
        signin_button = new QPushButton(sign_page);
        signin_button->setObjectName(QString::fromUtf8("signin_button"));

        buttons_layout->addWidget(signin_button);

        signup_button = new QPushButton(sign_page);
        signup_button->setObjectName(QString::fromUtf8("signup_button"));

        buttons_layout->addWidget(signup_button);

        verticalLayout_2->addLayout(buttons_layout);

        additional_info = new QLabel(sign_page);
        additional_info->setObjectName(QString::fromUtf8("additional_info"));
        additional_info->setAlignment(Qt::AlignCenter);

        verticalLayout_2->addWidget(additional_info);

        spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(spacer);

        multi_form->addWidget(sign_page);
        dialog_page = new QWidget();
        dialog_page->setObjectName(QString::fromUtf8("dialog_page"));
        verticalLayout_3 = new QVBoxLayout(dialog_page);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        listWidget = new QListWidget(dialog_page);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(listWidget->sizePolicy().hasHeightForWidth());
        listWidget->setSizePolicy(sizePolicy1);

        verticalLayout_3->addWidget(listWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        lineEdit = new QLineEdit(dialog_page);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(lineEdit->sizePolicy().hasHeightForWidth());
        lineEdit->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(lineEdit);

        pushButton = new QPushButton(dialog_page);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(pushButton->sizePolicy().hasHeightForWidth());
        pushButton->setSizePolicy(sizePolicy3);

        horizontalLayout->addWidget(pushButton);

        verticalLayout_3->addLayout(horizontalLayout);

        multi_form->addWidget(dialog_page);

        verticalLayout->addWidget(multi_form);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        multi_form->setCurrentIndex(0);

        QMetaObject::connectSlotsByName(MainWindow);
    }  // setupUi

    void retranslateUi(QMainWindow *MainWindow) {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        nice_text->setText(QCoreApplication::translate("MainWindow", "Sign IN / Sign UP", nullptr));
        signin_button->setText(QCoreApplication::translate("MainWindow", "Sign In", nullptr));
        signup_button->setText(QCoreApplication::translate("MainWindow", "Sign Up", nullptr));
        additional_info->setText(QCoreApplication::translate("MainWindow", "Nothing", nullptr));
        pushButton->setText(QCoreApplication::translate("MainWindow", "Send", nullptr));
    }  // retranslateUi
};

namespace Ui {
class MainWindow: public Ui_MainWindow {};
}  // namespace Ui

QT_END_NAMESPACE

#endif  // UI_MAINWINDOW_H

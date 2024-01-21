#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QMouseEvent>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QFileDialog>
#include <QTextStream>
#include <QVector>

class MyDialog : public QDialog {
public:
    MyDialog();
    QVBoxLayout* mainLayout;
    QWidget* editWidget;
    QFormLayout* editLayout;
    QLineEdit* lineEdit;
    QDialogButtonBox* buttonBox;
};

MyDialog::MyDialog() {
    lineEdit = new QLineEdit;
    editLayout = new QFormLayout;
    editLayout->addRow(new QLabel(tr("Novi tekst:")), lineEdit);
    editWidget = new QWidget;
    editWidget->setLayout(editLayout);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(editWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle("Promjena teksta");
}

class MyMainWindow : public QMainWindow {
public:
    MyMainWindow();
    QVector<QLabel*> labels;

    void mousePressEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

    void FileSaveAsMenu();
    void FileOpenMenu();
    QMenu* FileMenu;
    QAction* FileSaveAs;
    QAction* FileOpen;
};

void MyMainWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        MyDialog dialog;
        int ret = dialog.exec();
        if (ret == QDialog::Accepted) {
            QLabel* newLabel = new QLabel(this);
            newLabel->setText(dialog.lineEdit->text());
            newLabel->move(event->position().toPoint());
            labels.append(newLabel);

            updateGeometry();
            update();
        }
    }


    else if (event->button() == Qt::RightButton) {

        for (int i = 0; i < labels.size(); ++i) {
            QLabel* label = labels.at(i);
            if (label->geometry().contains(event->pos())) {
                labels.removeAt(i);
                label->deleteLater();
                updateGeometry();
                update();
                break;
            }
        }
    }
}



void MyMainWindow::paintEvent(QPaintEvent*) {
    QSize size = this->size();
    QPainter painter(this);
    painter.drawLine(0, size.height(), size.width(), 20);

    for (const QLabel* label : labels) {
        painter.drawText(label->pos(), label->text());
    }
}

MyMainWindow::MyMainWindow() {
    FileSaveAs = new QAction(tr("&Save As..."), this);
    FileSaveAs->setShortcut(tr("CTRL+S"));
    connect(FileSaveAs, &QAction::triggered, this, &MyMainWindow::FileSaveAsMenu);

    FileOpen = new QAction(tr("&Open..."), this);
    FileOpen->setShortcut(tr("CTRL+O"));
    connect(FileOpen, &QAction::triggered, this, &MyMainWindow::FileOpenMenu);

    FileMenu = menuBar()->addMenu(tr("&File"));
    FileMenu->addAction(FileSaveAs);
    FileMenu->addAction(FileOpen);
}

void MyMainWindow::FileSaveAsMenu() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save As...", "", "FESB File (*.fsb)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, "Unable to Open File", file.errorString());
            return;
        }

        QTextStream out(&file);
        out << "fesb file" << Qt::endl;

        for (const QLabel* label : labels) {
            out << label->text() << Qt::endl;
            out << label->pos().x() << Qt::endl;
            out << label->pos().y() << Qt::endl;
        }

        QMessageBox::information(this, "File Saved", "File saved successfully!");
    }
}

void MyMainWindow::FileOpenMenu() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Geometry...", "", "FESB File (*.fsb)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, "Unable to Open File", file.errorString());
            return;
        }

        QTextStream in(&file);
        QString str;
        str = in.readLine();

        if (str == "fesb file") {
            while (!in.atEnd()) {
                str = in.readLine();
                QLabel* newLabel = new QLabel(this);
                newLabel->setText(str);

                int x, y;
                in >> x >> y;
                newLabel->move(x, y);

                labels.append(newLabel);
            }

            update();
            QMessageBox::information(this, "File Opened", "File opened successfully!");
        } else {
            QMessageBox::information(this, "Invalid File Format", "The selected file is not a valid FESB file.");
        }
    }
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    MyMainWindow mainWindow;
    mainWindow.resize(300, 150);

    mainWindow.show();
    return app.exec();
}

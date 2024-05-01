#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QMouseEvent>
namespace Ui {
class TitleBar;
}

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();


private slots:
    void close();
    void hide();
    void maximize();

private:
    Ui::TitleBar *ui;
    QWidget* parent;
};

#endif // TITLEBAR_H

#ifndef OSDWIDGET_H
#define OSDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>

//class QLabel;
//class QPropertyAnimation;
//class QTimer;

class OSDWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OSDWidget(QWidget *parent = nullptr);
    ~OSDWidget();
    // Show text for visibleMs milliseconds (fade in/out handled automatically).
    void showMessage(const QString &text, int visibleMs);
    void showMessage(const QString &text);
    // Optional: enable/disable click-through (Windows only).
    void setClickThrough(bool enable);
    void setDuration(int iDuration);
    int duration();
    void setTextSize(int iTextSize);
    void setPosition(const QString &sPosition);
protected:
    void showEvent(QShowEvent *event) override;
private slots:
    void onFadeInFinished();
    void onVisibilityTimeout();
    void onFadeOutFinished();
private:
    void positionAtBottomRight();
    void positionAtCenter();
    QLabel *m_label;
    QPropertyAnimation *m_fadeIn;
    QPropertyAnimation *m_fadeOut;
    QTimer *m_visibilityTimer;
    int m_iDuration;
    int m_iDurationPassed;
    int m_iTextSize;
    bool m_clickThrough;
    const int DEFAULT_DURATION = 2000;
    bool m_bDurationPassed;
    QString m_sPosition;
};

#endif // OSDWIDGET_H

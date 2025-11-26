#include "osdwidget.h"
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QScreen>

#ifdef Q_OS_WIN
    #include <windows.h>
    #include <QtWinExtras> // optional, but we only use native handle below
#endif

OSDWidget::OSDWidget(QWidget *parent)
    : QWidget(parent),
      m_label(new QLabel(this)),
      m_fadeIn(new QPropertyAnimation(this, "windowOpacity")),
      m_fadeOut(new QPropertyAnimation(this, "windowOpacity")),
      m_visibilityTimer(new QTimer(this)),
      m_iDuration(2000),
      m_iDurationPassed(2000),
      m_iTextSize(16),
      m_clickThrough(false),
      m_bDurationPassed(false),
      m_sPosition("Center")
{
    // Window flags: frameless, stays on top, tool so it doesn't show in taskbar.
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    // Allow translucent background
    setAttribute(Qt::WA_TranslucentBackground);
    // Don't activate window when shown
    setAttribute(Qt::WA_ShowWithoutActivating);
    // Don't accept focus
    setFocusPolicy(Qt::NoFocus);
    // Label style: semi-transparent background, padding, rounded corners
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setTextInteractionFlags(Qt::NoTextInteraction);
    m_label->setStyleSheet(
        "QLabel { "
        " color: white; "
        " background-color: rgba(16, 16, 16, 160); "
        " padding: 10px 18px; "
        " border-radius: 16px; "
        " font-size: 24px; "
        " }"
    );
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_label);
    // Fade in animation (0 -> 1)
    m_fadeIn->setDuration(300);
    m_fadeIn->setStartValue(0.0);
    m_fadeIn->setEndValue(1.0);
    connect(m_fadeIn, SIGNAL(finished()), this, SLOT(onFadeInFinished()));
    // Fade out animation (1 -> 0)
    m_fadeOut->setDuration(400);
    m_fadeOut->setStartValue(1.0);
    m_fadeOut->setEndValue(0.0);
    connect(m_fadeOut, SIGNAL(finished()), this, SLOT(onFadeOutFinished()));
    m_visibilityTimer->setSingleShot(true);
    connect(m_visibilityTimer, SIGNAL(timeout()), this, SLOT(onVisibilityTimeout()));
    // initial opacity 0
    setWindowOpacity(0.0);
}

OSDWidget::~OSDWidget()
{
}

void OSDWidget::showMessage(const QString &text)
{
    if (m_iDuration == 0)
    {
        showMessage (text, DEFAULT_DURATION);
        return;
    }
    m_bDurationPassed = false;
    // else visibleMs = m_iDuration;
    m_label->setText(text);
    adjustSize();
    if (m_sPosition == "Center")
        positionAtCenter ();
    else positionAtBottomRight ();
    // positionAtBottomRight();
    // make sure any existing animations/timers stop
    m_fadeOut->stop();
    m_visibilityTimer->stop();
    // show without activating
    show();
    raise();
    // start fade in
    m_fadeIn->stop();
    m_fadeIn->setDirection(QAbstractAnimation::Forward);
    m_fadeIn->start();
}

void OSDWidget::showMessage(const QString &text, int visibleMs)
{
    // QSettings settings;
    // m_iTextSize = settings.value ("OSD_TextSize", 16).toInt ();
    // m_iDuration = settings.value ("OSD_Duration", 2000).toInt ();
    // QString sStyleSheet = "QLabel { "
    // " color: white; "
    // " background-color: rgba(16, 16, 16, 160); "
    // " padding: 10px 18px; "
    // " border-radius: 16px; "
    // " font-size: ";
    // sStyleSheet.append (QString::number (m_iTextSize));
    // sStyleSheet.append ("px; }");
    // m_label->setStyleSheet (sStyleSheet);
    m_bDurationPassed = true;
    m_iDurationPassed = visibleMs;
    // else visibleMs = m_iDuration;
    m_label->setText(text);
    adjustSize();
    if (m_sPosition == "Center")
        positionAtCenter ();
    else positionAtBottomRight ();
    // positionAtBottomRight();
    // make sure any existing animations/timers stop
    m_fadeOut->stop();
    m_visibilityTimer->stop();
    // show without activating
    show();
    raise();
    // start fade in
    m_fadeIn->stop();
    m_fadeIn->setDirection(QAbstractAnimation::Forward);
    m_fadeIn->start();
}

void OSDWidget::setClickThrough(bool enable)
{
    m_clickThrough = enable;
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());
    if (!hwnd)
        return;
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (enable)
    {
        exStyle |= WS_EX_TRANSPARENT | WS_EX_LAYERED;
    }
    else
    {
        exStyle &= ~WS_EX_TRANSPARENT;
        // keep layered if you rely on per-pixel alpha
        exStyle |= WS_EX_LAYERED;
    }
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
    // Force a style update
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#else
    // On other platforms you'd use platform-specific API (X11 input shapes, etc.)
    (void)enable;
#endif
}

void OSDWidget::showEvent(QShowEvent * /*event*/)
{
    // Ensure we keep WA_ShowWithoutActivating and not grab focus
    setWindowOpacity(windowOpacity()); // no-op but ensures property exists
}

void OSDWidget::onFadeInFinished()
{
    // start visibility timer
    if (m_bDurationPassed)
        m_visibilityTimer->start(m_iDurationPassed);
    else
    {
        if (m_iDuration > 0)
            m_visibilityTimer->start(m_iDuration);
    }
}

void OSDWidget::onVisibilityTimeout()
{
    // start fade out
    m_fadeOut->stop();
    m_fadeOut->setDirection(QAbstractAnimation::Forward);
    m_fadeOut->start();
}

void OSDWidget::onFadeOutFinished()
{
    hide();
    setWindowOpacity(0.0);
}

void OSDWidget::positionAtCenter()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;
    QRect available = screen->availableGeometry();
    int x = available.center().x() - width() / 2;
    int y = available.center().y() - height() / 2;
    move(x, y);
}

void OSDWidget::setPosition(const QString &sPosition)
{
    m_sPosition = sPosition;
}

void OSDWidget::positionAtBottomRight()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;
    QRect available = screen->availableGeometry();
    const int margin = 20;
    move(available.right() - width() - margin,
        available.bottom() - height() - margin);
}

void OSDWidget::setTextSize(int iTextSize)
{
//    qDebug() << __PRETTY_FUNCTION__;
    m_iTextSize = iTextSize;
    QString sStyleSheet = "QLabel { "
                          " color: white; "
                          " background-color: rgba(16, 16, 16, 160); "
                          " padding: 10px 18px; "
                          " border-radius: 16px; "
                          " font-size: ";
    sStyleSheet.append (QString::number (m_iTextSize));
    sStyleSheet.append ("px; }");
    m_label->setStyleSheet (sStyleSheet);
}

void OSDWidget::setDuration(int iDuration)
{
    m_iDuration = iDuration;
}

int OSDWidget::duration()
{
    return m_iDuration;
}

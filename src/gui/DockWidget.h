#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>
#include <qwidget.h>
//#include <JoystickWidget.h>
//#include <qgamepad.h>

class ViewGroup;
class RouteGroup;
class FlightSimGroup;
class FlightQueryGroup;
class BasicDataGroup;

class LeftDockWidget : public QDockWidget {
    Q_OBJECT

public:
    LeftDockWidget(QWidget *parent = nullptr);
    ~LeftDockWidget();

private:
    QVBoxLayout *mpMainLayout;
    QScrollArea *mpScrollArea;
    QWidget *mpDockContent;
    ViewGroup *mpViewGroup;
    RouteGroup *mpRouteGroup;
    FlightSimGroup *mpFlightSimGroup;
    FlightQueryGroup *mpFlightQueryGroup;
    BasicDataGroup *mpBasicDataGroup;
    void createScrollArea(QWidget *parent);
    void createDockContent(QWidget *parent);
};

class JoystickWidget : public QWidget {
    Q_OBJECT

public:
    JoystickWidget(QWidget *parent = nullptr);
    ~JoystickWidget();

    // private slots:
    //   void handleJoystickMove(float dx, float dy);
    //   void switchToManualMode();
    //   void switchToAutoMode();
    //
    // private:
    //   QGamepad *m_gamepad = nullptr;
    //   JoystickWidget *m_leftJoystick = nullptr;
    //   JoystickWidget *m_rightJoystick = nullptr;
};

class RightDockWidget : public QDockWidget {
    Q_OBJECT

public:
    RightDockWidget(QWidget *parent = nullptr);
    ~RightDockWidget();
private:
    JoystickWidget *mJoystickWidget;
    QTreeWidget *mpFileTreeWidget;
    void createJoyDockWidgets();
    void onSelectDirectoryClicked();
    void loadDirectoryFiles(const QString &path);
    void loadDirectoryLevel(QTreeWidgetItem *parentItem, const QString &path,
                            int level, int maxLevel);
    void onTreeItemExpanded(QTreeWidgetItem *item);
    void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);
    QString getItemFullPath(QTreeWidgetItem *item);
};

template<typename LayoutType>
class FunctionGroup : public QGroupBox {
    Q_OBJECT

public:
    FunctionGroup(const QString &title, const QString &objectName, QWidget *parent = nullptr) : QGroupBox(title, parent) {
        setObjectName(objectName);
        mpGroupLayout = new QVBoxLayout(this);
        QString groupLayoutName = objectName + "Layout";
        mpGroupLayout->setObjectName(groupLayoutName);
    }
    ~FunctionGroup();

protected:
    LayoutType *mpGroupLayout;
    virtual void createSpins(){}
    virtual void createButtons(){}
};

class ViewGroup : protected FunctionGroup<QVBoxLayout> {
    Q_OBJECT

public:
    ViewGroup(QWidget *parent = nullptr);
    ~ViewGroup();

private:
    QPushButton *mpBtnReset;
    QPushButton *mpBtnSwitchTo3D;
    QPushButton *mpBtnSwitchTo2D;
};

class RouteGroup : protected FunctionGroup<QFormLayout> {
    Q_OBJECT

public:
    RouteGroup(QWidget *parent = nullptr);
    ~RouteGroup();

private:
    void createSpins() override;
    void createButtons() override;
    QVBoxLayout *mpButtonLayout;
    QWidget *mpButtonContainer;
    QDoubleSpinBox *mpBaseHeightSpin;
    QDoubleSpinBox *mpHeightSpin;
    QDoubleSpinBox *mpWidthSpin;
    QPushButton *mpBtnCreateRoute;
    QPushButton *mpBtnSetHome;
    QPushButton *mpBtnAddControlPoint;
    QPushButton *mpBtnEditPoint;
    QPushButton *mpBtnGenerate;
};

class FlightSimGroup : protected FunctionGroup<QFormLayout> {
    Q_OBJECT

public:
    FlightSimGroup(QWidget *parent = nullptr);
    ~FlightSimGroup();

private:
    void createSpins() override;
    void createButtons() override;
    QHBoxLayout *mpControlRow1;
    QHBoxLayout *mpControlRow2;
    QHBoxLayout *mpControlRow3;
    QDoubleSpinBox *mpSpeedSpin;
    QPushButton *mpBtnStart;
    QPushButton *mpBtnPause;
    QPushButton *mpBtnResume;
    QPushButton *mpBtnReturn;
    QPushButton *mpBtnStop;
};

class FlightQueryGroup : protected FunctionGroup<QVBoxLayout> {
  Q_OBJECT

public:
  FlightQueryGroup(QWidget *parent = nullptr);
  ~FlightQueryGroup();

private:
  void createButtons() override;
  QPushButton *mpBtnQueryParams;
  QLabel *mpFlightParamsDisplay;
};

class BasicDataGroup : protected FunctionGroup<QFormLayout> {
  Q_OBJECT

public:
  BasicDataGroup(QWidget *parent = nullptr);
  ~BasicDataGroup();

private:
  void createButtons() override;
  QPushButton *mpBtnRefreshData;
  QLabel *mpWeatherLabel;
  QLabel *mpTemperatureLabel;
  QLabel *mpPressureLabel;
};

#endif // DOCKWIDGET_H
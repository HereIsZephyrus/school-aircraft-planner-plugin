#include "plugin.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <memory>
#include <qnamespace.h>

namespace {
const QString s_name = QStringLiteral("3D school airspace simulation");
const QString s_description = QStringLiteral("3D school airspace simulation");
const QString s_category = QStringLiteral("Plugins");
const QString s_version = QStringLiteral("Version 0.0.1");
const QString s_icon = QStringLiteral(":/plugin.svg");
const QgisPlugin::PluginType s_type = QgisPlugin::UI;
} // namespace

QGISEXTERN QgisPlugin *classFactory(QgisInterface *qgis_if) {
  std::cout << "::classFactory" << std::endl;
  return new SchoolPlugin3D(qgis_if);
}

// See QGIS breaking change introduced qith QGIS 3.22:
// https://github.com/qgis/QGIS/commit/b3c5cf8d5fc1fdc289f1449df548acf9268140c6
#if _QGIS_VERSION_INT >= 32200
// Receny versions of QGIS use pointer to strings to pass the plugin
// information.
QGISEXTERN const QString *name() { return &s_name; }

QGISEXTERN const QString *description() { return &s_description; }

QGISEXTERN const QString *category() { return &s_category; }

QGISEXTERN const QString *version() { return &s_version; }

QGISEXTERN const QString *icon() { return &s_icon; }
#else
// Older versions of QGIS return the plugin names as copies.
QGISEXTERN const QString name() { return s_name; }

QGISEXTERN const QString description() { return s_description; }

QGISEXTERN const QString category() { return s_category; }

QGISEXTERN const QString version() { return s_version; }

QGISEXTERN const QString icon() { return s_icon; }
#endif

QGISEXTERN int type() { return s_type; }

QGISEXTERN void unload(QgisPlugin *plugin) {
  std::cout << "::unload" << std::endl;
  delete plugin;
}

SchoolPlugin3D ::SchoolPlugin3D(QgisInterface *iface)
    : QgisPlugin(s_name, s_description, s_category, s_version, s_type),
      m_qgis_if(iface) {}

void SchoolPlugin3D ::unload() {
  // TODO - need to remove the actions from the menu again.
}

void SchoolPlugin3D ::initGui() {
  QgsMessageLog::logMessage(QString("SchoolPlugin3D::initGui"),
                            QString("SchoolPlugin3D"),
                            Qgis::MessageLevel::Info);

  // add an action to the menu
  m_menu_action = new QAction(QIcon(":/plugin.svg"), QString("3D school-model"), this);
  connect(m_menu_action, SIGNAL(triggered()), this, SLOT(menu_button_action()));
  if (m_qgis_if == nullptr) {
    QgsMessageLog::logMessage(
        QString("failed to get the handle to QGIS"),
        QString("SchoolPlugin3D"),
        Qgis::MessageLevel::Critical
    );
    return;
  }
  m_qgis_if->addPluginToMenu(QString("&3D school-model"), m_menu_action);
  m_menu_action->setToolTip(tr("启动3D学校模型"));
  m_menu_action->setStatusTip(tr("打开3D学校模型界面"));
}

void SchoolPlugin3D ::menu_button_action() {
  QgsMessageLog::logMessage(QString("Open Window"),
                            QString("SchoolPlugin3D"),
                            Qgis::MessageLevel::Info);
  MainWindow& plugin_window = MainWindow::getInstance();
  plugin_window.setAttribute(Qt::WA_DeleteOnClose);
  plugin_window.setWindowModality(Qt::ApplicationModal);
  
  if (QApplication::instance()) {
    QgsMessageLog::logMessage(QString("Event loop exists"),
                            QString("SchoolPlugin3D"),
                            Qgis::MessageLevel::Success);
  } else {
    QgsMessageLog::logMessage(QString("No event loop!"),
                            QString("SchoolPlugin3D"),
                            Qgis::MessageLevel::Critical);
  }
  
  plugin_window.show();
}
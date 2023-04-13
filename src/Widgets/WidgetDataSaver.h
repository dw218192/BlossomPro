#pragma once
#include <QSettings>
#include <QString>
#include <QMetaProperty>
#include <QDebug>
#include <cassert>
#include <qcoreapplication.h>

class WidgetDataSaver {
public:
    explicit WidgetDataSaver(QWidget* widget) : m_owner(widget) {
        assert(widget);

        while (widget) {
            m_path.prepend(widget->objectName());
            widget = widget->parentWidget();
            if (widget) {
                m_path.prepend("/");
            }
        }

        qDebug()
    		<< "WidgetDataSaver: saving data at \n"
    		<< getSetting().fileName()
    		<< "\n";
    }
    WidgetDataSaver(WidgetDataSaver&) = delete;
    WidgetDataSaver(WidgetDataSaver&&) = delete;

    bool shouldSaveProperty(const QMetaProperty& property) {
        QString name{ property.name() };
        return name.startsWith("prop_") && property.isReadable() && property.isWritable() && property.isStored();
    }

    QSettings getSetting() {
        return QSettings {
		    QCoreApplication::instance()->applicationDirPath() + '/' + 
            QCoreApplication::instance()->applicationName() + ".ini",
		    QSettings::Format::IniFormat
        };
    }
    void saveData() {
        QSettings settings = getSetting();
    	settings.beginGroup(m_path);

        const QMetaObject* metaObject = m_owner->metaObject();

        if(metaObject->classInfoCount() > 0) {
	        // write version
            QMetaClassInfo info = metaObject->classInfo(0);
            if(strcmp(info.name(), "version") == 0) {
                settings.setValue("version", QString::fromUtf8(info.value()));
            }
        }

        for (int i = 0; i < metaObject->propertyCount(); ++i) {
            QMetaProperty property = metaObject->property(i);
            if (shouldSaveProperty(property)) {
                QString key = QString::fromUtf8(property.name());
                QVariant value = property.read(m_owner);

            	settings.setValue(key, value);
                settings.sync();
            }
        }

        settings.endGroup();
    }

    void loadData() {
        QSettings settings = getSetting();
        settings.beginGroup(m_path);

        const QMetaObject* metaObject = m_owner->metaObject();

        if (metaObject->classInfoCount() > 0) {
            // check version
            QMetaClassInfo info = metaObject->classInfo(0);
            if (strcmp(info.name(), "version") == 0) {
                QString ver = settings.value("version", "").toString();
                if(!ver.isEmpty() && ver != info.value()) {
                    qDebug() << "the file was saved with an older version of the widget\n"
                        << "ignoring data\n";
                	return;
                }
            }
        }

        for (int i = 0; i < metaObject->propertyCount(); ++i) {
            QMetaProperty property = metaObject->property(i);

            if (shouldSaveProperty(property)) {
                QString key = QString::fromUtf8(property.name());
                QVariant defaultValue = property.read(m_owner);
                QVariant value = settings.value(key, defaultValue);

                m_owner->setProperty(property.name(), value);
            }
        }

        settings.endGroup();
    }
private:
    QWidget* m_owner;
    QString m_path;
};

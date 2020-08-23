#include "ParametersDelegate.h"

#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <QMouseEvent>
#include <QApplication>

#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFNode.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFDouble.h>

#include "libraries/Coin3D/FieldEditor.h"
#include "libraries/Coin3D/UserSField.h"
#include "libraries/Coin3D/UserMField.h"
#include "main/MainWindow.h"
#include "main/PluginManager.h"
#include "kernel/scene/TFactory.h"
#include "kernel/scene/TNode.h"

#include "ParametersModel.h"
#include "ParametersItemField.h"
#include "ParametersEditor.h"


ParametersDelegate::ParametersDelegate(QObject* parent):
    QStyledItemDelegate(parent)
{

}

QWidget* ParametersDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const ParametersModel* model = static_cast<const ParametersModel*>(index.model());
    ParametersItemField* item = static_cast<ParametersItemField*>(model->itemFromIndex(index));
    SoField* field = item->field();

    if (SoSFEnum* f = dynamic_cast<SoSFEnum*>(field))
    {
        QComboBox* editor = new QComboBox(parent);
        SbName name;
        for (int n = 0; n < f->getNumEnums(); ++n)
        {
            f->getEnum(n, name);
            editor->addItem(name.getString());
        }
        editor->setCurrentIndex(f->getValue());
        connect(
            editor,  SIGNAL(activated(int)),
            editor, SLOT(close())
        );
        return editor;
    }
    else if (SoSFBool* f = dynamic_cast<SoSFBool*>(field))
    {
        QComboBox* editor = new QComboBox(parent);
        editor->addItems({"false", "true"});
        editor->setCurrentIndex(f->getValue() ? 1 : 0);
        connect(
            editor, SIGNAL(activated(int)),
            this, SLOT(onCloseEditor())
        );
        return editor;
    }
    else if (SoSFNode* f = dynamic_cast<SoSFNode*>(field))
    {
        QComboBox* editor = new QComboBox(parent);
        SoNode* node = f->getValue();
        MainWindow* main = model->getMain();
        QVector<TFactory*> factories = main->getPlugins()->getFactories(node);
        for (TFactory* tf : factories) {
            if (!tf)
                editor->insertSeparator(editor->count());
            else
                editor->addItem(tf->icon(), tf->name());
        }

        if (TNode* tnode = dynamic_cast<TNode*>(node))
            editor->setCurrentText(tnode->getTypeName());

        connect(
            editor, SIGNAL(activated(int)),
            this, SLOT(onCloseEditor())
        );
        return editor;
    }
    else if (SoSFInt32* f = dynamic_cast<SoSFInt32*>(field))
    {
        QSpinBox* editor = new QSpinBox(parent);
        editor->setMinimum(std::numeric_limits<int>::min());
        editor->setMaximum(std::numeric_limits<int>::max());
        editor->setValue(f->getValue());
        return editor;
    }
//    else if (SoSFDouble* f = dynamic_cast<SoSFDouble*>(field))
//    {
//        QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
//        editor->setValue(f->getValue());
//        return editor;
//    }
    else if (UserSField* f = dynamic_cast<UserSField*>(field))
    {
        FieldEditor* editor = f->getEditor();
        editor->setParent(parent);
        editor->setGeometry(option.rect);
        QString s = model->data(index).toString();
        editor->setData(s);
        connect(
            editor, SIGNAL(editingFinished()),
            this, SLOT(onCloseEditor())
        );
        return editor;
    }
    else if (UserMField* f = dynamic_cast<UserMField*>(field))
    {
        FieldEditor* editor = f->getEditor();
        editor->setParent(parent);
        editor->setGeometry(option.rect);
        QString s = model->data(index).toString();
        editor->setData(s);
        connect(
            editor, SIGNAL(editingFinished()),
            this, SLOT(onCloseEditor())
        );
        return editor;
    }
    else
    {
        QString text = model->data(index, Qt::EditRole).toString();

        if (text.indexOf('\n') >= 0) {
//            text = text.trimmed();
            ParametersEditor* editor = new ParametersEditor;
            editor->setText(text);
            return editor;
        } else {
            QLineEdit* editor = new QLineEdit(parent);
            editor->setText(text);
            return editor;
        }
    }
}

void ParametersDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
    // keep
}

void ParametersDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!dynamic_cast<ParametersEditor*>(editor))
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);

    if (QComboBox* cb = dynamic_cast<QComboBox*>(editor))
    {
        cb->setGeometry(option.rect);
//        cb->showPopup();
        QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, 0, 0);
        QApplication::sendEvent(cb, &event);
    }
}

void ParametersDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    ParametersModel* modelP = static_cast<ParametersModel*>(model);
    ParametersItemField* item = static_cast<ParametersItemField*>(modelP->itemFromIndex(index));
    SoField* field = item->field();

    QString value;

    if (dynamic_cast<SoSFEnum*>(field))
    {
        QComboBox* w = qobject_cast<QComboBox*>(editor);
        value = w->currentText();
    }
    else if (dynamic_cast<SoSFBool*>(field))
    {
        QComboBox* w = qobject_cast<QComboBox*>(editor);
        value = w->currentIndex() ? "TRUE" : "FALSE";
    }
    else if (SoSFNode* f = dynamic_cast<SoSFNode*>(field))
    {
        QComboBox* w = qobject_cast<QComboBox*>(editor);
        value = w->currentText();
        QString text = model->data(index, Qt::DisplayRole).toString();
        if (value == text) return;

        SoNode* node = f->getValue();
        MainWindow* main = modelP->getMain();
        TFactory* tf = main->getPlugins()->getFactories(node)[w->currentIndex()];
        main->Insert(tf);
        return;
    }
    else if (dynamic_cast<SoSFInt32*>(field))
    {
        QSpinBox* w = qobject_cast<QSpinBox*>(editor);
        value = QString::number(w->value());
    }
//    else if (dynamic_cast<SoSFDouble*>(field))
//    {
//        QDoubleSpinBox* w = qobject_cast<QDoubleSpinBox*>(editor);
//        value = QString::number(w->value());
//    }
    else if (dynamic_cast<UserSField*>(field))
    {
        FieldEditor* w = static_cast<FieldEditor*>(editor);
        value = w->getData();
    }
    else if (dynamic_cast<UserMField*>(field))
    {
        FieldEditor* w = static_cast<FieldEditor*>(editor);
        value = w->getData();
    }
    else
    {
        if (QLineEdit* w = dynamic_cast<QLineEdit*>(editor) )
            value = w->text();
        else if (ParametersEditor* w = dynamic_cast<ParametersEditor*>(editor) )
            value = w->text();
    }

    QString text = model->data(index, Qt::EditRole).toString();
    if (value == text) return;
    modelP->setData(index, value);
}

void ParametersDelegate::onCloseEditor()
{
    QWidget* editor = qobject_cast<QWidget*>(sender());
//    emit commitData(editor);
//    emit closeEditor(editor);
    editor->close();
}

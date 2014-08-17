/****************************************************************************
** Meta object code from reading C++ file 'GroupConfigWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ui/GroupConfigWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GroupConfigWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_GroupConfigWidget_t {
    QByteArrayData data[15];
    char stringdata[280];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GroupConfigWidget_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GroupConfigWidget_t qt_meta_stringdata_GroupConfigWidget = {
    {
QT_MOC_LITERAL(0, 0, 17),
QT_MOC_LITERAL(1, 18, 18),
QT_MOC_LITERAL(2, 37, 0),
QT_MOC_LITERAL(3, 38, 9),
QT_MOC_LITERAL(4, 48, 6),
QT_MOC_LITERAL(5, 55, 19),
QT_MOC_LITERAL(6, 75, 8),
QT_MOC_LITERAL(7, 84, 23),
QT_MOC_LITERAL(8, 108, 27),
QT_MOC_LITERAL(9, 136, 6),
QT_MOC_LITERAL(10, 143, 27),
QT_MOC_LITERAL(11, 171, 27),
QT_MOC_LITERAL(12, 199, 26),
QT_MOC_LITERAL(13, 226, 26),
QT_MOC_LITERAL(14, 253, 26)
    },
    "GroupConfigWidget\0groupConfigChanged\0"
    "\0KO_Config\0newCfg\0onStartLevelChanged\0"
    "newIndex\0onSecondSurvivesChanged\0"
    "onSpinBoxGroupCount1Changed\0newVal\0"
    "onSpinBoxGroupCount2Changed\0"
    "onSpinBoxGroupCount3Changed\0"
    "onSpinBoxGroupSize1Changed\0"
    "onSpinBoxGroupSize2Changed\0"
    "onSpinBoxGroupSize3Changed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GroupConfigWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   62,    2, 0x0a /* Public */,
       7,    0,   65,    2, 0x0a /* Public */,
       8,    1,   66,    2, 0x0a /* Public */,
      10,    1,   69,    2, 0x0a /* Public */,
      11,    1,   72,    2, 0x0a /* Public */,
      12,    1,   75,    2, 0x0a /* Public */,
      13,    1,   78,    2, 0x0a /* Public */,
      14,    1,   81,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,    9,

       0        // eod
};

void GroupConfigWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        GroupConfigWidget *_t = static_cast<GroupConfigWidget *>(_o);
        switch (_id) {
        case 0: _t->groupConfigChanged((*reinterpret_cast< const KO_Config(*)>(_a[1]))); break;
        case 1: _t->onStartLevelChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->onSecondSurvivesChanged(); break;
        case 3: _t->onSpinBoxGroupCount1Changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->onSpinBoxGroupCount2Changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->onSpinBoxGroupCount3Changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->onSpinBoxGroupSize1Changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->onSpinBoxGroupSize2Changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->onSpinBoxGroupSize3Changed((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (GroupConfigWidget::*_t)(const KO_Config & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&GroupConfigWidget::groupConfigChanged)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject GroupConfigWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_GroupConfigWidget.data,
      qt_meta_data_GroupConfigWidget,  qt_static_metacall, 0, 0}
};


const QMetaObject *GroupConfigWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GroupConfigWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GroupConfigWidget.stringdata))
        return static_cast<void*>(const_cast< GroupConfigWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int GroupConfigWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void GroupConfigWidget::groupConfigChanged(const KO_Config & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

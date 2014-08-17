/****************************************************************************
** Meta object code from reading C++ file 'CatTabWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ui/CatTabWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CatTabWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_CatTabWidget_t {
    QByteArrayData data[21];
    char stringdata[345];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CatTabWidget_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CatTabWidget_t qt_meta_stringdata_CatTabWidget = {
    {
QT_MOC_LITERAL(0, 0, 12),
QT_MOC_LITERAL(1, 13, 17),
QT_MOC_LITERAL(2, 31, 0),
QT_MOC_LITERAL(3, 32, 21),
QT_MOC_LITERAL(4, 54, 14),
QT_MOC_LITERAL(5, 69, 15),
QT_MOC_LITERAL(6, 85, 8),
QT_MOC_LITERAL(7, 94, 18),
QT_MOC_LITERAL(8, 113, 6),
QT_MOC_LITERAL(9, 120, 17),
QT_MOC_LITERAL(10, 138, 33),
QT_MOC_LITERAL(11, 172, 16),
QT_MOC_LITERAL(12, 189, 31),
QT_MOC_LITERAL(13, 221, 17),
QT_MOC_LITERAL(14, 239, 24),
QT_MOC_LITERAL(15, 264, 3),
QT_MOC_LITERAL(16, 268, 12),
QT_MOC_LITERAL(17, 281, 17),
QT_MOC_LITERAL(18, 299, 18),
QT_MOC_LITERAL(19, 318, 20),
QT_MOC_LITERAL(20, 339, 5)
    },
    "CatTabWidget\0onCatModelChanged\0\0"
    "onTabSelectionChanged\0QItemSelection\0"
    "onCbDrawChanged\0newState\0onDrawScoreChanged\0"
    "newVal\0onWinScoreChanged\0"
    "onUnpairedPlayersSelectionChanged\0"
    "onBtnPairClicked\0onPairedPlayersSelectionChanged\0"
    "onBtnSplitClicked\0onMatchTypeButtonClicked\0"
    "btn\0onSexClicked\0onDontCareClicked\0"
    "onBtnAddCatClicked\0onMatchSystemChanged\0"
    "newId"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CatTabWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x0a /* Public */,
       3,    2,   85,    2, 0x0a /* Public */,
       5,    1,   90,    2, 0x0a /* Public */,
       7,    1,   93,    2, 0x0a /* Public */,
       9,    1,   96,    2, 0x0a /* Public */,
      10,    0,   99,    2, 0x0a /* Public */,
      11,    0,  100,    2, 0x0a /* Public */,
      12,    0,  101,    2, 0x0a /* Public */,
      13,    0,  102,    2, 0x0a /* Public */,
      14,    1,  103,    2, 0x0a /* Public */,
      16,    1,  106,    2, 0x0a /* Public */,
      17,    0,  109,    2, 0x0a /* Public */,
      18,    0,  110,    2, 0x0a /* Public */,
      19,    1,  111,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4, 0x80000000 | 4,    2,    2,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   20,

       0        // eod
};

void CatTabWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CatTabWidget *_t = static_cast<CatTabWidget *>(_o);
        switch (_id) {
        case 0: _t->onCatModelChanged(); break;
        case 1: _t->onTabSelectionChanged((*reinterpret_cast< const QItemSelection(*)>(_a[1])),(*reinterpret_cast< const QItemSelection(*)>(_a[2]))); break;
        case 2: _t->onCbDrawChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->onDrawScoreChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->onWinScoreChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->onUnpairedPlayersSelectionChanged(); break;
        case 6: _t->onBtnPairClicked(); break;
        case 7: _t->onPairedPlayersSelectionChanged(); break;
        case 8: _t->onBtnSplitClicked(); break;
        case 9: _t->onMatchTypeButtonClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->onSexClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->onDontCareClicked(); break;
        case 12: _t->onBtnAddCatClicked(); break;
        case 13: _t->onMatchSystemChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject CatTabWidget::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_CatTabWidget.data,
      qt_meta_data_CatTabWidget,  qt_static_metacall, 0, 0}
};


const QMetaObject *CatTabWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CatTabWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CatTabWidget.stringdata))
        return static_cast<void*>(const_cast< CatTabWidget*>(this));
    return QDialog::qt_metacast(_clname);
}

int CatTabWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE

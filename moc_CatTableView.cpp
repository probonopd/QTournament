/****************************************************************************
** Meta object code from reading C++ file 'CatTableView.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ui/CatTableView.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CatTableView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_CategoryTableView_t {
    QByteArrayData data[9];
    char stringdata[120];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CategoryTableView_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CategoryTableView_t qt_meta_stringdata_CategoryTableView = {
    {
QT_MOC_LITERAL(0, 0, 17),
QT_MOC_LITERAL(1, 18, 15),
QT_MOC_LITERAL(2, 34, 0),
QT_MOC_LITERAL(3, 35, 18),
QT_MOC_LITERAL(4, 54, 18),
QT_MOC_LITERAL(5, 73, 11),
QT_MOC_LITERAL(6, 85, 4),
QT_MOC_LITERAL(7, 90, 23),
QT_MOC_LITERAL(8, 114, 5)
    },
    "CategoryTableView\0catModelChanged\0\0"
    "onTournamentClosed\0onTournamentOpened\0"
    "Tournament*\0tnmt\0onCategoryDoubleClicked\0"
    "index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CategoryTableView[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   35,    2, 0x0a /* Public */,
       4,    1,   36,    2, 0x0a /* Public */,
       7,    1,   39,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, QMetaType::QModelIndex,    8,

       0        // eod
};

void CategoryTableView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CategoryTableView *_t = static_cast<CategoryTableView *>(_o);
        switch (_id) {
        case 0: _t->catModelChanged(); break;
        case 1: _t->onTournamentClosed(); break;
        case 2: _t->onTournamentOpened((*reinterpret_cast< Tournament*(*)>(_a[1]))); break;
        case 3: _t->onCategoryDoubleClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Tournament* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (CategoryTableView::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CategoryTableView::catModelChanged)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject CategoryTableView::staticMetaObject = {
    { &QTableView::staticMetaObject, qt_meta_stringdata_CategoryTableView.data,
      qt_meta_data_CategoryTableView,  qt_static_metacall, 0, 0}
};


const QMetaObject *CategoryTableView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CategoryTableView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CategoryTableView.stringdata))
        return static_cast<void*>(const_cast< CategoryTableView*>(this));
    return QTableView::qt_metacast(_clname);
}

int CategoryTableView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void CategoryTableView::catModelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
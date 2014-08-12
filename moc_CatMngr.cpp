/****************************************************************************
** Meta object code from reading C++ file 'CatMngr.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "CatMngr.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CatMngr.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QTournament__CatMngr_t {
    QByteArrayData data[15];
    char stringdata[174];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QTournament__CatMngr_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QTournament__CatMngr_t qt_meta_stringdata_QTournament__CatMngr = {
    {
QT_MOC_LITERAL(0, 0, 20),
QT_MOC_LITERAL(1, 21, 13),
QT_MOC_LITERAL(2, 35, 0),
QT_MOC_LITERAL(3, 36, 8),
QT_MOC_LITERAL(4, 45, 1),
QT_MOC_LITERAL(5, 47, 6),
QT_MOC_LITERAL(6, 54, 2),
QT_MOC_LITERAL(7, 57, 2),
QT_MOC_LITERAL(8, 60, 12),
QT_MOC_LITERAL(9, 73, 21),
QT_MOC_LITERAL(10, 95, 1),
QT_MOC_LITERAL(11, 97, 25),
QT_MOC_LITERAL(12, 123, 19),
QT_MOC_LITERAL(13, 143, 17),
QT_MOC_LITERAL(14, 161, 12)
    },
    "QTournament::CatMngr\0playersPaired\0\0"
    "Category\0c\0Player\0p1\0p2\0playersSplit\0"
    "playerAddedToCategory\0p\0"
    "playerRemovedFromCategory\0beginCreateCategory\0"
    "endCreateCategory\0newCatSeqNum"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QTournament__CatMngr[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   44,    2, 0x06 /* Public */,
       8,    3,   51,    2, 0x06 /* Public */,
       9,    2,   58,    2, 0x06 /* Public */,
      11,    2,   63,    2, 0x06 /* Public */,
      12,    0,   68,    2, 0x06 /* Public */,
      13,    1,   69,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 5,    4,    6,    7,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5, 0x80000000 | 5,    4,    6,    7,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 3,   10,    4,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 3,   10,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   14,

       0        // eod
};

void QTournament::CatMngr::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CatMngr *_t = static_cast<CatMngr *>(_o);
        switch (_id) {
        case 0: _t->playersPaired((*reinterpret_cast< const Category(*)>(_a[1])),(*reinterpret_cast< const Player(*)>(_a[2])),(*reinterpret_cast< const Player(*)>(_a[3]))); break;
        case 1: _t->playersSplit((*reinterpret_cast< const Category(*)>(_a[1])),(*reinterpret_cast< const Player(*)>(_a[2])),(*reinterpret_cast< const Player(*)>(_a[3]))); break;
        case 2: _t->playerAddedToCategory((*reinterpret_cast< const Player(*)>(_a[1])),(*reinterpret_cast< const Category(*)>(_a[2]))); break;
        case 3: _t->playerRemovedFromCategory((*reinterpret_cast< const Player(*)>(_a[1])),(*reinterpret_cast< const Category(*)>(_a[2]))); break;
        case 4: _t->beginCreateCategory(); break;
        case 5: _t->endCreateCategory((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (CatMngr::*_t)(const Category , const Player & , const Player & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CatMngr::playersPaired)) {
                *result = 0;
            }
        }
        {
            typedef void (CatMngr::*_t)(const Category , const Player & , const Player & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CatMngr::playersSplit)) {
                *result = 1;
            }
        }
        {
            typedef void (CatMngr::*_t)(const Player & , const Category & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CatMngr::playerAddedToCategory)) {
                *result = 2;
            }
        }
        {
            typedef void (CatMngr::*_t)(const Player & , const Category & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CatMngr::playerRemovedFromCategory)) {
                *result = 3;
            }
        }
        {
            typedef void (CatMngr::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CatMngr::beginCreateCategory)) {
                *result = 4;
            }
        }
        {
            typedef void (CatMngr::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CatMngr::endCreateCategory)) {
                *result = 5;
            }
        }
    }
}

const QMetaObject QTournament::CatMngr::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QTournament__CatMngr.data,
      qt_meta_data_QTournament__CatMngr,  qt_static_metacall, 0, 0}
};


const QMetaObject *QTournament::CatMngr::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QTournament::CatMngr::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QTournament__CatMngr.stringdata))
        return static_cast<void*>(const_cast< CatMngr*>(this));
    if (!strcmp(_clname, "GenericObjectManager"))
        return static_cast< GenericObjectManager*>(const_cast< CatMngr*>(this));
    return QObject::qt_metacast(_clname);
}

int QTournament::CatMngr::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void QTournament::CatMngr::playersPaired(const Category _t1, const Player & _t2, const Player & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QTournament::CatMngr::playersSplit(const Category _t1, const Player & _t2, const Player & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QTournament::CatMngr::playerAddedToCategory(const Player & _t1, const Category & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QTournament::CatMngr::playerRemovedFromCategory(const Player & _t1, const Category & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void QTournament::CatMngr::beginCreateCategory()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void QTournament::CatMngr::endCreateCategory(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE

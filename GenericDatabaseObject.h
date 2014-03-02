/* 
 * File:   GenericDatabaseObject.h
 * Author: nyoknvk1
 *
 * Created on 18. Februar 2014, 14:25
 */

#ifndef GENERICDATABASEOBJECT_H
#define	GENERICDATABASEOBJECT_H

namespace QTournament
{

    class GenericDatabaseObject
    {
    public:
        GenericDatabaseObject(const QString& _tabName, int _id);

        bool operator==(const GenericDatabaseObject& other);

    private:
        QString tabName;
        int id;

    };
}

#endif	/* GENERICDATABASEOBJECT_H */


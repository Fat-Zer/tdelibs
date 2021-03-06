#include <db.h>

#include <tqfile.h>

#include "dbwrapper.h"


using namespace Evolution;

struct DBIterator::Data {
    DBWrapper *wrapper;
    TQString key;
    TQString data;
    DBC* cursor;
    bool atEnd;
};

DBIterator::DBIterator( DBWrapper* wra) {
    data = new Data;
    data->wrapper = wra;
    data->atEnd = false;
    data->cursor = 0l;
}
DBIterator::DBIterator( const DBIterator& copy ) {
    data = new Data;
    data->wrapper = copy.data->wrapper;
    data->key = copy.data->key;
    data->data = copy.data->data;
    data->atEnd = copy.data->atEnd;
    if (copy.data->cursor )
        copy.data->cursor->c_dup(copy.data->cursor, &data->cursor, 0 );
    else
        data->cursor = 0l;
}
DBIterator::~DBIterator() {
    if (data->cursor)
        data->cursor->c_close(data->cursor);
    delete data;
}
DBIterator& DBIterator::operator=( const DBIterator& rhs ) {
    if ( *this == rhs )
        return *this;
    if (data->cursor)
        data->cursor->c_close(data->cursor);
    delete data;
    data = new Data;
    data->wrapper = rhs.data->wrapper;
    data->key = rhs.data->key;
    data->data = rhs.data->data;
    data->atEnd = rhs.data->atEnd;
    if ( rhs.data->cursor )
        rhs.data->cursor->c_dup(rhs.data->cursor, &data->cursor, 0 );
    else
        data->cursor = 0l;

    return *this;
}
TQString DBIterator::key()const{
    return data->key;
}
TQString DBIterator::value()const {
    return data->data;
}
TQString DBIterator::operator*() {
    return data->data;
}
DBIterator& DBIterator::operator++() {
    DBT key, val;
    ::memset(&key, 0, sizeof(key) );
    ::memset(&val, 0, sizeof(val) );
    if ( data->cursor )
        if ( data->cursor->c_get(data->cursor, &key, &val,DB_NEXT ) != 0 )
            data->atEnd = true;
    data->key = TQString::fromUtf8( (char*)key.data, key.size );
    data->data = TQString::fromUtf8( (char*)val.data, val.size );
    return *this;
}
DBIterator& DBIterator::operator--() {
    DBT key, val;
    ::memset(&key, 0, sizeof(key) );
    ::memset(&val, 0, sizeof(val) );
    if ( data->cursor )
        if ( data->cursor->c_get(data->cursor, &key, &val,DB_PREV ) != 0 )
            data->atEnd = true;
    data->key = TQString::fromUtf8( (char*)key.data, key.size );
    data->data = TQString::fromUtf8( (char*)val.data, val.size );
    return *this;
}
bool DBIterator::operator==( const DBIterator& rhs ) {
    if ( data->atEnd && data->atEnd == rhs.data->atEnd ) return true;

    return false;
}
bool DBIterator::operator!=( const DBIterator& rhs ) {
    return !this->operator==(rhs );
}
struct DBWrapper::Data {
    DB* db;
    bool only;
};
DBWrapper::DBWrapper() {
    data = new Data;
    (void)db_create(&data->db, NULL, 0 );
    data->only = false;
}
DBWrapper::~DBWrapper() {
    data->db->close(data->db, 0 );
    delete data;
}
bool DBWrapper::open( const TQString& file, bool on) {
    data->only = on;
    return  !data->db->open(data->db, TQFile::encodeName( file ), NULL, DB_HASH, 0, 0666 );
}
bool DBWrapper::save() {
    return true;
}
DBIterator DBWrapper::begin() {
    DBIterator it(this);
    DBC* cursor;
    DBT key, val;
    int ret;
    ret = data->db->cursor(data->db, NULL, &cursor, 0 );
    if (ret ) {
        it.data->atEnd = true;
        return it;
    }

    ::memset(&key, 0, sizeof(key) );
    ::memset(&val, 0, sizeof(val) );
    ret = cursor->c_get(cursor, &key, &val, DB_FIRST );
    if (ret ) {
        it.data->atEnd = true;
        return it;
    }

    it.data->cursor = cursor;
    it.data->key = TQString::fromUtf8((char*)key.data, key.size );
    it.data->data = TQString::fromUtf8((char*)val.data, val.size );

    return it;
}
DBIterator DBWrapper::end() {
    DBIterator it(this);
    it.data->atEnd = true;

    return it;
}
bool DBWrapper::find( const TQString& _key,  TQString& _val ) {
    DBT key, val;
    ::memset(&key, 0, sizeof(key) );
    ::memset(&val, 0, sizeof(val) );

    TQCString db_key = _key.local8Bit();
    key.data = db_key.data();
    key.size = db_key.size();

    int ret  = data->db->get(data->db, NULL, &key,  &val, 0 );
    if (!ret) {
        _val = TQString::fromUtf8( (char*)val.data, val.size );
        tqWarning("key: %s val: %sXXX", (char*)key.data, (char*)val.data );
        return true;
    }
    return false;
}
bool DBWrapper::add( const TQString& _key, const TQString& _val ) {
    TQCString db_key = _key.local8Bit();
    TQCString db_val = _val.local8Bit();
    DBT key, val;
    ::memset(&key, 0, sizeof(key) );
    ::memset(&val, 0, sizeof(val) );

    key.data = db_key.data();
    key.size = db_key.size();
    val.data = db_val.data();
    val.size = db_val.size();

    return !data->db->put(data->db, NULL, &key, &val, 0 );
}
bool DBWrapper::remove( const TQString& _key ) {
    TQCString db_key = _key.local8Bit();
    DBT key;
    memset(&key, 0, sizeof(key) );
    key.data = db_key.data();
    key.size = db_key.size();

    return !data->db->del(data->db, NULL, &key, 0 );
}

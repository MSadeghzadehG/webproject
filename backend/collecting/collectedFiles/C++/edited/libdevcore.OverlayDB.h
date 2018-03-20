


#pragma once

#include <memory>
#include <libdevcore/db.h>
#include <libdevcore/Common.h>
#include <libdevcore/Log.h>
#include <libdevcore/MemoryDB.h>

namespace dev
{

struct DBDetail: public LogChannel { static const char* name() { return "DBDetail"; } static const int verbosity = 14; };

class OverlayDB: public MemoryDB
{
public:
    explicit OverlayDB(std::unique_ptr<db::DatabaseFace> _db = nullptr)
      : m_db(_db.release(), [](db::DatabaseFace* db) {
            clog(DBDetail) << "Closing state DB";
            delete db;
        })
    {}

    ~OverlayDB();

        OverlayDB(OverlayDB const&) = default;
    OverlayDB& operator=(OverlayDB const&) = default;
        OverlayDB(OverlayDB&&) = default;
    OverlayDB& operator=(OverlayDB&&) = default;

    void commit();
	void rollback();

	std::string lookup(h256 const& _h) const;
	bool exists(h256 const& _h) const;
	void kill(h256 const& _h);

	bytes lookupAux(h256 const& _h) const;

private:
	using MemoryDB::clear;

    std::shared_ptr<db::DatabaseFace> m_db;
};

}

#include "Globals.h"
#include "Databaza/SqliteDatabase.h"

std::unique_ptr<IDatabase> g_db = std::make_unique<SqliteDatabase>("db/messages.db");
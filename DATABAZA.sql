CREATE TABLE IF NOT EXISTS messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    source_service TEXT NOT NULL,
    timestamp_utc DATETIME NOT NULL,
    payload TEXT NOT NULL,
    received_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    status TEXT DEFAULT 'received',
    schema_version INTEGER DEFAULT 1,
    processed BOOLEAN DEFAULT 0,
    UNIQUE(source_service, timestamp_utc, payload(100))
);

CREATE INDEX IF NOT EXISTS idx_source_service ON messages(source_service);
CREATE INDEX IF NOT EXISTS idx_timestamp ON messages(timestamp_utc);
CREATE INDEX IF NOT EXISTS idx_status ON messages(status);

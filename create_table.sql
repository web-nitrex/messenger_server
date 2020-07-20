CREATE TABLE users (
    id       INTEGER    PRIMARY KEY AUTOINCREMENT
                        UNIQUE
                        NOT NULL,
    login    CHAR (255) UNIQUE
                        NOT NULL,
    password            NOT NULL
);
